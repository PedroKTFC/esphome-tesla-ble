#include "tesla_ble_listener.h"
#include "esphome/core/log.h"
#include "mbedtls/sha1.h"
#include <string>
#include <cinttypes>

namespace esphome
{
  namespace tesla_ble_listener
  {
    static const char *const TAG = "tesla_ble_listener";

    void TeslaBLEListener::setup()
    {
      this->set_timeout ("startup_log", 10000, [this]()
      {
        this->dump_configuration();
      });
    }

    void TeslaBLEListener::dump_configuration()
    {
      ESP_LOGI (TAG, "Configured VIN: %s", this->vin_.c_str());
      ESP_LOGI (TAG, "VIN advertisement name: %s", this->vin_ad_name_.c_str());
      ESP_LOGI (TAG, "Configured MAC address: %s", this->expected_mac_.empty() ? "Not configured" : this->expected_mac_.c_str());
    }

    std::string get_vin_advertisement_name(const char *vin)
    {
      // * BLE advertisement local name: `S + <ID> + C`, where `<ID>` is the
      //  lower-case hex-encoding of the first eight bytes of the SHA1 digest of the
      //  Vehicle Identification Number (VIN). For example, If the VIN is
      //  `5YJS0000000000000`, then the BLE advertisement Local Name is
      //  `S1a87a5a75f3df858C`.

      unsigned char vin_sha1[20];
      size_t vin_length = 17; // Assuming standard VIN length
      int return_code = mbedtls_sha1((const unsigned char *)vin, vin_length, vin_sha1);
      if (return_code != 0)
      {
        ESP_LOGE(TAG, "Failed to calculate SHA1 of VIN");
        return "";
      }

      char result[19]; // 'S' + 16 hex chars + 'C' + null terminator
      result[0] = 'S';
      for (int i = 0; i < 8; i++)
      {
        snprintf(&result[1 + i * 2], 3, "%02x", vin_sha1[i]);
      }
      result[17] = 'C';
      result[18] = '\0'; // Null terminator for safe printing

      std::string result_str(result);
      ESP_LOGD(TAG, "VIN advertisement name: %s", result_str.c_str());
      return result_str;
    }

    bool TeslaBLEListener::parse_device(const esp32_ble_tracker::ESPBTDevice &device)
    {
      const std::string device_name = device.get_name();
      const std::string device_mac = device.address_str();

      ESP_LOGD(TAG, "Parsing device: [%s]: %s", device.address_str().c_str(), device_name.c_str());

      if (device_name == this->vin_ad_name_)
      {
        ESP_LOGI(TAG, "Found Tesla vehicle | Name: %s | MAC: %s", device.get_name().c_str(), device.address_str().c_str());
        return true;
      }
      // Only report a VIN mismatch when the MAC address identifies
      // the vehicle we are expecting.
      if ((!this->expected_mac_.empty()) &&
          (device_mac == this->expected_mac_) &&
          (device_name.length() == 18) &&
          (device_name.front() == 'S') &&
          (device_name.back() == 'C'))
      {
        ESP_LOGW(TAG, "Possible VIN mismatch! MAC matches configured vehicle but advertisement name differs. Expected: %s | Actual: %s",
                 this->vin_ad_name_.c_str(), device_name.c_str());
      }

      return false;
    }
  } // namespace tesla_ble_listener
} // namespace esphome
