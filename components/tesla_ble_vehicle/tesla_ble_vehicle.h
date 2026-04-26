#pragma once

#include <algorithm>
#include <cstring>
#include <iterator>
#include <vector>
#include <queue>
#include <array>
#include <unordered_map>
#include <functional>

#include <esp_gattc_api.h>
#include <esphome/components/binary_sensor/binary_sensor.h>
#include <esphome/components/text_sensor/text_sensor.h>
#include <esphome/components/sensor/sensor.h>
#include <esphome/components/ble_client/ble_client.h>
#include <esphome/components/esp32_ble_tracker/esp32_ble_tracker.h>
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

namespace esphome
{

    namespace tesla_ble_vehicle
    {
        namespace espbt = esphome::esp32_ble_tracker;

        enum class BLE_CarServer_VehicleAction
        {
            DO_NOTHING, // Blank empty entry
            GET_CHARGE_STATE,
            GET_CLIMATE_STATE,
            GET_DRIVE_STATE,
            GET_LOCATION_STATE,
            GET_CLOSURES_STATE,
            GET_TYRES_STATE,
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
            DEFROST_CAR,
            SET_CLIMATE_TEMP,
            MEDIA_PLAY,
            MEDIA_NEXT_TRACK,
            MEDIA_PREVIOUS_TRACK,
            _COUNT  // sentinel value to get count of entries
        };
        enum class AllowedMsg // The type of messages to send
        {
            VehicleActionMessage,
            GetVehicleDataMessage,
            Empty
        };

        enum class GetOnSet
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
        };

        struct ActionMessageDetail
        /*
        *   This defines the contents of the rows of the ACTION_SPECIFICS table below. For every localActionDef, this describes the specific
        *   contents for the message to send to the car. The rows of the table must be in the same order as BLE_CarServer_VehicleAction.
        *   If a new message is needed, simply add a row to the table with the appropriate contents, no need to edit the code in the .cpp.
        */
        {
            BLE_CarServer_VehicleAction localActionDef;
            const char* action_str;
            AllowedMsg whichMsg;
            int actionTag;
            GetOnSet getOnSet;
            int numberUpdatesBetweenGets; // Only used for GetVehicleDataMessage
        };
        static constexpr std::array<ActionMessageDetail, 23> ACTION_SPECIFICS // Don't forget to increase the size when adding a row
        {{
            {BLE_CarServer_VehicleAction::DO_NOTHING,                       "",                          AllowedMsg::Empty,                 0,                                                              GetOnSet::Invalid,         0},
            {BLE_CarServer_VehicleAction::GET_CHARGE_STATE,                 "getChargeState",            AllowedMsg::GetVehicleDataMessage, CarServer_GetVehicleData_getChargeState_tag,                    GetOnSet::Invalid,         1},
            {BLE_CarServer_VehicleAction::GET_CLIMATE_STATE,                "getClimateState",           AllowedMsg::GetVehicleDataMessage, CarServer_GetVehicleData_getClimateState_tag,                   GetOnSet::Invalid,         5},
            {BLE_CarServer_VehicleAction::GET_DRIVE_STATE,                  "getDriveState",             AllowedMsg::GetVehicleDataMessage, CarServer_GetVehicleData_getDriveState_tag,                     GetOnSet::Invalid,         1},
            {BLE_CarServer_VehicleAction::GET_LOCATION_STATE,               "getLocationState",          AllowedMsg::GetVehicleDataMessage, CarServer_GetVehicleData_getLocationState_tag,                  GetOnSet::Invalid,         10},
            {BLE_CarServer_VehicleAction::GET_CLOSURES_STATE,               "getClosuresState",          AllowedMsg::GetVehicleDataMessage, CarServer_GetVehicleData_getClosuresState_tag,                  GetOnSet::Invalid,         6},
            {BLE_CarServer_VehicleAction::GET_TYRES_STATE,                  "getTyresState",             AllowedMsg::GetVehicleDataMessage, CarServer_GetVehicleData_getTirePressureState_tag,              GetOnSet::Invalid,         17},
            {BLE_CarServer_VehicleAction::SET_CHARGING_SWITCH,              "setChargingSwitch",         AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_chargingStartStopAction_tag,            GetOnSet::GetChargeState,  0},
            {BLE_CarServer_VehicleAction::SET_CHARGING_AMPS,                "setChargingAmps",           AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_setChargingAmpsAction_tag,              GetOnSet::GetChargeState,  0},
            {BLE_CarServer_VehicleAction::SET_CHARGING_LIMIT,               "setChargingLimit",          AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_chargingSetLimitAction_tag,             GetOnSet::GetChargeState,  0},
            {BLE_CarServer_VehicleAction::SET_SENTRY_SWITCH,                "setSentrySwitch",           AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_vehicleControlSetSentryModeAction_tag,  GetOnSet::Invalid,         0},
            {BLE_CarServer_VehicleAction::SET_HVAC_SWITCH,                  "setHVACSwitch",             AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_hvacAutoAction_tag,                     GetOnSet::GetClimateState, 0},
            {BLE_CarServer_VehicleAction::SET_HVAC_STEERING_HEATER_SWITCH,  "setHVACSteeringHeatSwitch", AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_hvacSteeringWheelHeaterAction_tag,      GetOnSet::Invalid,         0},
            {BLE_CarServer_VehicleAction::SET_OPEN_CHARGE_PORT_DOOR,        "setOpenChargePortDoor",     AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_chargePortDoorOpen_tag,                 GetOnSet::Invalid,         0},
            {BLE_CarServer_VehicleAction::SET_CLOSE_CHARGE_PORT_DOOR,       "setCloseChargePortDoor",    AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_chargePortDoorClose_tag,                GetOnSet::Invalid,         0},
            {BLE_CarServer_VehicleAction::SOUND_HORN,                       "soundHorn",                 AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_vehicleControlHonkHornAction_tag,       GetOnSet::Invalid,         0},
            {BLE_CarServer_VehicleAction::FLASH_LIGHT,                      "flashLight",                AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_vehicleControlFlashLightsAction_tag,    GetOnSet::Invalid,         0},
            {BLE_CarServer_VehicleAction::SET_WINDOWS_SWITCH,               "setWindowsSwitch",          AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_vehicleControlWindowAction_tag,         GetOnSet::GetClosureState, 0},
            {BLE_CarServer_VehicleAction::DEFROST_CAR,                      "defrostCar",                AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_hvacSetPreconditioningMaxAction_tag,    GetOnSet::GetClimateState, 0},
            {BLE_CarServer_VehicleAction::SET_CLIMATE_TEMP,                 "setClimateTemp",            AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_hvacTemperatureAdjustmentAction_tag,    GetOnSet::GetClimateState, 0},
            {BLE_CarServer_VehicleAction::MEDIA_PLAY,                       "mediaPlay",                 AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_mediaPlayAction_tag,                    GetOnSet::Invalid,         0},
            {BLE_CarServer_VehicleAction::MEDIA_NEXT_TRACK,                 "mediaNextTrack",            AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_mediaNextTrack_tag,                     GetOnSet::Invalid,         0},
            {BLE_CarServer_VehicleAction::MEDIA_PREVIOUS_TRACK,             "mediaPreviousTrack",        AllowedMsg::VehicleActionMessage,  CarServer_VehicleAction_mediaPreviousTrack_tag,                 GetOnSet::Invalid,         0}
        }};
        static_assert(ACTION_SPECIFICS.size() == static_cast<std::size_t>(BLE_CarServer_VehicleAction::_COUNT), "ACTION_SPECIFICS out of sync with enum");
        static const char *const TAG = "tesla_ble_vehicle";
        static const char *nvs_key_infotainment = "tk_infotainment";
        static const char *nvs_key_vcsec = "tk_vcsec";

        static const char *const SERVICE_UUID = "00000211-b2d1-43f0-9b88-960cebf8b91e";
        static const char *const READ_UUID = "00000213-b2d1-43f0-9b88-960cebf8b91e";
        static const char *const WRITE_UUID = "00000212-b2d1-43f0-9b88-960cebf8b91e";

        static const int PRIVATE_KEY_SIZE = 228;
        static const int PUBLIC_KEY_SIZE = 65;
        static const int MAX_BLE_MESSAGE_SIZE = 4608; // Max size of a BLE message
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

            BLECommand(UniversalMessage_Domain d, std::function<int()> e, std::string n = "", BLE_CarServer_VehicleAction a = BLE_CarServer_VehicleAction::DO_NOTHING)
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
        typedef enum // connected, disconnected, disconnected and Unknowns have been set
        {
            BleConnected,
            BleDisconnected,
            BleDisconnectedUnknownsSet
        } BleConnectedStatus;
        typedef enum // connected, disconnected, disconnected and Unknowns have been set
        {
            NotCharging,
            ChargingJustStarted,
            ChargingOngoing
        } ChargingStatus;

        enum class BinarySensorId : uint8_t {
            IsAsleep,
            IsUnlocked,
            IsUserPresent,
            IsChargeFlapOpen,
            IsBootOpen,
            IsFrunkOpen,
            IsClimateOn,
            WindowsState,
            Count
        };
        enum class TextSensorId : uint8_t {
            ShiftState,
            DefrostState,
            ChargingState,
            ChargePortLatchState,
            LastUpdate,
            Count
        };
        enum class NumericSensorId : uint8_t {
            ChargeState,
            Odometer,
            ChargeCurrent,
            ChargeVoltage,
            ChargePower,
            MaxSoc,
            MaxAmps,
            MinsToLimit,
            BatteryRange,
            InternalTemp,
            ExternalTemp,
            ChargeEnergyAdded,
            ChargeDistanceAdded,
            TpmsFl,
            TpmsFr,
            TpmsRl,
            TpmsRr,
            BleDisconnectedTime,
            ChargerPhases,
            ChargeRate,
            DriverTemp,
            Count
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
            int car_is_charging_ = NotCharging;
            bool do_poll_ = false;
            BleConnectedStatus ble_disconnected_ = BleDisconnected;
            int ble_disconnected_time_;
            int ble_disconnected_min_time_;
            int fast_poll_if_unlocked_ = 1; // != 0 enables fast polling
            int number_updates_since_connection_ = 0;
            UniversalMessage_RoutableMessage read_queue_message_;
            CarServer_Response static_carserver_response_;
            unsigned char static_message_buffer_[UniversalMessage_RoutableMessage_size];
            //BLETXChunk static_tx_chunk_;
            //BLERXChunk static_rx_chunk_;

            TeslaBLEVehicle();
            void setup() override;
            void loop() override;
            void update() override;
            void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                     esp_ble_gattc_cb_param_t *param) override;
            void dump_config() override;
            void set_nvs_namespace(const std::string &ns);
            void set_vin(const char *vin);
            void load_polling_parameters (const int post_wake_poll_time, const int poll_data_period,
                                          const int poll_asleep_period, const int poll_charging_period,
                                          const int ble_disconnected_min_time, const int fast_poll_if_unlocked,
                                          const int wake_on_boot);
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

            int handleInfoCarServerResponse (const CarServer_Response& carserver_response);
            int handleSessionInfoUpdate(const UniversalMessage_RoutableMessage& message, UniversalMessage_Domain domain);
            int handleVCSECVehicleStatus(VCSEC_VehicleStatus vehicleStatus);

            int wakeVehicle(void);
            int lockVehicle (VCSEC_RKEAction_E lock);
            void placeAtFrontOfQueue (UniversalMessage_Domain domain, std::function<int()> execute, std::string execute_name, BLE_CarServer_VehicleAction action = BLE_CarServer_VehicleAction::DO_NOTHING);
    
            int sendVCSECActionMessage(VCSEC_RKEAction_E action);
            int sendVCSECClosureMoveRequestMessage (int moveWhat, VCSEC_ClosureMoveType_E moveType);
            int sendCarServerVehicleActionMessage(BLE_CarServer_VehicleAction action, int param);
            int sendSessionInfoRequest(UniversalMessage_Domain domain);
            int sendVCSECInformationRequest(void);
            void enqueueVCSECInformationRequest(bool force = false);
            int wake_on_boot_ = 0; // != 0 wakes car on device boot

            int writeBLE(const unsigned char *message_buffer, size_t message_length,
                         esp_gatt_write_type_t write_type, esp_gatt_auth_req_t auth_req);

            inline const ActionMessageDetail& get_action_detail (BLE_CarServer_VehicleAction action)
            { // Get the entry in the ACTION_SPECIFICS table corresponding to the action (we can't be sure of the order)
                return ACTION_SPECIFICS[static_cast<size_t>(action)];
            }
            // sensors
            // set sensors to unknown (e.g. when vehicle is disconnected)
            void setSensors(bool has_state) // has_state is an anachronism
            {
                for (auto* s : numeric_sensors_)
                    if (s) s->publish_state(NAN);
                for (auto* s : text_sensors_)
                    if (s) s->publish_state("Unknown");
                for (auto* s : binary_sensors_)
                    if (s) s->invalidate_state();
            }
            template<typename T, typename V>
            inline void publish_if (T* sensor, const V& value) {
                if (sensor) sensor->publish_state (value);
            }
            inline void publishSensor (BinarySensorId id, bool value) {
                publish_if (binary_sensors_[static_cast<size_t>(id)], value);
            }

            inline void publishSensor (TextSensorId id, const std::string& value) {
                publish_if (text_sensors_[static_cast<size_t>(id)], value);
            }

            inline void publishSensor (NumericSensorId id, float value) {
                publish_if (numeric_sensors_[static_cast<size_t>(id)], value);
            }
            void set_binary_sensor (BinarySensorId id, binary_sensor::BinarySensor* s) {
                binary_sensors_[static_cast<size_t>(id)] = s;
            }
            void set_text_sensor (TextSensorId id, text_sensor::TextSensor* s) {
                text_sensors_[static_cast<size_t>(id)] = s;
            }
            void set_numeric_sensor (NumericSensorId id, sensor::Sensor* s) {
                numeric_sensors_[static_cast<size_t>(id)] = s;
            }
            inline static constexpr std::pair<int, const char*> SHIFT_MAP[] = {
                {CarServer_ShiftState_Invalid_tag,  "Invalid"},
                {CarServer_ShiftState_P_tag,        "P"},
                {CarServer_ShiftState_R_tag,        "R"},
                {CarServer_ShiftState_N_tag,        "N"},
                {CarServer_ShiftState_D_tag,        "D"},
                {CarServer_ShiftState_SNA_tag,      "SNA"},
            };
            std::string lookup_shift_state (int state)
            {
                for (const auto& entry : SHIFT_MAP)
                {
                    if (entry.first == state)
                        return entry.second;
                }
                return "Shift state look up error";
            }
            inline static constexpr std::pair<int, const char*> DEFROST_MAP[] = {
                {CarServer_ClimateState_DefrostMode_Off_tag,    "Off"},
                {CarServer_ClimateState_DefrostMode_Normal_tag, "Normal"},
                {CarServer_ClimateState_DefrostMode_Max_tag,    "Max"},
            };
            std::string lookup_defrost_state (int state)
            {
                for (const auto& entry : DEFROST_MAP)
                {
                    if (entry.first == state)
                        return entry.second;
                }
                return "Defrost state look up error";
            }
            inline static constexpr std::pair<int, const char*> CHARGING_STATE_MAP[] = {
                {CarServer_ChargeState_ChargingState_Unknown_tag,       "Unknown"},
                {CarServer_ChargeState_ChargingState_Disconnected_tag,  "Disconnected"},
                {CarServer_ChargeState_ChargingState_NoPower_tag,       "No Power"},
                {CarServer_ChargeState_ChargingState_Starting_tag,      "Starting"},
                {CarServer_ChargeState_ChargingState_Charging_tag,      "Charging"},
                {CarServer_ChargeState_ChargingState_Complete_tag,      "Complete"},
                {CarServer_ChargeState_ChargingState_Stopped_tag,       "Stopped"},
                {CarServer_ChargeState_ChargingState_Calibrating_tag,   "Calibrating"},
            };
            std::string lookup_charging_state (int state)
            {
                for (const auto& entry : CHARGING_STATE_MAP)
                {
                    if (entry.first == state)
                        return entry.second;
                }
                return "Charging state look up error";
            }
            inline static constexpr std::pair<int, const char*> CHARGE_PORT_LATCH_STATE_MAP[] = {
                {CarServer_ChargePortLatchState_SNA_tag,        "SNA"},
                {CarServer_ChargePortLatchState_Disengaged_tag, "Disengaged"},
                {CarServer_ChargePortLatchState_Engaged_tag,    "Engaged"},
                {CarServer_ChargePortLatchState_Blocking_tag,   "Blocking"},
            };
            std::string lookup_charge_port_latch_state (int state)
            {
                for (const auto& entry : CHARGE_PORT_LATCH_STATE_MAP)
                {
                    if (entry.first == state)
                        return entry.second;
                }
                return "Charge port latch state look up error";
            }

        protected:
            std::queue<BLERXChunk> ble_read_queue_;
            std::queue<BLEResponse> response_queue_;
            std::queue<BLETXChunk> ble_write_queue_;
            std::queue<BLECommand> command_queue_;

            TeslaBLE::Client *tesla_ble_client_;
            uint32_t storage_handle_;
            std::string nvs_namespace_;
            uint16_t handle_;
            uint16_t read_handle_{0};
            uint16_t write_handle_{0};

            espbt::ESPBTUUID service_uuid_;
            espbt::ESPBTUUID read_uuid_;
            espbt::ESPBTUUID write_uuid_;

            // sensors
            std::array<binary_sensor::BinarySensor*, static_cast<size_t>(BinarySensorId::Count)> binary_sensors_{};

            std::array<text_sensor::TextSensor*, static_cast<size_t>(TextSensorId::Count)> text_sensors_{};

            std::array<sensor::Sensor*, static_cast<size_t>(NumericSensorId::Count)> numeric_sensors_{};

            std::vector<unsigned char> ble_read_buffer_;

            void initializeFlash();
            void openNVSHandle();
            void initializePrivateKey();
            void loadSessionInfo();
            void loadDomainSessionInfo(UniversalMessage_Domain domain);
        };

    } // namespace tesla_ble_vehicle
} // namespace esphome
