#pragma once

#include "esphome/core/component.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"

namespace esphome
{
  namespace tesla_ble_listener
  {

    std::string get_vin_advertisement_name (const char *vin);

    class TeslaBLEListener : public Component, public esp32_ble_tracker::ESPBTDeviceListener
    {
    public:
      std::string vin_ad_name_;
      std::string expected_mac_;
      std::string vin_;

      void setup () override;
      void dump_configuration ();
      bool parse_device( const esp32_ble_tracker::ESPBTDevice &device) override;
      void set_vin (const char *vin)
      {
        vin_ad_name_ = get_vin_advertisement_name(vin);
        this->vin_ = vin;
      }
      void set_expected_mac (const std::string &mac) { expected_mac_ = mac; }
    };

  } // namespace tesla_ble_listener
} // namespace esphome
