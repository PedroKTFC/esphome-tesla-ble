## Charging schedules
This project allows you to **view** and **delete** Tesla charging schedules. Unlike the other sensors and controls, charging schedules are handled differently and require some familiarity with Home Assistant (HA). Since many users will not need this functionality, it is documented separately.

### Key features

- Charging schedules are returned to Home Assistant as an **event**, rather than a sensor. This is because a single schedule list can exceed the 255-character limit of an ESPHome text sensor. The event data can then be captured into a Home Assistant entity (this guide demonstrates capturing it in a number sensor).
- Charging schedules are retrieved **on demand** by pressing the **Get charge schedules button**. Note the car must be awake.
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

### Deleting a charging schedule
The **Delete a charging schedule** control (disabled by default) allows you to remove an individual charging schedule. Enter the **ID** of the schedule you wish to delete and submit the request. Once the schedule has been deleted, the list of charging schedules is automatically refreshed. Note this control wakes the car if it is asleep.
