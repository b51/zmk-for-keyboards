/*
 * Custom ZMK behavior: Force RGB underglow ON after waking from deep sleep.
 *
 * Problem: CONFIG_ZMK_RGB_UNDERGLOW_AUTO_OFF_IDLE saves state.on=false to flash
 * before sys_poweroff(). On next boot, Zephyr settings subsystem restores that
 * state AFTER CONFIG_ZMK_RGB_UNDERGLOW_ON_START has already set state.on=true,
 * overwriting it. Result: LEDs stay off after waking from sleep.
 *
 * Solution: This module schedules a delayed work item to force RGB on ~2 seconds
 * after boot, well after settings load has completed. The delay is invisible to
 * the user (BLE pairing takes longer).
 *
 * For the Sofle RGB split keyboard (nice_nano_v2, nRF52840).
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>

#include <zmk/rgb_underglow.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* ── delayed work: force RGB on ─────────────────────────────────────── */

static void rgb_wake_work_handler(struct k_work *work) {
    int ret = zmk_rgb_underglow_on();
    if (ret == 0) {
        LOG_DBG("rgb_wake: forced RGB on after settings load");
    } else {
        LOG_WRN("rgb_wake: zmk_rgb_underglow_on() returned %d", ret);
    }
}

static K_WORK_DELAYABLE_DEFINE(rgb_wake_work, rgb_wake_work_handler);

/* ── SYS_INIT entry: schedule the delayed work ──────────────────────── */

static int rgb_wake_init(void) {
    /*
     * 2-second delay ensures Zephyr settings subsystem has finished loading
     * and restoring saved state from flash. At this point the saved "off"
     * state has already been applied; we override it with a forced ON.
     */
    k_work_schedule(&rgb_wake_work, K_SECONDS(2));
    return 0;
}

/*
 * Priority 99 (highest number in APPLICATION band) runs as late as possible
 * in the APPLICATION init phase, but we still use the delayed work for safety
 * because settings_load may use a workqueue and hasn't completed by SYS_INIT
 * time regardless of priority.
 */
SYS_INIT(rgb_wake_init, APPLICATION, 99);
