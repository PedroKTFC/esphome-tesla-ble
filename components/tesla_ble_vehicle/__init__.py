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
CONF_CHARGE_STATE = "charge_state"
CONF_ODOMETER = "odometer"
CONF_CHARGE_CURRENT = "charge_current"
CONF_MAX_SOC = "max_soc"
CONF_BATTERY_RANGE = "battery_range"
CONF_CHARGING_STATE = "charging_state"
CONF_LAST_UPDATE = "last_update"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_ID): cv.declare_id(TeslaBLEVehicle),
            # add support to set VIN (required)
            cv.Required(CONF_VIN): cv.string,
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
            cv.Optional(CONF_SHIFT_STATE): text_sensor.text_sensor_schema(
                icon="mdi:car-shift-pattern"
            ).extend(),
            cv.Optional(CONF_CHARGE_STATE): sensor.sensor_schema(
                icon="mdi:battery-medium", device_class=sensor.DEVICE_CLASS_BATTERY,
                unit_of_measurement="%"
            ).extend(),
            cv.Optional(CONF_ODOMETER): sensor.sensor_schema(
                icon="mdi:counter", device_class=sensor.DEVICE_CLASS_DISTANCE,
                accuracy_decimals=2, unit_of_measurement="miles"
            ).extend(),
            cv.Optional(CONF_CHARGE_CURRENT): sensor.sensor_schema(
                icon="mdi:current-ac", device_class=sensor.DEVICE_CLASS_CURRENT,
                unit_of_measurement="A"
            ).extend(),
            cv.Optional(CONF_MAX_SOC): sensor.sensor_schema(
                icon="mdi:battery-lock", device_class=sensor.DEVICE_CLASS_ENERGY_STORAGE,
                unit_of_measurement="%"
            ).extend(),
            cv.Optional(CONF_BATTERY_RANGE): sensor.sensor_schema(
                icon="mdi:gauge", device_class=sensor.DEVICE_CLASS_DISTANCE,
                accuracy_decimals=2, unit_of_measurement="miles"
            ).extend(),
            cv.Optional(CONF_CHARGING_STATE): text_sensor.text_sensor_schema(
                icon="mdi:ev-station"
            ).extend(),
            cv.Optional(CONF_LAST_UPDATE): text_sensor.text_sensor_schema(
                icon="mdi:update"
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

    if CONF_MAX_SOC in config:
        conf = config[CONF_MAX_SOC]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_max_soc_state(ss))

    if CONF_BATTERY_RANGE in config:
        conf = config[CONF_BATTERY_RANGE]
        ss = await sensor.new_sensor(conf)
        cg.add(var.set_sensor_battery_range_state(ss))

    if CONF_CHARGING_STATE in config:
        conf = config[CONF_CHARGING_STATE]
        ts = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_text_sensor_charging_state(ts))

    if CONF_LAST_UPDATE in config:
        conf = config[CONF_LAST_UPDATE]
        ts = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_text_sensor_last_update_state(ts))
