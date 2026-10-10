#define DT_DRV_COMPAT zmk_behavior_bat_print

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/battery.h>
#include <dt-bindings/zmk/keys.h>
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
#include <zmk/split/central.h>
#endif

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define KP_DEV DEVICE_DT_NAME(DT_INST(0, zmk_behavior_key_press))

struct behavior_bat_print_config {
	uint32_t tap_ms;
	uint32_t wait_ms;
};

static const uint32_t digit_keycodes[] = {N0, N1, N2, N3, N4, N5, N6, N7, N8, N9};

static void tap_key(const struct behavior_bat_print_config *cfg,
		    struct zmk_behavior_binding_event *event, uint32_t keycode)
{
	struct zmk_behavior_binding binding = {.behavior_dev = KP_DEV, .param1 = keycode};

	zmk_behavior_queue_add(event, binding, true, cfg->tap_ms);
	zmk_behavior_queue_add(event, binding, false, cfg->wait_ms);
}

static void tap_number(const struct behavior_bat_print_config *cfg,
		       struct zmk_behavior_binding_event *event, uint8_t pct)
{
	if (pct > 100) {
		pct = 100;
	}
	if (pct >= 100) {
		tap_key(cfg, event, digit_keycodes[pct / 100]);
	}
	if (pct >= 10) {
		tap_key(cfg, event, digit_keycodes[(pct / 10) % 10]);
	}
	tap_key(cfg, event, digit_keycodes[pct % 10]);
}

static int on_bat_print_binding_pressed(struct zmk_behavior_binding *binding,
					struct zmk_behavior_binding_event event)
{
	const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
	const struct behavior_bat_print_config *cfg = dev->config;
	uint8_t pct = zmk_battery_state_of_charge();

	LOG_DBG("bat_print: central %d%%", pct);
	tap_number(cfg, &event, pct);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
	for (uint8_t i = 0; i < ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT; i++) {
		uint8_t periph_pct;

		if (zmk_split_central_get_peripheral_battery_level(i, &periph_pct) == 0) {
			LOG_DBG("bat_print: peripheral %d %d%%", i, periph_pct);
			tap_key(cfg, &event, SPACE);
			tap_number(cfg, &event, periph_pct);
		}
	}
#endif

	return ZMK_BEHAVIOR_OPAQUE;
}

static int on_bat_print_binding_released(struct zmk_behavior_binding *binding,
					 struct zmk_behavior_binding_event event)
{
	return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_bat_print_driver_api = {
	.binding_pressed = on_bat_print_binding_pressed,
	.binding_released = on_bat_print_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
	.get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define BAT_PRINT_INST(n)                                                                          \
	static const struct behavior_bat_print_config behavior_bat_print_config_##n = {            \
		.tap_ms = DT_INST_PROP(n, tap_ms),                                                 \
		.wait_ms = DT_INST_PROP(n, wait_ms),                                               \
	};                                                                                         \
	BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &behavior_bat_print_config_##n, POST_KERNEL,  \
				CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                               \
				&behavior_bat_print_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BAT_PRINT_INST)
