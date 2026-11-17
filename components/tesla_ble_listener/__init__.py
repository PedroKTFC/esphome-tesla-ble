import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32_ble_tracker
from esphome.const import CONF_ID

CODEOWNERS = ["@ypedroktfc"]
DEPENDENCIES = ["esp32_ble_tracker"]
CONF_VIN = "vin"
CONF_MAC_ADDRESS = "mac_address"

tesla_ble_listener_ns = cg.esphome_ns.namespace("tesla_ble_listener")
TeslaBLEListener = tesla_ble_listener_ns.class_(
    "TeslaBLEListener", cg.Component, esp32_ble_tracker.ESPBTDeviceListener
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(TeslaBLEListener),
        cv.Required(CONF_VIN): cv.string,
        cv.Optional(CONF_MAC_ADDRESS): cv.string,
    }
).extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield esp32_ble_tracker.register_ble_device(var, config)
    cg.add(var.set_vin(config[CONF_VIN]))
    if CONF_MAC_ADDRESS in config:
        cg.add(var.set_expected_mac(config[CONF_MAC_ADDRESS]))
