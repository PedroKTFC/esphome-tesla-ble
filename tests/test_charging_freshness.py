"""Run the package's actual timestamp lambdas with a controllable UTC clock.

Requires PyYAML and a C++17 compiler; no vehicle or ESP32 is needed.
"""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

import yaml


ROOT = Path(__file__).resolve().parents[1]
PACKAGE = ROOT / "packages/charging-freshness.yml"
FIELDS = {
    "charge_state": "charge_level_received",
    "charge_power": "charge_power_received",
    "charging_state": "charging_state_received",
}


class PackageLoader(yaml.SafeLoader):
    pass


PackageLoader.add_constructor("!lambda", lambda loader, node: loader.construct_scalar(node))


class ChargingFreshnessTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.package = yaml.load(PACKAGE.read_text(), Loader=PackageLoader)

    def test_each_field_has_its_own_raw_sample_callback(self):
        vehicle = self.package["tesla_ble_vehicle"]
        self.assertEqual(set(vehicle), set(FIELDS))
        for field, target in FIELDS.items():
            self.assertEqual(set(vehicle[field]), {"on_raw_value"})
            actions = vehicle[field]["on_raw_value"]
            self.assertEqual(len(actions), 1)
            self.assertEqual(actions[0]["text_sensor.template.publish"]["id"], target)

    def test_timestamps_cannot_refresh_on_a_timer(self):
        self.assertEqual(set(self.package), {"tesla_ble_vehicle", "text_sensor"})
        sensors = {sensor["id"]: sensor for sensor in self.package["text_sensor"]}
        self.assertEqual(set(sensors), set(FIELDS.values()))
        for sensor in sensors.values():
            self.assertEqual(sensor["platform"], "template")
            self.assertEqual(sensor["device_class"], "timestamp")
            self.assertEqual(sensor["entity_category"], "diagnostic")
            self.assertEqual(sensor["update_interval"], "never")
            self.assertNotIn("lambda", sensor)
            self.assertNotIn("restore_value", sensor)

    def test_actual_lambdas_with_repeated_invalid_and_unsynchronized_samples(self):
        functions = []
        for field in FIELDS:
            action = self.package["tesla_ble_vehicle"][field]["on_raw_value"][0]
            body = action["text_sensor.template.publish"]["state"]
            parameter = "const std::string&" if field == "charging_state" else "float"
            functions.append(f"std::string {field}({parameter} x) {{\n{body}\n}}")

        # This stand-in only supplies the clock. The validation and formatting
        # decisions below are the C++ lambdas extracted unchanged from the YAML.
        source = r'''
#include <cassert>
#include <cmath>
#include <ctime>
#include <string>
struct Clock {
  std::time_t timestamp = 1792324800;
  bool valid = true;
  Clock utcnow() const { return *this; }
  bool is_valid() const { return valid; }
  std::string strftime(const char *format) {
    char result[32];
    std::tm utc = *std::gmtime(&timestamp);
    std::strftime(result, sizeof(result), format, &utc);
    return result;
  }
} homeassistant_time;
#define id(name) name
''' + "\n".join(functions) + r'''
int main() {
  assert(charge_state(52) == "2026-10-18T12:00:00Z");
  assert(charge_power(7) == "2026-10-18T12:00:00Z");
  assert(charging_state("Charging") == "2026-10-18T12:00:00Z");
  homeassistant_time.timestamp += 10;
  assert(charge_state(52) == "2026-10-18T12:00:10Z");
  assert(charge_power(7) == "2026-10-18T12:00:10Z");
  assert(charging_state("Charging") == "2026-10-18T12:00:10Z");
  assert(charge_state(0) != "unavailable");
  assert(charge_state(100) != "unavailable");
  assert(charge_power(0) != "unavailable");
  for (float value : {NAN, INFINITY, -INFINITY, -1.0f}) {
    assert(charge_state(value) == "unavailable");
    assert(charge_power(value) == "unavailable");
  }
  assert(charge_state(101) == "unavailable");
  for (const auto *value : {"Disconnected", "No Power", "Starting", "Charging",
                            "Complete", "Stopped", "Calibrating"}) {
    assert(charging_state(value) != "unavailable");
  }
  for (const auto *value : {"Unknown", "Charging state look up error", "", "unavailable"}) {
    assert(charging_state(value) == "unavailable");
  }
  homeassistant_time.valid = false;
  assert(charge_state(52) == "unavailable");
  assert(charge_power(7) == "unavailable");
  assert(charging_state("Charging") == "unavailable");
}
'''
        with tempfile.TemporaryDirectory() as directory:
            cpp = Path(directory) / "freshness.cpp"
            executable = Path(directory) / "freshness"
            cpp.write_text(source)
            subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra",
                            "-Werror", str(cpp), "-o", str(executable)], check=True)
            subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    unittest.main()
