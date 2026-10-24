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

### Home Assistant package and dashboard
Everything needed on the Home Assistant side is in two files you can paste as they are:

| File | What it contains |
| --- | --- |
| [ha/tesla_ble_schedules.yaml](./ha/tesla_ble_schedules.yaml) | A [package](https://www.home-assistant.io/docs/configuration/packages/): the two schedule sensors, the scripts, and the schedule editor (helpers and two automations). Install steps are at the top of the file. |
| [ha/tesla_ble_dashboard.yaml](./ha/tesla_ble_dashboard.yaml) | A summary card for your car view and an editor subview. Only built-in cards are used. |

Both assume the ESPHome device name `tesla-ble` (entities `*.tesla_ble_*`, actions `esphome.tesla_ble_*`). If yours differs, search-and-replace `tesla_ble_` in both files.

#### Sensors
- **Tesla charge schedules** / **Tesla precondition schedules**: the state is the number of schedules; the `schedules` attribute holds the list exactly as the car reports it (see the event fields above). They update whenever the lists are read, which happens automatically after every add, edit or delete.

#### Scripts
All scripts that talk to the car call **Tesla - ensure awake** first. It presses **Wake up** if the car is asleep, waits up to 60 s for **Asleep** to turn off, and stops with an error if the car does not wake, so nothing is sent to a sleeping car.

| Script | Use |
| --- | --- |
| `script.tesla_ensure_awake` | Wake the car and wait for it (used by all the others). |
| `script.tesla_refresh_schedules` | Read both schedule lists. |
| `script.tesla_precondition_now` | Start preconditioning now at a given temperature. |
| `script.tesla_set_precondition_schedule` | Add, or edit by `schedule_id`, a preconditioning schedule: time, day names, one time, enabled, location (zone, or latitude/longitude). |
| `script.tesla_set_charge_schedule` | The same for a charging schedule, with start/end times. |
| `script.tesla_delete_schedule` | Delete one schedule (`kind` + `schedule_id`). |

These are also the building blocks for your own automations, for example adding a one-time preconditioning schedule from a calendar event.

#### Schedule editor
The dashboard subview lets you edit schedules without knowing their IDs:

1. Pick a schedule in the **Schedule** dropdown (entries look like `🌡️ 07:30 · Mon, Tue, Wed, Thu, Fri · Weekly · #1792681416`), or pick **➕ New precondition schedule** / **➕ New charge schedule**.
2. The fields below fill in with its values: time(s), one time, enabled and a toggle per day.
3. **Save to car** edits the selected schedule (it keeps its location) or adds the new one at Home. **Delete selected** removes only the selected schedule, after a confirmation.

How it works: the *sync list* automation rebuilds the dropdown whenever a schedule sensor changes, and keeps the current selection if it still exists. The *load selection* automation copies the picked schedule into the helpers. The schedule ID is the `#…` suffix of the dropdown entry, so there is no separate ID field to get out of step.

> [!NOTE]
> It is not yet confirmed whether the car deletes a one-time schedule after it has run, or keeps it disabled. If it keeps them, delete old ones now and then: the list is limited to about ten schedules of each type (see above).
