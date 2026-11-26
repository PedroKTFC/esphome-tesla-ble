## Charging schedules
This project allows you to **view**, **add**, **edit** and **delete** Tesla charging schedules. Preconditioning (departure) schedules work the same way, see [preconditioning.md](./preconditioning.md). Unlike the other sensors and controls, charging schedules are handled differently and require some familiarity with Home Assistant (HA). Since many users will not need this functionality, it is documented separately.

> [!TIP]
> For a ready-made setup (sensors, scripts, a schedule editor and a dashboard for both charging and preconditioning schedules) use the Home Assistant package in [ha/](./ha/), described in [preconditioning.md](./preconditioning.md#home-assistant-package-and-dashboard). The rest of this page explains the underlying events and actions.

### Key features

- Charging schedules are returned to Home Assistant as an **event**, rather than a sensor. This is because a single schedule list can exceed the 255-character limit of an ESPHome text sensor. The event data can then be captured into a Home Assistant entity (this guide demonstrates capturing it in a number sensor).
- Charging schedules are retrieved **on demand** by pressing the **Get charge schedules button**. Note the car must be awake.
- Charging schedules can be added or edited one at a time. A build-time option **allow_setting_schedules** enables/disables them.
- Individual charging schedules can be deleted by specifying the schedule's **ID** using the **Delete a charging schedule** control.
- All charging schedule controls are **disabled by default**.

Thanks to [iancg](https://github.com/iancg) for suggesting the use of templating to extract event data into a sensor's attributes and for the markdown cardapproach  to display the schedules, which I've shamelessly plagiarised!

### Reading charging schedules
To read the charging schedules the car must be awake. The charging schedules stored in the car are retrieved in a single request and returned to HA as JSON via the **esphome.tesla_schedules_updated** event. The event is fired on demand by "pressing" the **Get charge schedules** button (disabled by default). The event includes the following data fields:

| Field | Description |
| --- | --- |
| comment | Informational text. At the time of writing, it notes that fields beginning with * are derived values. |
| count | Number of charging schedules returned. |
| schedules | Charging schedule data in JSON format. |
| version | Project version information. |

Most JSON fields map directly to the data returned by the car. Fields beginning with `*` are derived for convenience. For example, `days` is stored as a bitfield, while `*days` contains a human-readable representation of the selected days.

For the most up-to-date event format, use the **Developer Tools → Events** page in HA to inspect the `esphome.tesla_schedules_updated` event.

### Using and displaying charging schedules in Home Assistant
A convenient way to work with charging schedules in Home Assistant is to capture the event data into the attributes of a dedicated sensor. The schedule data can then be used in dashboards, templates, scripts, and automations.

#### Capturing the event data into a sensor
Adding the following YAML to your `configuration.yaml` updates the **Charging schedules** sensor whenever the `esphome.tesla_schedules_updated` event is received.
```
template:
  - trigger:
      - platform: event
        event_type: esphome.tesla_schedules_updated
    sensor:
      - name: "Charging schedules"
        state: "{{ trigger.event.data.count }}"
        attributes:
          charge_schedules: "{{ (trigger.event.data.schedules | from_json).charge_schedules }}"
```
The sensor's state is set to the number of schedules returned, while the decoded JSON data is stored in the sensor's attributes.

#### Displaying charging schedules
Charging schedules can be displayed in many ways. One simple approach is to use a **Markdown card** which reads the schedule information directly from the sensor attributes. The example below displays the number of schedules together with a table showing each schedule's ID, days, start and end times, location, enabled status, one-time status, and name.
```
type: markdown
content: >
  Number of schedules:  {{states('sensor.charging_schedules')}}

  |#| ID | Days | Start | End | Lat | Long|  Enabled | One-Time | Name |

  |:--:| :--:| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |

  {% set schedules = state_attr('sensor.charging_schedules',
  'charge_schedules')%} {% if schedules %} {% for sched in schedules %} | {{
  loop.index }} | {{ sched['id'] }} | {{ sched['*days'] }} | {{
  sched['*start_time'] }} | {{ sched['*end_time'] }} | {{ sched['latitude'] }} |
  {{ sched['longitude'] }} | {{'✅' if sched['enabled'] else '❌' }} | {{ '✅' if
  sched['one_time'] else '❌' }}| {{sched['name'] }} |

  {% endfor %}

  {% else %} |0| No schedules found | | | | | {%-endif %}
title: Scheduled Charges
```
This produces the following display:

<img width=50%  alt="image" src="https://github.com/user-attachments/assets/4045d442-0f4a-459b-9cff-71fe3ba08241" />

### Creating a charging schedule
The ability to create charging schedules is not enabled by default. The build-time parameter **allow_setting_schedules** must be set to non-zero to enable setting. You must rebuild the code if you change this parameter. The action to set a charging schedule is always available but any request will be rejected (with a helpful warning in the logs) if this parameter is not set non-zero.
```
  allow_setting_schedules: 0 # if = 0 the set_charge_schedule action will reject all requests. MUST rebuild if changed
```
#### Set charge schedule action
The action to set a charge schedule is named **esphome.<your_friendly_name>_set_charge_schedule**. It provides all the fields of the Tesla message apart from ID (which is set internally from the current time), so it always adds a new schedule. They are as follows:
- name - doesn't seem to be used
- days_of_week - 7 bit bit mask
- start_enabled - boolean
- start_time - minutes from 00:00
- end_enabled - boolean
- end_time - minutes from 00:00
- one_time - boolean
- enabled - boolean
- latitude - float
- longitude - float

`days_of_week` bits are Sun = 1, Mon = 2, Tue = 4, Wed = 8, Thu = 16, Fri = 32, Sat = 64 (so Mon-Fri = 62, every day = 127).

#### Edit charge schedule action
**esphome.<your_friendly_name>_edit_charge_schedule** takes the same fields plus **schedule_id**. Give the ID of an existing schedule (from the schedule list) and that schedule is replaced with the values given; leave it empty to add a new schedule. The same build-time option applies.

Some basic validation is performed and a request will be rejected if this validation fails (with a helpful message in the log). Validation includes days_of_week bit mask in range, times minutes do not exceed 24 hours, latitude and longitude are on the globe.

Once the car accepts the schedule, the list of schedules is automatically read back (firing the `esphome.tesla_schedules_updated` event).

I suggest you try it using the **tools** section in Home Assistant.
#### Example script
To make things easy for testing, I used a script to specify a schedule and read back the list of schedules:
```
sequence:
  - action: esphome.tesla_ble_set_charge_schedule
    metadata: {}
    data:
      start_enabled: "{{ start_enabled }}"
      end_enabled: "{{ end_enabled }}"
      one_time: "{{ one_time }}"
      enabled: "{{ enabled }}"
      name: Unused
      days_of_week: "{{ days_of_week }}"
      start_time: "{{ start_time }}"
      end_time: "{{ end_time }}"
      latitude: "{{ state_attr('zone.home', 'latitude')  | float }}"
      longitude: "{{ state_attr('zone.home', 'longitude')  | float }}"
  - action: button.press
    metadata: {}
    target:
      entity_id: button.tesla_ble_get_charge_schedules
    data: {}
alias: Set Tesla charging schedule
description: ""
fields:
  days_of_week:
    selector:
      number:
        min: 1
        max: 127
    required: true
    name: Days of week
  start_enabled:
    selector:
      boolean: {}
    default: true
    required: false
  start_time:
    selector:
      number:
        min: 0
        max: 1440
    name: Start time
    required: true
  end_enabled:
    selector:
      boolean: {}
    name: End enabled
    default: true
  end_time:
    selector:
      number:
        min: 0
        max: 1440
    name: End time
    required: true
  one_time:
    selector:
      boolean: {}
    default: false
  enabled:
    selector:
      boolean: {}
    default: false
```
#### Other actions
- **esphome.<your_friendly_name>_delete_charge_schedule** (`schedule_id`) deletes a schedule; the list is then re-read automatically.
- **esphome.<your_friendly_name>_get_charge_schedules** reads the schedules (same as the button).
- **esphome.<your_friendly_name>_set_scheduled_charging** (`start_time` in minutes after midnight, negative to disable) sets the older single "scheduled charging" start time.

### Deleting a charging schedule
The **Delete a charging schedule** control (disabled by default) allows you to remove an individual charging schedule. Enter the **ID** of the schedule you wish to delete and submit the request. Once the schedule has been deleted, the list of charging schedules is automatically refreshed. Note this control wakes the car if it is asleep.
