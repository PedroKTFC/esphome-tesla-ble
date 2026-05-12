import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client, binary_sensor, text_sensor, sensor
from esphome.const import CONF_ID, STATE_CLASS_MEASUREMENT
from enum import Enum, auto
from dataclasses import dataclass
from typing import Dict, Any

CODEOWNERS = ["@PedroKTFC"]
DEPENDENCIES = ["ble_client"]

class SensorTypes(Enum):
    BINARY =  auto()
    TEXT =    auto()
    NUMERIC = auto()

tesla_ble_vehicle_ns = cg.esphome_ns.namespace("tesla_ble_vehicle")
TeslaBLEVehicle = tesla_ble_vehicle_ns.class_(
    "TeslaBLEVehicle", cg.PollingComponent, ble_client.BLEClientNode
)

BinarySensorId = tesla_ble_vehicle_ns.enum("BinarySensorId", is_class=True)
TextSensorId = tesla_ble_vehicle_ns.enum("TextSensorId", is_class=True)
NumericSensorId = tesla_ble_vehicle_ns.enum("NumericSensorId", is_class=True)

@dataclass
class SensorSpec:
    type: SensorTypes
    setter_id: cg.MockObj
    schema_options: Dict[str, Any]

#AUTO_LOAD = ["binary_sensor", "sensor", "text_sensor"]

def binary(id, **schema):
    return SensorSpec(SensorTypes.BINARY, id, schema)
def numeric(id, **schema):
    return SensorSpec(SensorTypes.NUMERIC, id, schema)
def text(id, **schema):
    return SensorSpec(SensorTypes.TEXT, id, schema)

# Constants
CONF_VIN = "vin"
CONF_POST_WAKE_POLL_TIME = "post_wake_poll_time" # How long to poll for data after car awakes (s)
CONF_POLL_DATA_PERIOD = "poll_data_period" # Normal period when polling for data when not asleep (s)
CONF_POLL_ASLEEP_PERIOD = "poll_asleep_period" # Period to poll for data when asleep (s)
CONF_POLL_CHARGING_PERIOD = "poll_charging_period" # Period to poll for data when charging (s)
CONF_BLE_DISCONNECTED_MIN_TIME = "ble_disconnected_min_time" # Minimum time BLE must be disconnected before sensors are Unknown (s)
CONF_FAST_POLL_IF_UNLOCKED = "fast_poll_if_unlocked" # if != 0, fast polls are enabled when unlocked
CONF_WAKE_ON_BOOT = "wake_on_boot" # != 0 wakes car on device boot

SENSORS = {
    "is_asleep": binary (BinarySensorId.IsAsleep,
        icon = "mdi:sleep",),
    "is_unlocked": binary (
        BinarySensorId.IsUnlocked,
        device_class = binary_sensor.DEVICE_CLASS_LOCK,),
    "is_user_present": binary (BinarySensorId.IsUserPresent,
        icon = "mdi:account-check", device_class = binary_sensor.DEVICE_CLASS_OCCUPANCY,),
    "is_charge_flap_open": binary (BinarySensorId.IsChargeFlapOpen,
        icon = "mdi:ev-plug-tesla", device_class = binary_sensor.DEVICE_CLASS_DOOR,),
    "is_boot_open": binary (BinarySensorId.IsBootOpen,
        icon = "mdi:car-back", device_class = binary_sensor.DEVICE_CLASS_DOOR,),
    "is_frunk_open": binary (BinarySensorId.IsFrunkOpen,
        icon = "mdi:car", device_class = binary_sensor.DEVICE_CLASS_DOOR,),
    "charge_state": numeric (NumericSensorId.ChargeState,
        icon = "mdi:battery-medium", device_class = sensor.DEVICE_CLASS_BATTERY, unit_of_measurement = "%",),
    "odometer": numeric (NumericSensorId.Odometer,
        icon = "mdi:counter", device_class = sensor.DEVICE_CLASS_DISTANCE, accuracy_decimals = 2, unit_of_measurement = "mi",),
    "charge_distance_added": numeric (NumericSensorId.ChargeDistanceAdded,
        icon = "mdi:map-marker-distance", device_class = sensor.DEVICE_CLASS_DISTANCE, accuracy_decimals = 2, unit_of_measurement = "mi",),
    "charge_energy_added": numeric (NumericSensorId.ChargeEnergyAdded,
        icon = "mdi:battery-positive", device_class = sensor.DEVICE_CLASS_ENERGY_STORAGE, accuracy_decimals = 2, unit_of_measurement = "kWh",),
    "charge_current": numeric (NumericSensorId.ChargeCurrent,
        icon = "mdi:current-ac", device_class = sensor.DEVICE_CLASS_CURRENT, unit_of_measurement = "A",),
    "charge_voltage": numeric (NumericSensorId.ChargeVoltage,
        icon = "mdi:flash-triangle", device_class = sensor.DEVICE_CLASS_VOLTAGE, unit_of_measurement = "V",),
    "charge_power": numeric (NumericSensorId.ChargePower,
        icon = "mdi:lightning-bolt-circle", device_class = sensor.DEVICE_CLASS_POWER, unit_of_measurement = "kW",),
    "max_soc": numeric (NumericSensorId.MaxSoc,
        icon = "mdi:battery-lock", device_class = sensor.DEVICE_CLASS_BATTERY, unit_of_measurement = "%",),
    "max_amps": numeric (NumericSensorId.MaxAmps,
        icon = "mdi:current-ac", device_class = sensor.DEVICE_CLASS_CURRENT, unit_of_measurement = "A",),
    "mins_to_limit": numeric (NumericSensorId.MinsToLimit,
        icon = "mdi:timer-sand", device_class = sensor.DEVICE_CLASS_DURATION, unit_of_measurement = "min",),
    "battery_range": numeric (NumericSensorId.BatteryRange,
        icon = "mdi:gauge", device_class = sensor.DEVICE_CLASS_DISTANCE, accuracy_decimals = 2, unit_of_measurement = "mi",),
    "internal_temp": numeric (NumericSensorId.InternalTemp,
        icon = "mdi:thermometer", device_class = sensor.DEVICE_CLASS_TEMPERATURE, accuracy_decimals = 1, unit_of_measurement = "°C",),
    "external_temp": numeric (NumericSensorId.ExternalTemp,
        icon = "mdi:sun-thermometer-outline", device_class = sensor.DEVICE_CLASS_TEMPERATURE, accuracy_decimals = 1, unit_of_measurement = "°C",),
    "driver_temp": numeric (NumericSensorId.DriverTemp,
        icon = "mdi:thermometer", device_class = sensor.DEVICE_CLASS_TEMPERATURE, accuracy_decimals = 1, unit_of_measurement = "°C",),
    "tpms_pressure_fl": numeric (NumericSensorId.TpmsFl,
        icon = "mdi:tire", device_class = sensor.DEVICE_CLASS_PRESSURE, accuracy_decimals = 1, unit_of_measurement = "bar",),
    "tpms_pressure_fr": numeric (NumericSensorId.TpmsFr,
        icon = "mdi:tire", device_class = sensor.DEVICE_CLASS_PRESSURE, accuracy_decimals = 1, unit_of_measurement = "bar",),
    "tpms_pressure_rl": numeric (NumericSensorId.TpmsRl,
        icon = "mdi:tire", device_class = sensor.DEVICE_CLASS_PRESSURE, accuracy_decimals = 1, unit_of_measurement = "bar",),
    "tpms_pressure_rr": numeric (NumericSensorId.TpmsRr,
        icon = "mdi:tire", device_class = sensor.DEVICE_CLASS_PRESSURE, accuracy_decimals = 1, unit_of_measurement = "bar",),
    "is_climate_on": binary (BinarySensorId.IsClimateOn,
        icon = "mdi:fan",),
    "windows_state": binary (BinarySensorId.WindowsState,
        icon = "mdi:car-door", device_class = binary_sensor.DEVICE_CLASS_WINDOW,),
    "shift_state": text (TextSensorId.ShiftState,
        icon = "mdi:car-shift-pattern",),
    "defrost_state": text (TextSensorId.DefrostState,
        icon = "mdi:snowflake-melt",),
    "charging_state": text (TextSensorId.ChargingState,
        icon = "mdi:ev-station",),
    "charge_port_latch_state": text (TextSensorId.ChargePortLatchState,
        icon = "mdi:battery-lock",),
    "last_update": text (TextSensorId.LastUpdate,
        icon = "mdi:update",),
    "ble_disconnected_time": numeric (NumericSensorId.BleDisconnectedTime,
        icon = "mdi:bluetooth-off", device_class = sensor.DEVICE_CLASS_DURATION, unit_of_measurement = "s",),
    "charger_phases": numeric (NumericSensorId.ChargerPhases,
        icon = "mdi:surround-sound-3-1", device_class = "", state_class = STATE_CLASS_MEASUREMENT, accuracy_decimals = 0,),
    "charge_rate": numeric (NumericSensorId.ChargeRate,
        icon = "mdi:speedometer", device_class = sensor.DEVICE_CLASS_SPEED, accuracy_decimals = 0, unit_of_measurement = "mph",),
}

SENSOR_TYPES_INFO = {
    SensorTypes.BINARY: {
        "schema"    : binary_sensor.binary_sensor_schema,
        "creator"   : binary_sensor.new_binary_sensor,
        "setter"    : "set_binary_sensor",
    },
    SensorTypes.TEXT: {
        "schema"    : text_sensor.text_sensor_schema,
        "creator"   : text_sensor.new_text_sensor,
        "setter"    : "set_text_sensor",
    },
    SensorTypes.NUMERIC: {
        "schema"    : sensor.sensor_schema,
        "creator"   : sensor.new_sensor,
        "setter"    : "set_numeric_sensor",
    },
}

schema_dict = {
    cv.GenerateID(CONF_ID): cv.declare_id(TeslaBLEVehicle),
    cv.Required(CONF_VIN): cv.string,

    cv.Optional(CONF_POST_WAKE_POLL_TIME): cv.uint16_t,
    cv.Optional(CONF_POLL_DATA_PERIOD): cv.uint16_t,
    cv.Optional(CONF_POLL_ASLEEP_PERIOD): cv.uint16_t,
    cv.Optional(CONF_POLL_CHARGING_PERIOD): cv.uint16_t,
    cv.Optional(CONF_BLE_DISCONNECTED_MIN_TIME): cv.uint16_t,
    cv.Optional(CONF_FAST_POLL_IF_UNLOCKED): cv.uint16_t,
    cv.Optional(CONF_WAKE_ON_BOOT): cv.uint16_t,
}
for key, spec in SENSORS.items():
    builder = SENSOR_TYPES_INFO[spec.type]["schema"]
    schema_dict[cv.Optional(key)] = (builder(**spec.schema_options))

MULTI_CONF = True

CONFIG_SCHEMA = (
    cv.Schema(schema_dict)
    .extend(cv.polling_component_schema("1min"))
    .extend(ble_client.BLE_CLIENT_SCHEMA)
)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    component_id = str(config[CONF_ID])
    nvs_namespace = "storage" if component_id == "tesla_ble_vehicle_id" else component_id[:15]
    cg.add(var.set_nvs_namespace(nvs_namespace))

    cg.add(var.set_vin(config[CONF_VIN]))

    cg.add(
        var.load_polling_parameters(
            config.get(CONF_POST_WAKE_POLL_TIME),
            config.get(CONF_POLL_DATA_PERIOD),
            config.get(CONF_POLL_ASLEEP_PERIOD),
            config.get(CONF_POLL_CHARGING_PERIOD),
            config.get(CONF_BLE_DISCONNECTED_MIN_TIME),
            config.get(CONF_FAST_POLL_IF_UNLOCKED),
            config.get(CONF_WAKE_ON_BOOT),
        )
    )
    # 🔁 Auto-register all sensors
    for key, spec in SENSORS.items():
        if key not in config:
            continue
        info = SENSOR_TYPES_INFO[spec.type]
        sensor_obj = await info["creator"](config[key])
        setter = getattr(var, info["setter"])
        cg.add(setter(spec.setter_id, sensor_obj))
