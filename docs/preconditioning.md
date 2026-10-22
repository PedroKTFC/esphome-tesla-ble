## Preconditioning and preconditioning schedules

This project lets Home Assistant (HA) **start/stop preconditioning now**, and **view**, **add**, **edit** and **delete** the car's preconditioning (departure) schedules. These are the same schedules you see under *Schedule → Precondition* in the Tesla app. It also supports the older single **scheduled departure** setting.

The commands are the BLE equivalents of the ones in Tesla's [vehicle-command](https://github.com/teslamotors/vehicle-command) (`add-precondition-schedule`, `remove-precondition-schedule`, `scheduled-departure`, `climate-on`).

In the action names below, `tesla_ble` is your device name with `-` replaced by `_` (eg **esphome.tesla_ble_start_preconditioning**).

> [!IMPORTANT]
> Reading preconditioning schedules requires a version of the [TeslaBLE library](https://github.com/PedroKTFC/tesla-ble) that decodes them (the `precondition-schedules` change, which gives the nanopb `PreconditioningScheduleState` a fixed size). With an older library, setting and deleting schedules still work but reading them logs a warning instead of firing the event.

### Start/stop preconditioning now
| Action | Fields | Notes |
| --- | --- | --- |
| `start_preconditioning` | `temperature` (°C, 15-28, `0` keeps the current setting) | Sets driver and passenger temperature, then turns climate on. Wakes the car. |
| `stop_preconditioning` | | Turns climate off. |

The existing **Climate** switch and **Defrost** (max preconditioning) switch also still work.

### Battery warming
Tesla's protocol has no separate "warm the battery" command (neither BLE nor the Fleet API). The car heats the battery itself when it preconditions in the cold: preconditioning schedules, scheduled departure, climate on, and most strongly **Defrost** (max preconditioning). Two binary sensors show what the car is actually doing:
- **Battery heater**: on while the battery heater runs.
- **Preconditioning**: on while the car is preconditioning (cabin and/or battery).

Both come from the climate data that is polled anyway, so they add no extra traffic to the car.

### Preconditioning schedules
Like charging schedules, setting is only allowed when the build-time option `allow_setting_schedules` is non-zero (see [charging_schedules.md](./charging_schedules.md)). Reading and deleting are always allowed.

| Action | Fields |
| --- | --- |
| `set_precondition_schedule` | `schedule_id` (empty = new, an existing ID = edit/replace), `name`, `days_of_week` (bitmask Sun = 1 … Sat = 64), `precondition_time` (minutes after midnight, the time the car should be ready), `one_time`, `enabled`, `latitude`, `longitude` |
| `delete_precondition_schedule` | `schedule_id` |
| `get_precondition_schedules` | |

There is also a **Get precondition schedules** button and a **Delete a precondition schedule** text control (both disabled by default).

After a set or delete the list is read back automatically. The list arrives as the event `esphome.tesla_precondition_schedules_updated` with fields `count`, `schedules` (JSON: `{"precondition_schedules": [...]}`), `version` and `comment`. Each schedule has `id`, `name`, `days`, `*days`, `precondition_time`, `*precondition_time` (HH:MM), `one_time`, `enabled`, `latitude`, `longitude`. Fields starting with `*` are derived.

The schedule location matters: the car only runs a schedule when it is parked near that latitude/longitude, so use the location where the car will be (eg `zone.home`).

> [!NOTE]
> A BLE response is limited to 400 bytes, enough for roughly ten schedules of each type. If you have more, the response cannot be decoded.

### Scheduled departure (older single setting)
`set_scheduled_departure` fields: `enabled` (false clears it), `departure_time` (minutes after midnight), `preconditioning_policy` and `off_peak_policy` (0 = off, 1 = all week, 2 = weekdays only), `off_peak_end_time` (minutes after midnight). Cars on recent software use preconditioning schedules instead.

### Home Assistant package
The package below adds two sensors holding the schedule lists, and scripts to add/edit/delete schedules with friendly fields (time picker, day names, zone). Save it as `packages/tesla_schedules.yaml` in your HA config (with `homeassistant: packages: !include_dir_named packages` in `configuration.yaml`), and replace `tesla_ble` if your device name differs.

```yaml
template:
  - triggers:
      - trigger: event
        event_type: esphome.tesla_schedules_updated
    sensor:
      - name: Tesla charge schedules
        unique_id: tesla_charge_schedules
        state: "{{ trigger.event.data.count }}"
        attributes:
          schedules: "{{ (trigger.event.data.schedules | from_json).charge_schedules }}"
  - triggers:
      - trigger: event
        event_type: esphome.tesla_precondition_schedules_updated
    sensor:
      - name: Tesla precondition schedules
        unique_id: tesla_precondition_schedules
        state: "{{ trigger.event.data.count }}"
        attributes:
          schedules: "{{ (trigger.event.data.schedules | from_json).precondition_schedules }}"

script:
  tesla_set_precondition_schedule:
    alias: Tesla - add or edit precondition schedule
    mode: queued
    fields:
      schedule_id:
        name: Schedule ID
        description: Leave empty to add a new schedule, or give an existing ID to edit it
        selector:
          text:
      time:
        name: Ready at
        required: true
        selector:
          time:
      days:
        name: Days
        required: true
        default: [Mon, Tue, Wed, Thu, Fri]
        selector:
          select:
            multiple: true
            options: [Sun, Mon, Tue, Wed, Thu, Fri, Sat]
      one_time:
        name: One time
        default: false
        selector:
          boolean:
      enabled:
        name: Enabled
        default: true
        selector:
          boolean:
      location:
        name: Location
        default: zone.home
        selector:
          entity:
            domain: zone
    sequence:
      - variables:
          day_bits: {Sun: 1, Mon: 2, Tue: 4, Wed: 8, Thu: 16, Fri: 32, Sat: 64}
          zone: "{{ location | default('zone.home') }}"
      - action: esphome.tesla_ble_set_precondition_schedule
        data:
          schedule_id: "{{ schedule_id | default('') }}"
          name: ""
          days_of_week: >-
            {% set ns = namespace(bits=0) %}{% for d in days %}{% set ns.bits = ns.bits + day_bits[d] %}{% endfor %}{{ ns.bits }}
          precondition_time: "{{ (time.split(':')[0] | int) * 60 + (time.split(':')[1] | int) }}"
          one_time: "{{ one_time | default(false) }}"
          enabled: "{{ enabled | default(true) }}"
          latitude: "{{ state_attr(zone, 'latitude') | float }}"
          longitude: "{{ state_attr(zone, 'longitude') | float }}"

  tesla_delete_precondition_schedule:
    alias: Tesla - delete precondition schedule
    fields:
      schedule_id:
        name: Schedule ID
        required: true
        selector:
          text:
    sequence:
      - action: esphome.tesla_ble_delete_precondition_schedule
        data:
          schedule_id: "{{ schedule_id }}"

  tesla_precondition_now:
    alias: Tesla - precondition now
    fields:
      temperature:
        name: Temperature
        default: 21
        selector:
          number:
            min: 15
            max: 28
            step: 0.5
            unit_of_measurement: °C
    sequence:
      - action: esphome.tesla_ble_start_preconditioning
        data:
          temperature: "{{ temperature | default(0) }}"
```

To edit a schedule, copy its `id` from the sensor's `schedules` attribute into the script's **Schedule ID** field and change the other fields. Enabling/disabling a schedule is an edit with `enabled` changed.

#### Displaying the schedules
```yaml
type: markdown
title: Precondition schedules
content: >
  | ID | Days | Ready at | Enabled | One-time |
  |:--:|:--:|:--:|:--:|:--:|
  {% for s in state_attr('sensor.tesla_precondition_schedules', 'schedules') or [] -%}
  | {{ s.id }} | {{ s['*days'] }} | {{ s['*precondition_time'] }} | {{ '✅' if s.enabled else '❌' }} | {{ '✅' if s.one_time else '❌' }} |
  {% endfor %}
```
