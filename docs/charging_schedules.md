## Charging schedules
It is possible to view and delete charging schedules with this project. Charging schedules are handled a little differently to the other sensors and controls and
require a little more expertise in Home Assistant (HA). Many users will not be interested in using this aspect of the project hence they are documented separately.
Key features include:

- As there can be many charging schedules, they are returned to HA through an event, not a sensor (the largest string that can be transferred from ESPHome to an HA
text sensor is 255 characters). The event data needs to be captured into an HA entity where it can be used (in this document capture to a text sensor is described)
- The data is only captured on demand through a button control.
- An individual charging schedule can be deleted using a number control by specifying the charging schedule's ID.

All controls are disabled by default.

My thanks to [iancg](https://github.com/iancg) for his input in using templating to extract the data from the event into a sensor's attirbutes and the use of the markdown card to display the schedules which I've shamelessly plagiarised!
