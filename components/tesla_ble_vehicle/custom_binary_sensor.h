#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"

// https://github.com/esphome/feature-requests/issues/2324
namespace esphome
{
    namespace binary_sensor
    {

        class CustomBinarySensor : public BinarySensor {
        public:
            void set_custom_state(bool state) {
                this->publish_state(state);  // Publish that we have a state
            }
        };

    } // namespace binary_sensor
} // namespace esphome
