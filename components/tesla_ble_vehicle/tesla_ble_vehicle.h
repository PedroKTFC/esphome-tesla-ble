#pragma once

#include <algorithm>
#include <cstring>
#include <iterator>
#include <vector>
#include <queue>

#include <esp_gattc_api.h>
#include <esphome/components/binary_sensor/binary_sensor.h>
#include <esphome/components/text_sensor/text_sensor.h>
#include <esphome/components/sensor/sensor.h>
#include <esphome/components/ble_client/ble_client.h>
#include <esphome/components/esp32_ble_tracker/esp32_ble_tracker.h>
#include <esphome/components/sensor/sensor.h>
#include <esphome/core/component.h>
#include <esphome/core/log.h>

#include <universal_message.pb.h>
#include <vcsec.pb.h>
#include <errors.h>

//#include "custom_binary_sensor.h"

namespace TeslaBLE
{
    class Client;
}

typedef enum BLE_CarServer_VehicleAction_E
{
    DO_NOTHING, // Blank empty entry
    GET_CHARGE_STATE,
    GET_CLIMATE_STATE,
    GET_DRIVE_STATE,
    GET_LOCATION_STATE,
    GET_CLOSURES_STATE,
    SET_CHARGING_SWITCH,
    SET_CHARGING_AMPS,
    SET_CHARGING_LIMIT,
    SET_SENTRY_SWITCH,
    SET_HVAC_SWITCH,
    SET_HVAC_STEERING_HEATER_SWITCH,
    SET_OPEN_CHARGE_PORT_DOOR,
    SET_CLOSE_CHARGE_PORT_DOOR,
    SOUND_HORN,
    FLASH_LIGHT,
    SET_WINDOWS_SWITCH,
    DEFROST_CAR
} BLE_CarServer_VehicleAction;

typedef enum // The type of messages to send
{
    VehicleActionMessage,
    GetVehicleDataMessage,
    Empty
} AllowedMsg;

typedef enum 
/*
*   The type of get to do immediately following a set (having this means a new set doesn't need a code change if it uses an existing
*   getxxxstate call.
*/
{
    GetChargeState,
    GetClimateState,
    GetDriveState,
    GetLocationState,
    GetClosureState,
    Invalid // Eg a data read, sensor not yet implemented, etc
} GetOnSet;

struct ActionMessageDetail
/*
*   This defines the contents of the rows of the ACTION_SPECIFICS table below. For every localActionDef, this describes the specific
*   contents for the message to send to the car. The rows of the table must be in the same order as BLE_CarServer_VehicleAction.
*   If a new message is needed, simply add a row to the table with the appropriate contents, no need to edit the code in the .cpp.
*/
{
    BLE_CarServer_VehicleAction_E localActionDef;
    std::string action_str;
    AllowedMsg whichMsg;
    int actionTag;
    GetOnSet getOnSet;
};

namespace esphome
{

    namespace tesla_ble_vehicle
    {
        namespace espbt = esphome::esp32_ble_tracker;

        static const struct ActionMessageDetail ACTION_SPECIFICS[] =
        {
            {DO_NOTHING,                       "",                          Empty,                 0,                                                              Invalid},
            {GET_CHARGE_STATE,                 "getChargeState",            GetVehicleDataMessage, CarServer_GetVehicleData_getChargeState_tag,                    Invalid},
            {GET_CLIMATE_STATE,                "getClimateState",           GetVehicleDataMessage, CarServer_GetVehicleData_getClimateState_tag,                   Invalid},
            {GET_DRIVE_STATE,                  "getDriveState",             GetVehicleDataMessage, CarServer_GetVehicleData_getDriveState_tag,                     Invalid},
            {GET_LOCATION_STATE,               "getLocationState",          GetVehicleDataMessage, CarServer_GetVehicleData_getLocationState_tag,                  Invalid},
            {GET_CLOSURES_STATE,               "getClosuresState",          GetVehicleDataMessage, CarServer_GetVehicleData_getClosuresState_tag,                  Invalid},
            {SET_CHARGING_SWITCH,              "setChargingSwitch",         VehicleActionMessage,  CarServer_VehicleAction_chargingStartStopAction_tag,            GetChargeState},
            {SET_CHARGING_AMPS,                "setChargingAmps",           VehicleActionMessage,  CarServer_VehicleAction_setChargingAmpsAction_tag,              GetChargeState},
            {SET_CHARGING_LIMIT,               "setChargingLimit",          VehicleActionMessage,  CarServer_VehicleAction_chargingSetLimitAction_tag,             GetChargeState},
            {SET_SENTRY_SWITCH,                "setSentrySwitch",           VehicleActionMessage,  CarServer_VehicleAction_vehicleControlSetSentryModeAction_tag,  Invalid},
            {SET_HVAC_SWITCH,                  "setHVACSwitch",             VehicleActionMessage,  CarServer_VehicleAction_hvacAutoAction_tag,                     GetClimateState},
            {SET_HVAC_STEERING_HEATER_SWITCH,  "setHVACSteeringHeatSwitch", VehicleActionMessage,  CarServer_VehicleAction_hvacSteeringWheelHeaterAction_tag,      Invalid},
            {SET_OPEN_CHARGE_PORT_DOOR,        "setOpenChargePortDoor",     VehicleActionMessage,  CarServer_VehicleAction_chargePortDoorOpen_tag,                 Invalid},
            {SET_CLOSE_CHARGE_PORT_DOOR,       "setCloseChargePortDoor",    VehicleActionMessage,  CarServer_VehicleAction_chargePortDoorClose_tag,                Invalid},
            {SOUND_HORN,                       "soundHorn",                 VehicleActionMessage,  CarServer_VehicleAction_vehicleControlHonkHornAction_tag,       Invalid},
            {FLASH_LIGHT,                      "flashLight",                VehicleActionMessage,  CarServer_VehicleAction_vehicleControlFlashLightsAction_tag,    Invalid},
            {SET_WINDOWS_SWITCH,               "setWindowsSwitch",          VehicleActionMessage,  CarServer_VehicleAction_vehicleControlWindowAction_tag,         GetClosureState},
            {DEFROST_CAR,                      "defrostCar",                VehicleActionMessage,  CarServer_VehicleAction_hvacSetPreconditioningMaxAction_tag,    GetClimateState}
        };

        static const char *const TAG = "tesla_ble_vehicle";
        static const char *nvs_key_infotainment = "tk_infotainment";
        static const char *nvs_key_vcsec = "tk_vcsec";

        static const char *const SERVICE_UUID = "00000211-b2d1-43f0-9b88-960cebf8b91e";
        static const char *const READ_UUID = "00000213-b2d1-43f0-9b88-960cebf8b91e";
        static const char *const WRITE_UUID = "00000212-b2d1-43f0-9b88-960cebf8b91e";

        static const int PRIVATE_KEY_SIZE = 228;
        static const int PUBLIC_KEY_SIZE = 65;
        static const int MAX_BLE_MESSAGE_SIZE = 1024; // Max size of a BLE message
        static const int RX_TIMEOUT = 1 * 1000;       // Timeout interval between receiving chunks of a message (1s)
        static const int MAX_LATENCY = 4 * 1000;      // Max allowed error when syncing vehicle clock (4s)
        static const int BLOCK_LENGTH = 20;           // BLE MTU is 23 bytes, so we need to split the message into chunks (20 bytes as in vehicle_command)
        static const int MAX_RETRIES = 5;             // Max number of retries for a command
        static const int COMMAND_TIMEOUT = 30 * 1000; // Overall timeout for a command (30s)

        enum class BLECommandState
        {
            IDLE,
            WAITING_FOR_VCSEC_AUTH,
            WAITING_FOR_VCSEC_AUTH_RESPONSE,
            WAITING_FOR_INFOTAINMENT_AUTH,
            WAITING_FOR_INFOTAINMENT_AUTH_RESPONSE,
            WAITING_FOR_WAKE,
            WAITING_FOR_WAKE_RESPONSE,
            WAITING_FOR_LOCK_RESPONSE,
            READY,
            WAITING_FOR_RESPONSE,
            WAITING_FOR_GET_POST_SET
        };

        struct BLECommand
        {
            UniversalMessage_Domain domain;
            std::function<int()> execute;
            std::string execute_name;
            BLE_CarServer_VehicleAction action; // Only used for Infotainment domain to store the detailed request made
            BLECommandState state;
            uint32_t started_at = millis();
            uint32_t last_tx_at = 0;
            uint8_t retry_count = 0;
            int done_times = 0; // Used to count if something has been done and how many times

            BLECommand(UniversalMessage_Domain d, std::function<int()> e, std::string n = "", BLE_CarServer_VehicleAction a = DO_NOTHING)
                : domain(d), execute(e), execute_name(n), action(a), state(BLECommandState::IDLE) {}
        };

        struct BLETXChunk
        {
            std::vector<unsigned char> data;
            esp_gatt_write_type_t write_type;
            esp_gatt_auth_req_t auth_req;
            uint32_t sent_at = millis();
            uint8_t retry_count = 0;

            BLETXChunk(std::vector<unsigned char> d, esp_gatt_write_type_t wt, esp_gatt_auth_req_t ar)
                : data(d), write_type(wt), auth_req(ar) {}
        };

        struct BLERXChunk
        {
            std::vector<unsigned char> buffer;
            uint32_t received_at = millis();

            BLERXChunk(std::vector<unsigned char> b)
                : buffer(b) {}
        };

        struct BLEResponse
        {
            // universal message
            UniversalMessage_RoutableMessage message;
            uint32_t received_at = millis();

            BLEResponse(UniversalMessage_RoutableMessage m)
                : message(m) {}
        };

        class TeslaBLEVehicle : public PollingComponent,
                                public ble_client::BLEClientNode
        {
        public:
            int post_wake_poll_time_;
            int poll_data_period_;
            int poll_asleep_period_;
            int poll_charging_period_;
            int car_just_woken_ = 0;
            bool previous_asleep_state_ = false;
            bool one_off_update_ = false;
            int car_wake_time_;
            int last_infotainment_poll_time_;
            int esp32_just_started_ = 0;
            int car_is_charging_ = 0;
            bool do_poll_ = false;
            int ble_disconnected_ = 1; // 0 = connected, 1 = disconnected, 2 = disconnected and Unknowns have been set
            int ble_disconnected_time_;
            int ble_disconnected_min_time_;
            int fast_poll_if_unlocked_ = 1; // != 0 enables fast polling
            TeslaBLEVehicle();
            void setup() override;
            void loop() override;
            void update() override;
            void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                     esp_ble_gattc_cb_param_t *param) override;
            void dump_config() override;
            void set_vin(const char *vin);
            void load_polling_parameters (const int post_wake_poll_time, const int poll_data_period,
                                          const int poll_asleep_period, const int poll_charging_period,
                                          const int ble_disconnected_min_time, const int fast_poll_if_unlocked);
            void process_command_queue();
            void process_response_queue();
            void process_ble_read_queue();
            void process_ble_write_queue();
            void invalidateSession(UniversalMessage_Domain domain);

            void regenerateKey();
            int startPair(void);
            int nvs_save_session_info(const Signatures_SessionInfo &session_info, const UniversalMessage_Domain domain);
            int nvs_load_session_info(Signatures_SessionInfo *session_info, const UniversalMessage_Domain domain);
            int nvs_initialize_private_key();

            int handleInfoCarServerResponse (CarServer_Response carserver_response);
            int handleSessionInfoUpdate(UniversalMessage_RoutableMessage message, UniversalMessage_Domain domain);
            int handleVCSECVehicleStatus(VCSEC_VehicleStatus vehicleStatus);

            int wakeVehicle(void);
            int lockVehicle (VCSEC_RKEAction_E lock);
            void placeAtFrontOfQueue (UniversalMessage_Domain domain, std::function<int()> execute, std::string execute_name, BLE_CarServer_VehicleAction action = DO_NOTHING);
    
            int sendVCSECActionMessage(VCSEC_RKEAction_E action);
            int sendVCSECClosureMoveRequestMessage (int moveWhat, VCSEC_ClosureMoveType_E moveType);
            int sendCarServerVehicleActionMessage(BLE_CarServer_VehicleAction action, int param);
            int sendSessionInfoRequest(UniversalMessage_Domain domain);
            int sendVCSECInformationRequest(void);
            void enqueueVCSECInformationRequest(bool force = false);

            int writeBLE(const unsigned char *message_buffer, size_t message_length,
                         esp_gatt_write_type_t write_type, esp_gatt_auth_req_t auth_req);

            // sensors
            // Sleep state (vehicleSleepStatus)
            void set_binary_sensor_is_asleep(binary_sensor::BinarySensor *s) { isAsleepSensor = static_cast<binary_sensor::BinarySensor *>(s); }
            void updateIsAsleep(bool asleep)
            {
                isAsleepSensor->publish_state(asleep);
            }
            // Door lock (vehicleLockState)
            void set_binary_sensor_is_unlocked(binary_sensor::BinarySensor *s) { isUnlockedSensor = static_cast<binary_sensor::BinarySensor *>(s); }
            void updateisUnlocked(bool locked)
            {
                isUnlockedSensor->publish_state(locked);
            }
            // User presence (userPresence)
            void set_binary_sensor_is_user_present(binary_sensor::BinarySensor *s) { isUserPresentSensor = static_cast<binary_sensor::BinarySensor *>(s); }
            void updateIsUserPresent(bool present)
            {
                isUserPresentSensor->publish_state(present);
            }
            // Charge flap (chargeFlapStatus)
            void set_binary_sensor_is_charge_flap_open(binary_sensor::BinarySensor *s) { isChargeFlapOpenSensor = static_cast<binary_sensor::BinarySensor *>(s); }
            void updateIsChargeFlapOpen(bool open)
            {
                isChargeFlapOpenSensor->publish_state(open);
            }
            void setChargeFlapHasState(bool has_state)
            {
                if (has_state)
                {
                    isChargeFlapOpenSensor->publish_state (has_state);
                }
                else
                {
                    isChargeFlapOpenSensor->invalidate_state ();
                }
            }

            // set sensors to unknown (e.g. when vehicle is disconnected)
            void setSensors(bool has_state) // has_state is an anachronism
            {
                isAsleepSensor->invalidate_state ();
                isUnlockedSensor->invalidate_state ();
                isUserPresentSensor->invalidate_state ();
            }

            void setInfotainmentSensors (bool state) // state is an anachronism
            {
                if (!state)
                {
                    ChargeStateSensor->publish_state (NAN);
                    OdometerStateSensor->publish_state (NAN);
                    ChargeCurrentStateSensor->publish_state (NAN);
                    ChargeVoltageStateSensor->publish_state (NAN);
                    ChargePowerStateSensor->publish_state (NAN);
                    MaxSocStateSensor->publish_state (NAN);
                    MaxAmpsStateSensor->publish_state (NAN);
                    MinsToLimitStateSensor->publish_state (NAN);
                    ShiftStateSensor->publish_state ("Unknown");
                    DefrostStateSensor->publish_state ("Unknown");
                    ChargingStateSensor->publish_state ("Unknown");
                    BatteryRangeStateSensor->publish_state (NAN);
                    isClimateOnSensor->invalidate_state ();
                    insideTempStateSensor->publish_state (NAN);
                    outsideTempStateSensor->publish_state (NAN);
                    ChargeEnergyAddedSensor->publish_state (NAN);
                    ChargeDistanceAddedSensor->publish_state (NAN);
                    isBootOpenSensor->invalidate_state ();
                    isFrunkOpenSensor->invalidate_state ();
                    windowsStateSensor->invalidate_state ();
                }

            }
            void setCarBatteryLevel (int battery_level)
            {
                ChargeStateSensor->publish_state (battery_level);
            }
            void setCarOdometer (int odometer)
            {
                OdometerStateSensor->publish_state (odometer);
            }
            void setChargeCurrent (int current)
            {
                ChargeCurrentStateSensor->publish_state (current);
            }
            void setChargeVoltage (int volts)
            {
                ChargeVoltageStateSensor->publish_state (volts);
            }
            void setChargePower (int power)
            {
                ChargePowerStateSensor->publish_state (power);
            }
            void setMaxSoc (int max)
            {
                MaxSocStateSensor->publish_state (max);
            }
            void setMaxAmps (int max)
            {
                MaxAmpsStateSensor->publish_state (max);
            }
            void setMinsToLimit (int limit)
            {
                MinsToLimitStateSensor->publish_state (limit);
            }
            void setBatteryRange (float range)
            {
                BatteryRangeStateSensor->publish_state (range);
            }
            void setCarShiftState (std::string shift_state)
            {
                ShiftStateSensor->publish_state (shift_state);
            }
            void setDefrostState (std::string defrost_state)
            {
                DefrostStateSensor->publish_state (defrost_state);
            }
            void setChargingState (std::string charging_state)
            {
                ChargingStateSensor->publish_state (charging_state);
            }
            void setLastUpdateState (std::string last_update)
            {
                LastUpdateStateSensor->publish_state (last_update);
            }
            void setClimateState (bool climate_state)
            {
                isClimateOnSensor->publish_state (climate_state);
            }
            void setBootState (bool boot_state)
            {
                isBootOpenSensor->publish_state (boot_state);
            }
            void setFrunkState (bool frunk_state)
            {
                isFrunkOpenSensor->publish_state (frunk_state);
            }
            void setWindowsState (bool windows_state)
            {
                windowsStateSensor->publish_state (windows_state);
            }
            void setInsideTemp (float temp)
            {
                insideTempStateSensor->publish_state (temp);
            }
            void setOutsideTemp (float temp)
            {
                outsideTempStateSensor->publish_state (temp);
            }
            void setChargeEnergyAdded (float energy)
            {
                ChargeEnergyAddedSensor->publish_state (energy);
            }
            void setChargeMilesAdded (float dist)
            {
                ChargeDistanceAddedSensor->publish_state (dist);
            }

            void set_text_sensor_shift_state (text_sensor::TextSensor *s)
            {
                ShiftStateSensor = static_cast<text_sensor::TextSensor *>(s);
            }
            void set_text_sensor_defrost_state (text_sensor::TextSensor *s)
            {
                DefrostStateSensor = static_cast<text_sensor::TextSensor *>(s);
            }
            void set_text_sensor_charging_state (text_sensor::TextSensor *s)
            {
                ChargingStateSensor = static_cast<text_sensor::TextSensor *>(s);
            }
            void set_text_sensor_last_update_state (text_sensor::TextSensor *s)
            {
                LastUpdateStateSensor = static_cast<text_sensor::TextSensor *>(s);
            }
            void set_sensor_charge_state (sensor::Sensor *s)
            {
                ChargeStateSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_sensor_charge_current_state (sensor::Sensor *s)
            {
                ChargeCurrentStateSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_sensor_charge_voltage_state (sensor::Sensor *s)
            {
                ChargeVoltageStateSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_sensor_charge_power_state (sensor::Sensor *s)
            {
                ChargePowerStateSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_sensor_max_soc_state (sensor::Sensor *s)
            {
                MaxSocStateSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_sensor_max_amps_state (sensor::Sensor *s)
            {
                MaxAmpsStateSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_sensor_mins_to_limit_state (sensor::Sensor *s)
            {
                MinsToLimitStateSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_sensor_battery_range_state (sensor::Sensor *s)
            {
                BatteryRangeStateSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_sensor_odometer_state (sensor::Sensor *s)
            {
                OdometerStateSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_sensor_charge_energy_added_state (sensor::Sensor *s)
            {
                ChargeEnergyAddedSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_sensor_charge_distance_added_state (sensor::Sensor *s)
            {
                ChargeDistanceAddedSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_binary_sensor_is_climate_on (binary_sensor::BinarySensor *s)
            {
                isClimateOnSensor = static_cast<binary_sensor::BinarySensor *>(s);
            }
            void set_binary_sensor_is_boot_open (binary_sensor::BinarySensor *s)
            {
                isBootOpenSensor = static_cast<binary_sensor::BinarySensor *>(s);
            }
            void set_binary_sensor_is_frunk_open (binary_sensor::BinarySensor *s)
            {
                isFrunkOpenSensor = static_cast<binary_sensor::BinarySensor *>(s);
            }
            void set_binary_sensor_windows_state (binary_sensor::BinarySensor *s)
            {
                windowsStateSensor = static_cast<binary_sensor::BinarySensor *>(s);
            }
            void set_sensor_internal_temp_state (sensor::Sensor *s)
            {
                insideTempStateSensor = static_cast<sensor::Sensor *>(s);
            }
            void set_sensor_external_temp_state (sensor::Sensor *s)
            {
                outsideTempStateSensor = static_cast<sensor::Sensor *>(s);
            }

            std::string lookup_shift_state (int shift_state)
            {
                switch (shift_state)
                {
                    case CarServer_ShiftState_Invalid_tag:  return ("Invalid");
                    case CarServer_ShiftState_P_tag:        return ("P");
                    case CarServer_ShiftState_R_tag:        return ("R");
                    case CarServer_ShiftState_N_tag:        return ("N");
                    case CarServer_ShiftState_D_tag:        return ("D");
                    case CarServer_ShiftState_SNA_tag:      return ("SNA");
                }
                return ("Shift state look up error");
            }
            std::string lookup_defrost_state (int defrost_state)
            {
                switch (defrost_state)
                {
                    case CarServer_ClimateState_DefrostMode_Off_tag:       return ("Off");
                    case CarServer_ClimateState_DefrostMode_Normal_tag:    return ("Normal");
                    case CarServer_ClimateState_DefrostMode_Max_tag:       return ("Max");
                }
                return ("Shift state look up error");
            }
            std::string lookup_charging_state (int charging_state)
            {
                switch (charging_state)
                {
                    case CarServer_ChargeState_ChargingState_Unknown_tag:       return ("Unknown");
                    case CarServer_ChargeState_ChargingState_Disconnected_tag:  return ("Disconnected");
                    case CarServer_ChargeState_ChargingState_NoPower_tag:       return ("No Power");
                    case CarServer_ChargeState_ChargingState_Starting_tag:      return ("Starting");
                    case CarServer_ChargeState_ChargingState_Charging_tag:      return ("Charging");
                    case CarServer_ChargeState_ChargingState_Complete_tag:      return ("Complete");
                    case CarServer_ChargeState_ChargingState_Stopped_tag:       return ("Stopped");
                    case CarServer_ChargeState_ChargingState_Calibrating_tag:   return ("Calibrating");
                }
                return ("Charging state look up error");
            }

        protected:
            std::queue<BLERXChunk> ble_read_queue_;
            std::queue<BLEResponse> response_queue_;
            std::queue<BLETXChunk> ble_write_queue_;
            std::queue<BLECommand> command_queue_;

            TeslaBLE::Client *tesla_ble_client_;
            uint32_t storage_handle_;
            uint16_t handle_;
            uint16_t read_handle_{0};
            uint16_t write_handle_{0};

            espbt::ESPBTUUID service_uuid_;
            espbt::ESPBTUUID read_uuid_;
            espbt::ESPBTUUID write_uuid_;

            // sensors
            binary_sensor::BinarySensor *isAsleepSensor;
            binary_sensor::BinarySensor *isUnlockedSensor;
            binary_sensor::BinarySensor *isUserPresentSensor;
            binary_sensor::BinarySensor *isChargeFlapOpenSensor;
            binary_sensor::BinarySensor *isClimateOnSensor;
            binary_sensor::BinarySensor *isBootOpenSensor;
            binary_sensor::BinarySensor *isFrunkOpenSensor;
            binary_sensor::BinarySensor *windowsStateSensor;
            text_sensor::TextSensor *ShiftStateSensor;
            text_sensor::TextSensor *DefrostStateSensor;
            text_sensor::TextSensor *ChargingStateSensor;
            text_sensor::TextSensor *LastUpdateStateSensor;
            sensor::Sensor *ChargeStateSensor;
            sensor::Sensor *OdometerStateSensor;
            sensor::Sensor *ChargeCurrentStateSensor;
            sensor::Sensor *ChargeVoltageStateSensor;
            sensor::Sensor *ChargePowerStateSensor;
            sensor::Sensor *MaxSocStateSensor;
            sensor::Sensor *MaxAmpsStateSensor;
            sensor::Sensor *MinsToLimitStateSensor;
            sensor::Sensor *BatteryRangeStateSensor;
            sensor::Sensor *outsideTempStateSensor;
            sensor::Sensor *insideTempStateSensor;
            sensor::Sensor *ChargeEnergyAddedSensor;
            sensor::Sensor *ChargeDistanceAddedSensor;

            std::vector<unsigned char> ble_read_buffer_;

            void initializeFlash();
            void openNVSHandle();
            void initializePrivateKey();
            void loadSessionInfo();
            void loadDomainSessionInfo(UniversalMessage_Domain domain);
        };

    } // namespace tesla_ble_vehicle
} // namespace esphome
