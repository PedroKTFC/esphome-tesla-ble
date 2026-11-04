# One Home Assistant software update sensor

The ESPHome component exposes the update fields as diagnostic entities. To show them as one Home Assistant sensor with attributes, add this state-based template sensor to your Home Assistant configuration. Replace the `tesla_ble` entity ID prefix with the prefix used by your ESPHome device.

```yaml
template:
  - sensor:
      - name: Tesla software update
        unique_id: tesla_ble_software_update
        icon: mdi:update
        availability: >-
          {{ states('binary_sensor.tesla_ble_software_update_available') in ['on', 'off'] }}
        state: >-
          {% set available = states('binary_sensor.tesla_ble_software_update_available') %}
          {% set status = states('sensor.tesla_ble_software_update_state') %}
          {% if available == 'off' and status == 'No update available' %}
            No update available
          {% elif available == 'on' %}
            {{ status }}
          {% else %}
            Unknown
          {% endif %}
        attributes:
          update_available: >-
            {% set available = states('binary_sensor.tesla_ble_software_update_available') %}
            {% set status = states('sensor.tesla_ble_software_update_state') %}
            {% if available == 'on' %}{{ true }}
            {% elif available == 'off' and status == 'No update available' %}{{ false }}
            {% else %}{{ none }}
            {% endif %}
          target_version: >-
            {% set available = states('binary_sensor.tesla_ble_software_update_available') %}
            {% set version = states('sensor.tesla_ble_software_update_version') %}
            {{ version if available == 'on' and version not in ['', 'Unknown', 'unknown', 'unavailable'] else none }}
          download_progress: >-
            {% set available = states('binary_sensor.tesla_ble_software_update_available') %}
            {% set value = states('sensor.tesla_ble_software_update_download_progress') %}
            {{ value | float if available == 'on' and is_number(value) else none }}
          install_progress: >-
            {% set available = states('binary_sensor.tesla_ble_software_update_available') %}
            {% set value = states('sensor.tesla_ble_software_update_install_progress') %}
            {{ value | float if available == 'on' and is_number(value) else none }}
```

The sensor state is `No update available` when the component reports no pending update, and `Unknown` when it cannot determine the status. Version and progress attributes remain empty when Tesla does not provide them; they are not treated as zero. The BLE endpoint reports a target version when an update is pending, not the vehicle's currently installed software version.
