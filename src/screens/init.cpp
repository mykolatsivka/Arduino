#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "context.h"
#include "lcd_wrapper.h"
#include "rtc_wrapper.h"
#include "screens.h"
#include "sensors.h"

#define BTN_MODE BTN1_PIN
#define BTN_SELECT BTN2_PIN
#define BTN_UP BTN3_PIN
#define BTN_DOWN BTN4_PIN

void load_alarm_settings(struct context *ctx);

static void context_init(struct context *ctx) {
    ctx->current_screen = CLOCK_SCR;

    ctx->mode_button = { BTN_MODE, HIGH, 0 };
    ctx->select_button = { BTN_SELECT, HIGH, 0 };
    ctx->up_button = { BTN_UP, HIGH, 0 };
    ctx->down_button = { BTN_DOWN, HIGH, 0 };

    ctx->temperature = 0;
    ctx->humidity = 0;
    ctx->last_sensor_read = 0;

    ctx->setting_time = false;
    ctx->time_field = 0;
    ctx->set_hour = 0;
    ctx->set_minute = 0;

    ctx->setting_date = false;
    ctx->date_field = 0;
    ctx->set_day = 1;
    ctx->set_month = 1;
    ctx->set_year = 2026;

    ctx->setting_alarm = false;
    ctx->alarm_field = 0;
    ctx->alarm_hour = 7;
    ctx->alarm_minute = 0;
    ctx->alarm_enabled = false;
    ctx->alarm_ringing = false;
    ctx->last_alarm_key = -1;

    ctx->snooze_active = false;
    ctx->snooze_hour = 0;
    ctx->snooze_minute = 0;

    ctx->stopwatch_running = false;
    ctx->stopwatch_started_at = 0;
    ctx->stopwatch_elapsed_before_start = 0;

    ctx->timer_setting = true;
    ctx->timer_field = 0;
    ctx->timer_minutes = 1;
    ctx->timer_seconds = 0;
    ctx->timer_running = false;
    ctx->timer_done = false;
    ctx->timer_ends_at = 0;
    ctx->timer_remaining_ms = 60000;
}

enum screen init_screen(struct context *ctx) {
    init();
    context_init(ctx);

    Wire.begin();
    Serial.begin(BAUD_RATE);

    pinMode(BTN_MODE, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);

    lcd_init();
    lcd_backlight(true);
    lcd_clear();

    clock_init();
    sensors_init();
    load_alarm_settings(ctx);

    lcd_print_line(0, "Alarm Clock");
    lcd_print_line(1, "Ready");
    delay(900);
    lcd_clear();

    return CLOCK_SCR;
}
