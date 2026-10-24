#include <errno.h>
#include <stdbool.h>

#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/rgb_underglow.h>

LOG_MODULE_REGISTER(rgb_battery_indicator, CONFIG_ZMK_LOG_LEVEL);

enum {
    RGB_UNDERGLOW_EFFECT_SOLID = 0,
    BATTERY_LOW_THRESHOLD = 50,
    BATTERY_FULL = 100,
};

static int battery_indicator_listener(const zmk_event_t *event) {
    const struct zmk_battery_state_changed *battery = as_zmk_battery_state_changed(event);
    if (battery == NULL) {
        return -ENOTSUP;
    }

    bool battery_is_low = battery->state_of_charge <= BATTERY_LOW_THRESHOLD;
    bool battery_is_full = battery->state_of_charge >= BATTERY_FULL;
    struct zmk_led_hsb color = {
        .h = battery_is_full ? 120 : 0,
        .s = 100,
        .b = (battery_is_low || battery_is_full) ? 100 : 0,
    };

    int rc = zmk_rgb_underglow_set_hsb(color);
    if (rc < 0) {
        LOG_ERR("Failed to set battery underglow color: %d", rc);
        return rc;
    }

    rc = zmk_rgb_underglow_select_effect(RGB_UNDERGLOW_EFFECT_SOLID);
    if (rc < 0) {
        LOG_ERR("Failed to select solid underglow effect: %d", rc);
        return rc;
    }

    return 0;
}

ZMK_LISTENER(rgb_battery_indicator, battery_indicator_listener);
ZMK_SUBSCRIPTION(rgb_battery_indicator, zmk_battery_state_changed);
