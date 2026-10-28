#include <inttypes.h>
#include <string.h>

//#include <esphome/core/helpers.h>
//#include <esphome/core/log.h>

#include <car_server.pb.h>
#include <signatures.pb.h>
#include <universal_message.pb.h>
#include <vcsec.pb.h>
#include <client.h>
#include "tesla_ble_vehicle.h"

namespace esphome {
namespace tesla_ble_vehicle {

int TeslaBLEVehicle::actions_buildCarServerGetVehicleDataMessage (
	pb_byte_t *output_buffer,
    size_t *output_length,
    int which_get)
/*
*   Function to build a CarServer_GetVehicleData message.
*/
{
	// Build generic part
	actions_action_message_.which_action_msg                                           = CarServer_Action_vehicleAction_tag;
	actions_action_message_.action_msg.vehicleAction                                   = CarServer_VehicleAction_init_default;
	actions_action_message_.action_msg.vehicleAction.which_vehicle_action_msg          = CarServer_VehicleAction_getVehicleData_tag;

	auto& vehicle_action_msg_          = actions_action_message_.action_msg.vehicleAction.vehicle_action_msg; // For legibility
	vehicle_action_msg_.getVehicleData = CarServer_GetVehicleData_init_default;
	// Now the get specific part
	switch (which_get)
	{
	case CarServer_GetVehicleData_getChargeState_tag:
		vehicle_action_msg_.getVehicleData.getChargeState     = CarServer_GetChargeState_init_default;
		vehicle_action_msg_.getVehicleData.has_getChargeState = true;
		break;
	case CarServer_GetVehicleData_getClimateState_tag:
		vehicle_action_msg_.getVehicleData.getClimateState     = CarServer_GetClimateState_init_default;
		vehicle_action_msg_.getVehicleData.has_getClimateState = true;
		break;
	case CarServer_GetVehicleData_getDriveState_tag:
		vehicle_action_msg_.getVehicleData.getDriveState     = CarServer_GetDriveState_init_default;
		vehicle_action_msg_.getVehicleData.has_getDriveState = true;
		break;
	case CarServer_GetVehicleData_getLocationState_tag:
		vehicle_action_msg_.getVehicleData.getLocationState     = CarServer_GetLocationState_init_default;
		vehicle_action_msg_.getVehicleData.has_getLocationState = true;
		break;
	case CarServer_GetVehicleData_getClosuresState_tag:
		vehicle_action_msg_.getVehicleData.getClosuresState     = CarServer_GetClosuresState_init_default;
		vehicle_action_msg_.getVehicleData.has_getClosuresState = true;
		break;
	case CarServer_GetVehicleData_getTirePressureState_tag:
		vehicle_action_msg_.getVehicleData.getTirePressureState     = CarServer_GetTirePressureState_init_default;
		vehicle_action_msg_.getVehicleData.has_getTirePressureState = true;
		break;
	case CarServer_GetVehicleData_getChargeScheduleState_tag:
		vehicle_action_msg_.getVehicleData.getChargeScheduleState     = CarServer_GetChargeScheduleState_init_default;
		vehicle_action_msg_.getVehicleData.has_getChargeScheduleState = true;
		break;
	case CarServer_GetVehicleData_getSoftwareUpdateState_tag:
		vehicle_action_msg_.getVehicleData.getSoftwareUpdateState     = CarServer_GetSoftwareUpdateState_init_default;
		vehicle_action_msg_.getVehicleData.has_getSoftwareUpdateState = true;
		break;
	default:
		ESP_LOGE (TAG, "Invalid which_get type, action message not built");
		return TeslaBLE::TeslaBLE_Status_E_ERROR_INTERNAL;
	}
	// Add it to the message
	int status = tesla_ble_client_->buildCarServerActionPayload (&actions_action_message_, output_buffer, output_length);
	if (status != TeslaBLE::TeslaBLE_Status_E_OK)
	{
		ESP_LOGE (TAG, "[buildCarServerGetVehicleDataMessage] Failed to build car action message");
		return status;
	}
	return TeslaBLE::TeslaBLE_Status_E_OK;
}


int TeslaBLEVehicle::actions_buildCarServerVehicleActionMessage (
	int32_t set_value,
	pb_byte_t *output_buffer,
	size_t *output_length,
	int which_tag,
	uint64_t long_param)
{
	// Build generic part action.action_msg.vehicleAction
	actions_action_message_                                                   = CarServer_Action_init_default;
	actions_action_message_.which_action_msg                                  = CarServer_Action_vehicleAction_tag;
	actions_action_message_.action_msg.vehicleAction                          = CarServer_VehicleAction_init_default;
	actions_action_message_.action_msg.vehicleAction.which_vehicle_action_msg = which_tag;
	// Now the  specific part.
	auto& vehicle_action_msg_ = actions_action_message_.action_msg.vehicleAction.vehicle_action_msg; // For legibility
	switch (which_tag)
	{
	case CarServer_VehicleAction_setChargingAmpsAction_tag: 
		vehicle_action_msg_.setChargingAmpsAction               = CarServer_SetChargingAmpsAction_init_default;
		vehicle_action_msg_.setChargingAmpsAction.charging_amps = set_value;
		break;
	case CarServer_VehicleAction_chargingSetLimitAction_tag:
		vehicle_action_msg_.chargingSetLimitAction         = CarServer_ChargingSetLimitAction_init_default;
		vehicle_action_msg_.chargingSetLimitAction.percent = set_value;
		break;
	case CarServer_VehicleAction_chargingStartStopAction_tag:
		vehicle_action_msg_.chargingStartStopAction = CarServer_ChargingStartStopAction_init_default;
		if (set_value == 1)
		{
		vehicle_action_msg_.chargingStartStopAction.which_charging_action = CarServer_ChargingStartStopAction_start_tag;
		vehicle_action_msg_.chargingStartStopAction.charging_action.start = CarServer_Void_init_default;
		}
		else
		{
		vehicle_action_msg_.chargingStartStopAction.which_charging_action = CarServer_ChargingStartStopAction_stop_tag;
		vehicle_action_msg_.chargingStartStopAction.charging_action.stop  = CarServer_Void_init_default;
		}
		break;
	case CarServer_VehicleAction_vehicleControlSetSentryModeAction_tag:
		vehicle_action_msg_.vehicleControlSetSentryModeAction    = CarServer_VehicleControlSetSentryModeAction_init_default;
		vehicle_action_msg_.vehicleControlSetSentryModeAction.on = (set_value != 0);
		break;
	case CarServer_VehicleAction_hvacAutoAction_tag:
		vehicle_action_msg_.hvacAutoAction          = CarServer_HvacAutoAction_init_default;
		vehicle_action_msg_.hvacAutoAction.power_on = (set_value != 0);
		break;
	case CarServer_VehicleAction_hvacSteeringWheelHeaterAction_tag:
		vehicle_action_msg_.hvacSteeringWheelHeaterAction          = CarServer_HvacSteeringWheelHeaterAction_init_default;
		vehicle_action_msg_.hvacSteeringWheelHeaterAction.power_on = (set_value != 0);
		break;
	case CarServer_VehicleAction_chargePortDoorOpen_tag:
		vehicle_action_msg_.chargePortDoorOpen             = CarServer_ChargePortDoorOpen_init_default;
		vehicle_action_msg_.chargePortDoorOpen.dummy_field = 1;
		break;
	case CarServer_VehicleAction_chargePortDoorClose_tag:
		vehicle_action_msg_.chargePortDoorClose             = CarServer_ChargePortDoorClose_init_default;
		vehicle_action_msg_.chargePortDoorClose.dummy_field = 1;
		break;
	case CarServer_VehicleAction_vehicleControlFlashLightsAction_tag:
		vehicle_action_msg_.vehicleControlFlashLightsAction             = CarServer_VehicleControlFlashLightsAction_init_default;
		vehicle_action_msg_.vehicleControlFlashLightsAction.dummy_field = 1;
		break;
	case CarServer_VehicleAction_vehicleControlHonkHornAction_tag:
		vehicle_action_msg_.vehicleControlHonkHornAction             = CarServer_VehicleControlHonkHornAction_init_default;
		vehicle_action_msg_.vehicleControlHonkHornAction.dummy_field = 1;
		break;
	case CarServer_VehicleAction_vehicleControlWindowAction_tag:
		vehicle_action_msg_.vehicleControlWindowAction = CarServer_VehicleControlWindowAction_init_default;
		if (set_value == 1)
		{
		vehicle_action_msg_.vehicleControlWindowAction.which_action = CarServer_VehicleControlWindowAction_vent_tag;
		vehicle_action_msg_.vehicleControlWindowAction.action.vent  = CarServer_Void_init_default;
		}
		else
		{
		vehicle_action_msg_.vehicleControlWindowAction.which_action = CarServer_VehicleControlWindowAction_close_tag;
		vehicle_action_msg_.vehicleControlWindowAction.action.close = CarServer_Void_init_default;
		}
		break;
	case CarServer_VehicleAction_hvacSetPreconditioningMaxAction_tag:
		vehicle_action_msg_.hvacSetPreconditioningMaxAction    = CarServer_HvacSetPreconditioningMaxAction_init_default;
		vehicle_action_msg_.hvacSetPreconditioningMaxAction.on = (set_value != 0);
		break;
	case CarServer_VehicleAction_hvacTemperatureAdjustmentAction_tag:
		vehicle_action_msg_.hvacTemperatureAdjustmentAction                       = CarServer_HvacTemperatureAdjustmentAction_init_default;
		vehicle_action_msg_.hvacTemperatureAdjustmentAction.driver_temp_celsius   = set_value;
		vehicle_action_msg_.hvacTemperatureAdjustmentAction.passenger_temp_celsius = set_value;
		break;
	case CarServer_VehicleAction_mediaPlayAction_tag:
		vehicle_action_msg_.mediaPlayAction             = CarServer_MediaPlayAction_init_default;
		vehicle_action_msg_.mediaPlayAction.dummy_field = 1;
		break;
	case CarServer_VehicleAction_mediaNextTrack_tag:
		vehicle_action_msg_.mediaNextTrack             = CarServer_MediaNextTrack_init_default;
		vehicle_action_msg_.mediaNextTrack.dummy_field = 1;
		break;
	case CarServer_VehicleAction_mediaPreviousTrack_tag:
		vehicle_action_msg_.mediaPreviousTrack             = CarServer_MediaPreviousTrack_init_default;
		vehicle_action_msg_.mediaPreviousTrack.dummy_field = 1;
		break;
	case CarServer_VehicleAction_setLowPowerModeAction_tag:
		vehicle_action_msg_.setLowPowerModeAction                = CarServer_SetLowPowerModeAction_init_default;
		vehicle_action_msg_.setLowPowerModeAction.low_power_mode = (set_value != 0);
		break;
	case CarServer_VehicleAction_setKeepAccessoryPowerModeAction_tag:
		vehicle_action_msg_.setKeepAccessoryPowerModeAction                           = CarServer_SetKeepAccessoryPowerModeAction_init_default;
		vehicle_action_msg_.setKeepAccessoryPowerModeAction.keep_accessory_power_mode = (set_value != 0);
		break;
	case CarServer_VehicleAction_removeChargeScheduleAction_tag:
		vehicle_action_msg_.removeChargeScheduleAction    = CarServer_RemoveChargeScheduleAction_init_default;
		vehicle_action_msg_.removeChargeScheduleAction.id = long_param;
		break;
	case CarServer_VehicleAction_setCabinOverheatProtectionAction_tag :
		vehicle_action_msg_.setCabinOverheatProtectionAction    = CarServer_SetCabinOverheatProtectionAction_init_default;
		switch (set_value)
		{
		// Note the Tesla tags do not include an "off" tag hence "hard-coded" 0 here
		case 0:
			break; // No need to do anything as init_default sets it to off
		case CarServer_SetCabinOverheatProtectionAction_on_tag:
			vehicle_action_msg_.setCabinOverheatProtectionAction.on       = true;
			vehicle_action_msg_.setCabinOverheatProtectionAction.fan_only = false;
			break;
		case CarServer_SetCabinOverheatProtectionAction_fan_only_tag:
			vehicle_action_msg_.setCabinOverheatProtectionAction.on       = true;
			vehicle_action_msg_.setCabinOverheatProtectionAction.fan_only = true;
			break;
		default:
			ESP_LOGE (TAG, "Tag type %i, unexpected overheat set_value received %li", which_tag, set_value);
			return TeslaBLE::TeslaBLE_Status_E_ERROR_INTERNAL;
		}
		break;
	case CarServer_VehicleAction_setCopTempAction_tag :
		vehicle_action_msg_.setCopTempAction 				   = CarServer_SetCopTempAction_init_default;
		vehicle_action_msg_.setCopTempAction.copActivationTemp = static_cast<CarServer_ClimateState_CopActivationTemp>(set_value);
		break;
	default:
		ESP_LOGE (TAG, "Tag type %i, car server vehicle action message not built", which_tag);
		return TeslaBLE::TeslaBLE_Status_E_ERROR_INTERNAL;
	}
	// Add it to the message
	int status = tesla_ble_client_->buildCarServerActionPayload(&actions_action_message_, output_buffer, output_length);
	if (status != TeslaBLE::TeslaBLE_Status_E_OK)
	{
		ESP_LOGE (TAG, "Failed to build car server vehicle action message");
		return status;
	}
	return TeslaBLE::TeslaBLE_Status_E_OK;
}
int TeslaBLEVehicle::actions_setChargeSchedule (
	std::string name,
    int days_of_week,
    bool start_enabled,
    int start_time,
    bool end_enabled,
    int end_time,
    bool one_time,
    bool enabled,
    float latitude,
    float longitude)
{
	if (allow_setting_schedules_ == 0)
	{
		ESP_LOGW (TAG, "Setting charge schedules disabled. Set allow_setting_schedules in your yaml to non-zero and rebuild to enable.");
		return 0;
	}
    if ((days_of_week <= 0)  or (days_of_week > 127) or
        (start_time < 0)     or (start_time > 1440)  or
        (end_time < 0)       or (end_time > 1440) or
        (latitude < -90.0)   or (latitude > 90.0) or
        (longitude < -90.0)  or (longitude > 90.0))
    { // Do some basic validation on the numbers provided so impossible values rejected
        ESP_LOGE (TAG, "Set charge schedule command rejected, one or more of days_of_week, start_time, end_time, latitude and longitude out of range");
        return TeslaBLE::TeslaBLE_Status_E_ERROR_INTERNAL;
    }
    std::function<int()> execute_cmd;
    execute_cmd = [this, name, days_of_week, start_enabled, start_time, end_enabled, end_time, one_time, enabled, latitude, longitude]()
    {
        // Build generic part action.action_msg.vehicleAction
        actions_action_message_                                                   = CarServer_Action_init_default;
        actions_action_message_.which_action_msg                                  = CarServer_Action_vehicleAction_tag;
        actions_action_message_.action_msg.vehicleAction                          = CarServer_VehicleAction_init_default;
        actions_action_message_.action_msg.vehicleAction.which_vehicle_action_msg = CarServer_VehicleAction_addChargeScheduleAction_tag;
        // Now the  specific part.
        auto& schedule = actions_action_message_.action_msg.vehicleAction.vehicle_action_msg.addChargeScheduleAction; // For legibility
        if (name.length() >= sizeof(schedule.name))
        {
            ESP_LOGW(TAG, "Charge schedule name too long (%d characters), name will be truncated", name.length());
        }
        schedule.id =             static_cast<uint64_t>(time(nullptr));
        strncpy(schedule.name, name.c_str(), sizeof(schedule.name) - 1);
        schedule.name[sizeof(schedule.name) - 1] = '\0';
        schedule.days_of_week =   days_of_week;
        schedule.start_enabled =  start_enabled;
        schedule.start_time =     start_time;
        schedule.end_enabled =    end_enabled;
        schedule.end_time =       end_time;
        schedule.one_time =       one_time;
        schedule.enabled =        enabled;
        schedule.latitude =       latitude;
        schedule.longitude =      longitude;

  		ESP_LOGW (TAG,
          "Schedule details to create:\n"
          " id = %" PRIu64 "\n"
          " name = %s\n"
          " days_of_week = %" PRId32"\n"
          " start_enabled = %i\n"
          " start_time = %" PRId32"\n"
          " end_enabled = %i\n"
          " end_time = %" PRId32"\n"
          " one_time = %i\n"
          " enabled = %i\n"
          " latitude = %f\n"
          " longitude = %f",
          schedule.id,
          schedule.name,
          schedule.days_of_week,
          schedule.start_enabled,
          schedule.start_time,
          schedule.end_enabled,
          schedule.end_time,
          schedule.one_time,
          schedule.enabled,
          schedule.latitude,
          schedule.longitude);

        // Build the message (Lots of repetition here, will need tidying up.)
        size_t message_length = 0;
        int status = tesla_ble_client_->buildCarServerActionPayload (&actions_action_message_, static_message_buffer_, &message_length);
        if (status != TeslaBLE::TeslaBLE_Status_E_OK)
        {
            ESP_LOGE (TAG, "Failed to build car server vehicle action message for setting charge schedule");
            auto session = tesla_ble_client_->getPeer (UniversalMessage_Domain_DOMAIN_INFOTAINMENT);
            if (status == TeslaBLE::TeslaBLE_Status_E_ERROR_INVALID_SESSION)
            {
            	invalidateSession (UniversalMessage_Domain_DOMAIN_INFOTAINMENT);
            }
            return status;
        }
        status = writeBLE (static_message_buffer_, message_length, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
        if (status != 0)
        {
          	ESP_LOGE (TAG, "Failed to send set charge schedule message");
        	return status;
        }
        return static_cast<int>(TeslaBLE::TeslaBLE_Status_E_OK);
      };
      ESP_LOGW (TAG, "[%s] Adding command to queue (days_of_week = %d)", "set_charge_schedule", days_of_week);
      placeAtFrontOfQueue (UniversalMessage_Domain_DOMAIN_INFOTAINMENT, execute_cmd, "set_charge_schedule");
      return TeslaBLE::TeslaBLE_Status_E_OK;
    }

} // esphome
} // tesla_ble_vehicle
