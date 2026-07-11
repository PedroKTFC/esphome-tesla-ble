## Charging schedules
It is possible to view and delete charging schedules with this project. Charging schedules are handled a little differently to the other sensors and controls and
require a little more expertise in Home Assistant (HA). Many users will not be interested in using this aspect of the project hence they are documented separately.
Key features include:

- As there can be many charging schedules, they are returned to HA through an event, not a sensor (the largest string that can be transferred from ESPHome to an HA
text sensor is 255 characters). The event data needs to be captured into an HA entity where it can be used (in this document capture to a text sensor is described)
- The data is only captured on demand through a button control.
- An individual charging schedule can be deleted using a number control by specifying the charging schedule's ID.

All controls are disabled by default.

My thanks to [iancg](https://github.com/iancg) for his input in using templating to extract the data from the event into a sensor's attributes and the use of the markdown card to display the schedules which I've shamelessly plagiarised!

# Reading the charging schedules
All the charging schedules in the car are read in one go and returned to HA as JSON data through the event **esphome.tesla_schedules_updated**. The event is fired on demand by "pressing" the button **Get charge schedules** (which is disabled by default). The event includes the following data fields (for the complete and up to date view, use the `developer tools` in HA to see the contents of the event):

| Field | Description |
| --- | --- |
| comment | Some helpful text, at the time of writing: *Fields starting with * are derived* |
| count | The number of schedules returned |
| schedules | The actual schedules in JSON format |
| version | To track the code version |

There is a one-to-one correspondence between the JSON fields in the schedules and the data from the car except for those that start with a * which are derived (for example, *days* is a bitfield whereas **days* is a human readable version decoded from the bitfield). 

# Using and displaying charging schedules in Home Assistant
Probably the easiest way to use charging schedules in HA is to read them into the attributes of a dedicated sensor, where they can then be displayed and read in, for example, automations.

## Reading the event data into a sensor
The following yaml, when added to your configuration.yaml, will cause the sensor `Charging schedules` to be updated whenever the **esphome.tesla_schedules_updated** event fires. The value of the sensor is the number of charging schedules and the JSON data is decoded into the attributes of the sensor.

## Displaying the charging schedules
There are many ways of displaying the charging schedules. This section describes one approach which also shows how to access the charging schedules in the sensor attributes. Using a `markup card`, each charging schedule can be displayed as desired. The following is an example:
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
Which produces the following display:

<img width="792" height="252" alt="image" src="https://github.com/user-attachments/assets/4045d442-0f4a-459b-9cff-71fe3ba08241" />

