import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client, binary_sensor, text_sensor, sensor
from esphome.const import CONF_ID

CODEOWNERS = ["@PedroKTFC"]
DEPENDENCIES = ["ble_client"]

tesla_ble_vehicle_ns = cg.esphome_ns.namespace("tesla_ble_vehicle")
TeslaBLEVehicle = tesla_ble_vehicle_ns.class_(
    "TeslaBLEVehicle", cg.PollingComponent, ble_client.BLEClientNode
)

AUTO_LOAD = ["binary_sensor"]
CONF_VIN = "vin"
CONF_IS_ASLEEP = "is_asleep"
CONF_IS_UNLOCKED = "is_unlocked"
CONF_IS_USER_PRESENT = "is_user_present"
CONF_IS_CHARGE_FLAP_OPEN = "is_charge_flap_open"
CONF_SHIFT_STATE = "shift_state"
CONF_DEFROST_STATE = "defrost_state"
CONF_BOOT_STATE = "is_boot_open"
CONF_FRUNK_STATE = "is_frunk_open"
CONF_CHARGE_STATE = "charge_state"
CONF_ODOMETER = "odometer"
CONF_CHARGE_CURRENT = "charge_current"
CONF_CHARGE_VOLTAGE = "charge_voltage"
CONF_CHARGE_POWER = "charge_power"
CONF_MAX_SOC = "max_soc"
CONF_MAX_AMPS = "max_amps"
CONF_MINS_TO_LIMIT = "mins_to_limit"
CONF_BATTERY_RANGE = "battery_range"
CONF_CHARGING_STATE = "charging_state"
CONF_CHARGE_ENERGY_ADDED = "charge_energy_added"
CONF_CHARGE_DISTANCE_ADDED = "charge_distance_added"
CONF_LAST_UPDATE = "last_update"
CONF_IS_CLIMATE_ON = "is_climate_on"
CONF_INTERNAL_TEMP = "internal_temp"
CONF_EXTERNAL_TEMP = "external_temp"
CONF_WINDOWS_STATE = "windows_state"
CONF_POST_WAKE_POLL_TIME = "post_wake_poll_time" # How long to poll for data after car awakes (s)
CONF_POLL_DATA_PERIOD = "poll_data_period" # Normal period when polling for data when not asleep (s)
CONF_POLL_ASLEEP_PERIOD = "poll_asleep_period" # Period to poll for data when asleep (s)
CONF_POLL_CHARGING_PERIOD = "poll_charging_period" # Period to poll for data when charging (s)
CONF_BLE_DISCONNECTED_MIN_TIME = "ble_disconnected_min_time" # Minimum time BLE must be disconnected before sensors are Unknown (s)
CONF_FAST_POLL_IF_UNLOCKED = "fast_poll_if_unlocked" # if != 0, fast polls are enabled when unlocked
CONF_WAKE_ON_BOOT = "wake_on_boot" # != 0 wakes car on device boot

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_ID): cv.declare_id(TeslaBLEVehicle),
            # add support to set VIN (required)
            cv.Required(CONF_VIN): cv.string,
            cv.Optional(CONF_POST_WAKE_POLL_TIME): cv.uint16_t,
            cv.Optional(CONF_POLL_DATA_PERIOD): cv.uint16_t,
            cv.Optional(CONF_POLL_ASLEEP_PERIOD): cv.uint16_t,
            cv.Optional(CONF_POLL_CHARGING_PERIOD): cv.uint16_t,
            cv.Optional(CONF_BLE_DISCONNECTED_MIN_TIME): cv.uint16_t,
            cv.Optional(CONF_FAST_POLL_IF_UNLOCKED): cv.uint16_t,
            cv.Optional(CONF_WAKE_ON_BOOT): cv.uint16_t,
            cv.Optional(CONF_IS_ASLEEP): binary_sensor.binary_sensor_schema(
                icon="mdi:sleep"
            ).extend(),
            cv.Optional(CONF_IS_UNLOCKED): binary_sensor.binary_sensor_schema(
                device_class=binary_sensor.DEVICE_CLASS_LOCK,
            ).extend(),
            cv.Optional(CONF_IS_USER_PRESENT): binary_sensor.binary_sensor_schema(
                icon="mdi:account-check", device_class=binary_sensor.DEVICE_CLASS_OCCUPANCY
            ).extend(),
            cv.Optional(CONF_IS_CHARGE_FLAP_OPEN): binary_sensor.binary_sensor_schema(
                icon="mdi:ev-plug-tesla", device_class=binary_sensor.DEVICE_CLASS_DOOR
            ).extend(),
            cv.Optional(CONF_DEFROST_STATE): text_sensor.text_sensor_schema(
                icon="mdi:snowflake-melt"
            ).extend(),
            cv.Optional(CONF_SHIFT_STATE): text_sensor.text_sensor_schema(
                icon="mdi:car-shift-pattern"
            ).extend(),
            cv.Optional(CONF_BOOT_STATE): binary_sensor.binary_sensor_schema(
                icon="mdi:car-back", device_class=binary_sensor.DEVICE_CLASS_DOOR
            ).extend(),
            cv.Optional(CONF_FRUNK_STATE): binary_sensor.binary_sensor_schema(
                icon="mdi:car", device_class=binary_sensor.DEVICE_CLASS_DOOR
            ).extend(),
            cv.Optional(CONF_CHARGE_STATE): sensor.sensor_schema(
                icon="mdi:battery-medium", device_class=sensor.DEVICE_CLASS_BATTERY,
                unit_of_measurement="%"
            ).extend(),
            cv.Optional(CONF_ODOMETER): sensor.sensor_schema(
                icon="mdi:counter", device_class=sensor.DEVICE_CLASS_DISTANCE,
                accuracy_decimals=2, unit_of_measurement="mi"
            ).extend(),
            cv.Optional(CONF_CHARGE_DISTANCE_ADDED): sensor.sensor_schema(
                icon="mdi:map-marker-distance", device_class=sensor.DEVICE_CLASS_DISTANCE,
                accuracy_decimals=2, unit_of_measurement="mi"
            ).extend(),
            cv.Optional(CONF_CHARGE_ENERGY_ADDED): sensor.sensor_schema(
                icon="mdi:battery-positive", device_class=sensor.DEVICE_CLASS_ENERGY_STORAGE,
                accuracy_decimals=2, unit_of_measurement="kWh"
            ).extend(),
            cv.Optional(CONF_CHARGE_CURRENT): sensor.sensor_schema(
                icon="mdi:current-ac", device_class=sensor.DEVICE_CLASS_CURRENT,
                unit_of_measurement="A"
            ).extend(),
            cv.Optional(CONF_CHARGE_VOLTAGE): sensor.sensor_schema(
                icon="mdi:flash-triangle", device_class=sensor.DEVICE_CLASS_VOLTAGE,
                unit_of_measurement="V"
            ).extend(),
            cv.Optional(CONF_CHARGE_POWER): sensor.sensor_schema(
                icon="mdi:lightning-bolt-circle", device_class=sensor.DEVICE_CLASS_POWER,
                unit_of_measurement="kW"
            ).extend(),
            cv.Optional(CONF_MAX_SOC): sensor.sensor_schema(
                icon="mdi:battery-lock", device_class=sensor.DEVICE_CLASS_BATTERY,
                unit_of_measurement="%"
            ).extend(),
            cv.Optional(CONF_MAX_AMPS): sensor.sensor_schema(
                icon="mdi:current-ac", device_class=sensor.DEVICE_CLASS_CURRENT,
                unit_of_measurement="A"
            ).extend(),
            cv.Optional(CONF_MINS_TO_LIMIT): sensor.sensor_schema(
                icon="mdi:timer-sand", device_class=sensor.DEVICE_CLASS_DURATION,
                unit_of_measurement="min"
            ).extend(),
            cv.Optional(CONF_BATTERY_RANGE): sensor.sensor_schema(
                icon="mdi:gauge", device_class=sensor.DEVICE_CLASS_DISTANCE,
                accuracy_decimals=2, unit_of_measurement="mi"
            ).extend(),
            cv.Optional(CONF_CHARGING_STATE): text_sensor.text_sensor_schema(
                icon="mdi:ev-station"
            ).extend(),
            cv.Optional(CONF_LAST_UPDATE): text_sensor.text_sensor_schema(
                icon="mdi:update"
            ).extend(),
            cv.Optional(CONF_IS_CLIMATE_ON): binary_sensor.binary_sensor_schema(
                icon="mdi:fan"
            ).extend(),
            cv.Optional(CONF_WINDOWS_STATE): binary_sensor.binary_sensor_schema(
                icon="mdi:car-door", device_class=binary_sensor.DEVICE_CLASS_WINDOW
            ).extend(),
            cv.Optional(CONF_INTERNAL_TEMP): sensor.sensor_schema(
                icon="mdi:thermometer", device_class=sensor.DEVICE_CLASS_TEMPERATURE,
                accuracy_decimals=1, unit_of_measurement="°C"
            ).extend(),
            cv.Optional(CONF_EXTERNAL_TEMP): sensor.sensor_schema(
                icon="mdi:sun-thermometer-outline", device_class=sensor.DEVICE_CLASS_TEMPERATURE,
                accuracy_decimals=1, unit_of_measurement="°C"
            ).extend(),
        }
    )
    .extend(cv.polling_component_schema("1min"))
    .extend(ble_client.BLE_CLIENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    cg.add(var.set_vin(config[CONF_VIN]))
    cg.add(var.load_polling_parameters(config[CONF_POST_WAKE_POLL_TIME], config[CONF_POLL_DATA_PERIOD],
           config[CONF_POLL_ASLEEP_PERIOD], config[CONF_POLL_CHARGING_PERIOD], config[CONF_BLE_DISCONNECTED_MIN_TIME],
           config[CONF_FAST_POLL_IF_UNLOCKED], config[CONF_WAKE_ON_BOOT]))

    if CONF_IS_ASLEEP in config:
        conf = config[CONF_IS_ASLEEP]
        bs = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_binary_sensor_is_asleep(bs))
    if CONF_IS_UNLOCKED in config:
        conf = config[CONF_IS_UNLOCKED]
        bs = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_binary_sensor_is_unlocked(bs))
    if CONF_IS_USER_PRESENT in config:
        conf = config[CONF_IS_USER_PRESENT]
        bs = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_binary_sensor_is_user_present(bs))
    if CONF_IS_CHARGE_FLAP_OPEN in config:
        conf = config[CONF_IS_CHARGE_FLAP_OPEN]
        bs = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_binary_sensor_is_charge_flap_open(bs))
    if CONF_SHIFT_STATE in config:
        conf = config[CONF_SHIFT_STATE]
        ts = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_text_sensor_shift_state(ts))
    if CONF_DEFROST_STATE in config:
        conf = config[CONF_DEFROST_STATE]
        ts = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_text_sensor_defrost_state(ts))
    if CONF_BOOT_STATE in config:
        conf = config[CONF_BOOT_STATE]
        bs = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_binary_sensor_is_boot_open(bs))
    if CONF_FRUNK_STATE in config:
        conf = config[CONF_FRUNK_STATE]
        bs = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_binary_sensor_is_frunk_open(bs))
    if CONF_CHARGE_STATE in config:
        conf = config[CONF_CHARGE_STATE]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_charge_state(ss))
    if CONF_ODOMETER in config:
        conf = config[CONF_ODOMETER]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_odometer_state(ss))
    if CONF_CHARGE_CURRENT in config:
        conf = config[CONF_CHARGE_CURRENT]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_charge_current_state(ss))
    if CONF_CHARGE_VOLTAGE in config:
        conf = config[CONF_CHARGE_VOLTAGE]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_charge_voltage_state(ss))
    if CONF_CHARGE_POWER in config:
        conf = config[CONF_CHARGE_POWER]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_charge_power_state(ss))
    if CONF_MAX_SOC in config:
        conf = config[CONF_MAX_SOC]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_max_soc_state(ss))
    if CONF_MAX_AMPS in config:
        conf = config[CONF_MAX_AMPS]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_max_amps_state(ss))
    if CONF_MINS_TO_LIMIT in config:
        conf = config[CONF_MINS_TO_LIMIT]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_mins_to_limit_state(ss))
    if CONF_BATTERY_RANGE in config:
        conf = config[CONF_BATTERY_RANGE]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_battery_range_state(ss))
    if CONF_CHARGE_ENERGY_ADDED in config:
        conf = config[CONF_CHARGE_ENERGY_ADDED]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_charge_energy_added_state(ss))
    if CONF_CHARGE_DISTANCE_ADDED in config:
        conf = config[CONF_CHARGE_DISTANCE_ADDED]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_charge_distance_added_state(ss))
    if CONF_CHARGING_STATE in config:
        conf = config[CONF_CHARGING_STATE]
        ts = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_text_sensor_charging_state(ts))
    if CONF_LAST_UPDATE in config:
        conf = config[CONF_LAST_UPDATE]
        ts = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_text_sensor_last_update_state(ts))
    if CONF_IS_CLIMATE_ON in config:
        conf = config[CONF_IS_CLIMATE_ON]
        bs = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_binary_sensor_is_climate_on(bs))
    if CONF_WINDOWS_STATE in config:
        conf = config[CONF_WINDOWS_STATE]
        bs = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_binary_sensor_windows_state(bs))
    if CONF_INTERNAL_TEMP in config:
        conf = config[CONF_INTERNAL_TEMP]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_internal_temp_state(ss))
    if CONF_EXTERNAL_TEMP in config:
        conf = config[CONF_EXTERNAL_TEMP]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_external_temp_state(ss))
