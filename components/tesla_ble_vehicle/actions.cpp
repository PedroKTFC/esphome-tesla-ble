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
	case CarServer_GetVehicleData_getPreconditioningScheduleState_tag:
		vehicle_action_msg_.getVehicleData.getPreconditioningScheduleState     = CarServer_GetPreconditioningScheduleState_init_default;
		vehicle_action_msg_.getVehicleData.has_getPreconditioningScheduleState = true;
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
	case CarServer_VehicleAction_removePreconditionScheduleAction_tag:
		vehicle_action_msg_.removePreconditionScheduleAction    = CarServer_RemovePreconditionScheduleAction_init_default;
		vehicle_action_msg_.removePreconditionScheduleAction.id = long_param;
		break;
	case CarServer_VehicleAction_scheduledChargingAction_tag:
		// set_value is minutes after midnight, a negative value disables scheduled charging
		vehicle_action_msg_.scheduledChargingAction               = CarServer_ScheduledChargingAction_init_default;
		vehicle_action_msg_.scheduledChargingAction.enabled       = (set_value >= 0);
		vehicle_action_msg_.scheduledChargingAction.charging_time = (set_value >= 0) ? set_value : 0;
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
int TeslaBLEVehicle::actions_queueBuiltAction (BLE_CarServer_VehicleAction action, std::function<void(CarServer_VehicleAction&)> fill)
/*
*   Queues a vehicle action whose message is too complex for actions_buildCarServerVehicleActionMessage (eg schedules). fill() sets
*   the action specific part of the message. The action's ACTION_SPECIFICS row determines the tag and the get done afterwards.
*/
{
	std::string action_str = get_action_detail(action).action_str;
	std::function<int()> execute_cmd = [this, action, action_str, fill]()
	{
		actions_action_message_                                                   = CarServer_Action_init_default;
		actions_action_message_.which_action_msg                                  = CarServer_Action_vehicleAction_tag;
		actions_action_message_.action_msg.vehicleAction                          = CarServer_VehicleAction_init_default;
		actions_action_message_.action_msg.vehicleAction.which_vehicle_action_msg = get_action_detail(action).actionTag;
		fill (actions_action_message_.action_msg.vehicleAction);

		size_t message_length = 0;
		int status = tesla_ble_client_->buildCarServerActionPayload (&actions_action_message_, static_message_buffer_, &message_length);
		if (status != TeslaBLE::TeslaBLE_Status_E_OK)
		{
			ESP_LOGE (TAG, "[%s] Failed to build car server vehicle action message", action_str.c_str());
			if (status == TeslaBLE::TeslaBLE_Status_E_ERROR_INVALID_SESSION)
			{
				invalidateSession (UniversalMessage_Domain_DOMAIN_INFOTAINMENT);
			}
			return status;
		}
		status = writeBLE (static_message_buffer_, message_length, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
		if (status != 0)
		{
			ESP_LOGE (TAG, "[%s] Failed to send message", action_str.c_str());
			return status;
		}
		return static_cast<int>(TeslaBLE::TeslaBLE_Status_E_OK);
	};
	ESP_LOGI (TAG, "[%s] Adding command to queue", action_str.c_str());
	placeAtFrontOfQueue (UniversalMessage_Domain_DOMAIN_INFOTAINMENT, execute_cmd, action_str, action);
	return TeslaBLE::TeslaBLE_Status_E_OK;
}

static bool schedule_values_valid (int days_of_week, int time1, int time2, float latitude, float longitude)
{
	return (days_of_week > 0)    and (days_of_week <= 127) and
	       (time1 >= 0)          and (time1 <= 1440)       and
	       (time2 >= 0)          and (time2 <= 1440)       and
	       (latitude >= -90.0)   and (latitude <= 90.0)    and
	       (longitude >= -180.0) and (longitude <= 180.0);
}

static uint64_t new_schedule_id (uint64_t requested_id)
/*
*   Tesla uses the creation time (epoch seconds) as the schedule ID. Sending an existing ID replaces (edits) that schedule.
*/
{
	if (requested_id != 0)
		return requested_id;
	uint64_t now = static_cast<uint64_t>(time(nullptr));
	if (now < 1700000000ULL)
	{
		ESP_LOGW (TAG, "Clock not set (is a time component configured?), schedule ID %" PRIu64 " may not be unique", now);
	}
	return now;
}

int TeslaBLEVehicle::actions_setChargeSchedule (
	std::string id,
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
/*
*   Adds a charge schedule, or replaces an existing one if id is that of an existing schedule.
*/
{
	if (allow_setting_schedules_ == 0)
	{
		ESP_LOGW (TAG, "Setting charge schedules disabled. Set allow_setting_schedules in your yaml to non-zero and rebuild to enable.");
		return 0;
	}
	if (!schedule_values_valid (days_of_week, start_time, end_time, latitude, longitude))
	{ // Do some basic validation on the numbers provided so impossible values rejected
		ESP_LOGE (TAG, "Set charge schedule command rejected, one or more of days_of_week, start_time, end_time, latitude and longitude out of range");
		return TeslaBLE::TeslaBLE_Status_E_ERROR_INTERNAL;
	}
	uint64_t requested_id = parse_schedule_id (id);
	if (!id.empty() and requested_id == 0)
	{
		ESP_LOGE (TAG, "Set charge schedule command rejected, invalid schedule ID '%s'", id.c_str());
		return TeslaBLE::TeslaBLE_Status_E_ERROR_INTERNAL;
	}
	uint64_t schedule_id = new_schedule_id (requested_id); // Fixed now so a retry doesn't create a second schedule
	ESP_LOGI (TAG, "%s charge schedule %" PRIu64 ": days=%d start=%d(%d) end=%d(%d) one_time=%d enabled=%d lat=%f long=%f",
	          requested_id ? "Updating" : "Creating", schedule_id, days_of_week, start_time, start_enabled, end_time, end_enabled,
	          one_time, enabled, latitude, longitude);
	return actions_queueBuiltAction (BLE_CarServer_VehicleAction::ADD_CHARGE_SCHEDULE,
		[=](CarServer_VehicleAction& vehicle_action)
		{
			auto& schedule = vehicle_action.vehicle_action_msg.addChargeScheduleAction;
			schedule               = CarServer_ChargeSchedule_init_default;
			schedule.id            = schedule_id;
			strncpy (schedule.name, name.c_str(), sizeof(schedule.name) - 1);
			schedule.days_of_week  = days_of_week;
			schedule.start_enabled = start_enabled;
			schedule.start_time    = start_time;
			schedule.end_enabled   = end_enabled;
			schedule.end_time      = end_time;
			schedule.one_time      = one_time;
			schedule.enabled       = enabled;
			schedule.latitude      = latitude;
			schedule.longitude     = longitude;
		});
}

int TeslaBLEVehicle::actions_setPreconditionSchedule (
	std::string id,
	std::string name,
	int days_of_week,
	int precondition_time,
	bool one_time,
	bool enabled,
	float latitude,
	float longitude)
/*
*   Adds a preconditioning (departure) schedule, or replaces an existing one if id is that of an existing schedule.
*/
{
	if (allow_setting_schedules_ == 0)
	{
		ESP_LOGW (TAG, "Setting precondition schedules disabled. Set allow_setting_schedules in your yaml to non-zero and rebuild to enable.");
		return 0;
	}
	if (!schedule_values_valid (days_of_week, precondition_time, 0, latitude, longitude))
	{
		ESP_LOGE (TAG, "Set precondition schedule command rejected, one or more of days_of_week, precondition_time, latitude and longitude out of range");
		return TeslaBLE::TeslaBLE_Status_E_ERROR_INTERNAL;
	}
	uint64_t requested_id = parse_schedule_id (id);
	if (!id.empty() and requested_id == 0)
	{
		ESP_LOGE (TAG, "Set precondition schedule command rejected, invalid schedule ID '%s'", id.c_str());
		return TeslaBLE::TeslaBLE_Status_E_ERROR_INTERNAL;
	}
	uint64_t schedule_id = new_schedule_id (requested_id);
	ESP_LOGI (TAG, "%s precondition schedule %" PRIu64 ": days=%d time=%d one_time=%d enabled=%d lat=%f long=%f",
	          requested_id ? "Updating" : "Creating", schedule_id, days_of_week, precondition_time, one_time, enabled, latitude, longitude);
	return actions_queueBuiltAction (BLE_CarServer_VehicleAction::ADD_PRECONDITION_SCHEDULE,
		[=](CarServer_VehicleAction& vehicle_action)
		{
			auto& schedule = vehicle_action.vehicle_action_msg.addPreconditionScheduleAction;
			schedule                   = CarServer_PreconditionSchedule_init_default;
			schedule.id                = schedule_id;
#ifdef CarServer_PreconditionSchedule_size // Only defined if the TeslaBLE library has a static (not callback) name field
			strncpy (schedule.name, name.c_str(), sizeof(schedule.name) - 1);
#endif
			schedule.days_of_week      = days_of_week;
			schedule.precondition_time = precondition_time;
			schedule.one_time          = one_time;
			schedule.enabled           = enabled;
			schedule.latitude          = latitude;
			schedule.longitude         = longitude;
		});
}

int TeslaBLEVehicle::actions_deleteSchedule (BLE_CarServer_VehicleAction action, std::string id)
{
	uint64_t schedule_id = parse_schedule_id (id);
	if (schedule_id == 0)
	{
		ESP_LOGW (TAG, "[%s] Invalid schedule ID '%s', delete not sent", get_action_detail(action).action_str, id.c_str());
		return TeslaBLE::TeslaBLE_Status_E_ERROR_INTERNAL;
	}
	return sendCarServerVehicleActionMessage (action, 0, schedule_id);
}

int TeslaBLEVehicle::actions_setScheduledDeparture (
	bool enabled,
	int departure_time,
	int preconditioning_policy,
	int off_peak_policy,
	int off_peak_end_time)
/*
*   The car's single "scheduled departure". Policies are 0 = off, 1 = all week, 2 = weekdays only. Times are minutes after midnight.
*/
{
	if (enabled and ((departure_time < 0) or (departure_time > 1440) or (off_peak_end_time < 0) or (off_peak_end_time > 1440) or
	                 (preconditioning_policy < 0) or (preconditioning_policy > 2) or (off_peak_policy < 0) or (off_peak_policy > 2)))
	{
		ESP_LOGE (TAG, "Set scheduled departure rejected, a time or policy is out of range");
		return TeslaBLE::TeslaBLE_Status_E_ERROR_INTERNAL;
	}
	return actions_queueBuiltAction (BLE_CarServer_VehicleAction::SET_SCHEDULED_DEPARTURE,
		[=](CarServer_VehicleAction& vehicle_action)
		{
			auto& departure = vehicle_action.vehicle_action_msg.scheduledDepartureAction;
			departure         = CarServer_ScheduledDepartureAction_init_default;
			departure.enabled = enabled;
			if (!enabled)
				return; // Clears the scheduled departure
			departure.departure_time          = departure_time;
			departure.off_peak_hours_end_time = off_peak_end_time;
			if (preconditioning_policy != 0)
			{
				departure.has_preconditioning_times         = true;
				departure.preconditioning_times.which_times = (preconditioning_policy == 1) ? CarServer_PreconditioningTimes_all_week_tag
				                                                                            : CarServer_PreconditioningTimes_weekdays_tag;
			}
			if (off_peak_policy != 0)
			{
				departure.has_off_peak_charging_times         = true;
				departure.off_peak_charging_times.which_times = (off_peak_policy == 1) ? CarServer_OffPeakChargingTimes_all_week_tag
				                                                                       : CarServer_OffPeakChargingTimes_weekdays_tag;
			}
		});
}

int TeslaBLEVehicle::actions_startPreconditioning (float temperature)
/*
*   Starts the climate (preconditioning) now, optionally setting the cabin temperature first (temperature <= 0 leaves it unchanged).
*   Commands are placed at the front of the queue so the last one queued is sent first.
*/
{
	sendCarServerVehicleActionMessage (BLE_CarServer_VehicleAction::SET_HVAC_SWITCH, 1);
	if (temperature > 0)
	{
		if ((temperature < 15.0) or (temperature > 28.0))
		{
			ESP_LOGW (TAG, "Preconditioning temperature %.1f out of range 15-28, temperature not changed", temperature);
			return 0;
		}
		actions_queueBuiltAction (BLE_CarServer_VehicleAction::SET_CLIMATE_TEMP,
			[=](CarServer_VehicleAction& vehicle_action)
			{
				auto& adjust = vehicle_action.vehicle_action_msg.hvacTemperatureAdjustmentAction;
				adjust                        = CarServer_HvacTemperatureAdjustmentAction_init_default;
				adjust.driver_temp_celsius    = temperature;
				adjust.passenger_temp_celsius = temperature;
			});
	}
	return 0;
}

} // esphome
} // tesla_ble_vehicle
