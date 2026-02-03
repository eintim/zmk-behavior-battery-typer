#define DT_DRV_COMPAT zmk_behavior_bat_print

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/battery.h>
#include <zmk/events/keycode_state_changed.h>
#include <dt-bindings/zmk/keys.h>
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
#include <zmk/split/central.h>
#endif

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#define BAT_PRINT_MAX_DIGITS 3

static const uint32_t digit_keycodes[] = {N0, N1, N2, N3, N4, N5, N6, N7, N8, N9};

static uint8_t bat_print_digits[BAT_PRINT_MAX_DIGITS];
static uint8_t bat_print_num_digits;
static uint8_t bat_print_periph_digits[BAT_PRINT_MAX_DIGITS];
static uint8_t bat_print_periph_num_digits;

static void pct_to_digits(uint8_t pct, uint8_t *digits, uint8_t *num_digits)
{
	if (pct > 100) {
		pct = 100;
	}
	if (pct >= 100) {
		digits[0] = 1;
		digits[1] = 0;
		digits[2] = 0;
		*num_digits = 3;
	} else if (pct >= 10) {
		digits[0] = (uint8_t)(pct / 10);
		digits[1] = (uint8_t)(pct % 10);
		*num_digits = 2;
	} else {
		digits[0] = pct;
		*num_digits = 1;
	}
}

static void bat_print_work_handler(struct k_work *work)
{
	int64_t ts;

	for (uint8_t i = 0; i < bat_print_num_digits; i++) {
		uint8_t d = bat_print_digits[i];
		if (d > 9) {
			continue;
		}
		ts = k_uptime_get();
		raise_zmk_keycode_state_changed_from_encoded(digit_keycodes[d], true, ts);
		k_msleep(20);
		ts = k_uptime_get();
		raise_zmk_keycode_state_changed_from_encoded(digit_keycodes[d], false, ts);
		k_msleep(10);
	}

	if (bat_print_periph_num_digits > 0) {
		ts = k_uptime_get();
		raise_zmk_keycode_state_changed_from_encoded(SPC, true, ts);
		k_msleep(20);
		ts = k_uptime_get();
		raise_zmk_keycode_state_changed_from_encoded(SPC, false, ts);
		k_msleep(10);

		for (uint8_t i = 0; i < bat_print_periph_num_digits; i++) {
			uint8_t d = bat_print_periph_digits[i];
			if (d > 9) {
				continue;
			}
			ts = k_uptime_get();
			raise_zmk_keycode_state_changed_from_encoded(digit_keycodes[d], true, ts);
			k_msleep(20);
			ts = k_uptime_get();
			raise_zmk_keycode_state_changed_from_encoded(digit_keycodes[d], false, ts);
			k_msleep(10);
		}
	}
}

K_WORK_DEFINE(bat_print_work, bat_print_work_handler);

static int on_bat_print_binding_pressed(struct zmk_behavior_binding *binding,
					struct zmk_behavior_binding_event event)
{
	uint8_t pct = zmk_battery_state_of_charge();

	pct_to_digits(pct, bat_print_digits, &bat_print_num_digits);

	bat_print_periph_num_digits = 0;
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
	uint8_t periph_pct;
	if (zmk_split_central_get_peripheral_battery_level(0, &periph_pct) == 0) {
		pct_to_digits(periph_pct, bat_print_periph_digits, &bat_print_periph_num_digits);
	}
#endif

	k_work_submit(&bat_print_work);
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
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
			CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
			&behavior_bat_print_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
