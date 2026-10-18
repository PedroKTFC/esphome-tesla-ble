# Charging freshness

Home Assistant [can skip identical ESPHome readings](https://github.com/home-assistant/core/blob/2026.10.2/homeassistant/components/esphome/entry_data.py#L449-L475), leaving `last_reported` unchanged. An automation can then mistake a steady reading for stale data. The general `Last update` also includes unrelated vehicle data.

The optional `charging-freshness.yml` package records when the bridge receives each charging input:

| Entity | Source |
| --- | --- |
| `Charge level received` | Battery percentage |
| `Charge power received` | Charging power |
| `Charging state received` | Charging state |

## Setup

Add this entry after `packages/common.yml` and `packages/client.yml` in the example configuration's package list, then rebuild and install your firmware:

```yaml
      - packages/charging-freshness.yml
```

Each timestamp advances on a valid raw sample, including repeated values. Missing samples leave it unchanged; invalid samples or an unsynchronized clock produce `unavailable`. Timestamps have no periodic refresh or reboot persistence. API replay retains the old timestamp value.

## Use

Check each timestamp's **state value** and the corresponding reading's validity. Replace this example entity ID and choose a timeout that suits your polling intervals:

```jinja
{% set received = as_timestamp(states('sensor.tesla_ble_charge_level_received'), 0) %}
{{ received > 0 and 0 <= as_timestamp(now()) - received <= 180 }}
```

These are bridge receipt times with one-second resolution. They precede user filters and cannot establish the age of data cached inside the car. Account for delayed or suppressed filtered readings separately. Enabling the three diagnostic entities adds recorder activity.

## Verify

Build the dummy fixture with `esphome compile tests/charging-freshness.yml`; do not flash it. On your own device, confirm that repeated readings advance their timestamps and a quiet period or HA reconnect does not. Firmware compilation alone does not prove that behavior on hardware.
