#include <Arduino.h>
#include <EEPROM.h>

#include "config.h"
#include "context.h"
#include "lcd_wrapper.h"
#include "rtc_wrapper.h"
#include "screens.h"
#include "sensors.h"

#define EEPROM_MAGIC_ADDR 0
#define EEPROM_ALARM_HOUR_ADDR 1
#define EEPROM_ALARM_MINUTE_ADDR 2
#define EEPROM_ALARM_ENABLED_ADDR 3
#define EEPROM_MAGIC_VALUE 0xA6

static int days_in_month(byte month, int year) {
    if (month == 2) {
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return leap ? 29 : 28;
    }

    if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }

    return 31;
}

static bool read_pressed(struct button_state *button) {
    bool current_state = digitalRead(button->pin);

    if (current_state != button->last_state && millis() - button->last_change > 60) {
        button->last_change = millis();
        button->last_state = current_state;
        return current_state == LOW;
    }

    return false;
}

static void save_alarm_settings(struct context *ctx) {
    EEPROM.update(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
    EEPROM.update(EEPROM_ALARM_HOUR_ADDR, ctx->alarm_hour);
    EEPROM.update(EEPROM_ALARM_MINUTE_ADDR, ctx->alarm_minute);
    EEPROM.update(EEPROM_ALARM_ENABLED_ADDR, ctx->alarm_enabled ? 1 : 0);
}

void load_alarm_settings(struct context *ctx) {
    if (EEPROM.read(EEPROM_MAGIC_ADDR) != EEPROM_MAGIC_VALUE) {
        ctx->alarm_hour = 7;
        ctx->alarm_minute = 0;
        ctx->alarm_enabled = false;
        save_alarm_settings(ctx);
        return;
    }

    ctx->alarm_hour = EEPROM.read(EEPROM_ALARM_HOUR_ADDR);
    ctx->alarm_minute = EEPROM.read(EEPROM_ALARM_MINUTE_ADDR);
    ctx->alarm_enabled = EEPROM.read(EEPROM_ALARM_ENABLED_ADDR) == 1;

    if (ctx->alarm_hour > 23) ctx->alarm_hour = 7;
    if (ctx->alarm_minute > 59) ctx->alarm_minute = 0;
}

static void next_screen(struct context *ctx) {
    if (ctx->current_screen == ALARM_SCR) {
        ctx->current_screen = CLOCK_SCR;
    } else if (ctx->current_screen >= FACTORY_RESET_SCR) {
        ctx->current_screen = CLOCK_SCR;
    } else {
        ctx->current_screen++;
    }

    ctx->setting_time = false;
    ctx->setting_date = false;
    ctx->setting_alarm = false;
    lcd_clear();
}

static void read_sensor(struct context *ctx) {
    if (millis() - ctx->last_sensor_read < 2000) {
        return;
    }

    float new_temperature = get_temperature();
    int new_humidity = get_humidity();

    if (!isnan(new_temperature) && new_humidity >= 0) {
        ctx->temperature = new_temperature;
        ctx->humidity = new_humidity;
    }

    ctx->last_sensor_read = millis();
}

static void start_set_time(struct context *ctx) {
    struct dt current = now();
    ctx->set_hour = current.hours;
    ctx->set_minute = current.minutes;
    ctx->time_field = 0;
    ctx->setting_time = true;
    lcd_clear();
}

static void save_set_time(struct context *ctx) {
    struct dt current = now();
    set_datetime(current.day, current.month, current.year, ctx->set_hour, ctx->set_minute, 0);

    ctx->setting_time = false;
    ctx->time_field = 0;

    char line[17];
    lcd_print_line(0, "Time saved");
    snprintf(line, sizeof(line), "%02u:%02u:00", ctx->set_hour, ctx->set_minute);
    lcd_print_line(1, line);
    delay(700);
    lcd_clear();
}

static void change_set_time(struct context *ctx, int direction) {
    if (!ctx->setting_time) return;

    if (ctx->time_field == 0) {
        int value = ctx->set_hour + direction;
        if (value > 23) value = 0;
        if (value < 0) value = 23;
        ctx->set_hour = value;
    } else {
        int value = ctx->set_minute + direction;
        if (value > 59) value = 0;
        if (value < 0) value = 59;
        ctx->set_minute = value;
    }
}

static void start_set_date(struct context *ctx) {
    struct dt current = now();
    ctx->set_day = current.day;
    ctx->set_month = current.month;
    ctx->set_year = current.year;

    if (ctx->set_year < 2024 || ctx->set_year > 2099) ctx->set_year = 2026;
    if (ctx->set_month < 1 || ctx->set_month > 12) ctx->set_month = 1;
    if (ctx->set_day < 1 || ctx->set_day > days_in_month(ctx->set_month, ctx->set_year)) {
        ctx->set_day = 1;
    }

    ctx->date_field = 0;
    ctx->setting_date = true;
    lcd_clear();
}

static void save_set_date(struct context *ctx) {
    struct dt current = now();
    set_datetime(ctx->set_day, ctx->set_month, ctx->set_year,
                 current.hours, current.minutes, current.seconds);

    ctx->setting_date = false;
    ctx->date_field = 0;

    lcd_print_line(0, "Date saved");
    lcd_clear_line(1);
    delay(700);
    lcd_clear();
}

static void change_set_date(struct context *ctx, int direction) {
    if (!ctx->setting_date) return;

    if (ctx->date_field == 0) {
        int max_day = days_in_month(ctx->set_month, ctx->set_year);
        int value = ctx->set_day + direction;
        if (value > max_day) value = 1;
        if (value < 1) value = max_day;
        ctx->set_day = value;
    } else if (ctx->date_field == 1) {
        int value = ctx->set_month + direction;
        if (value > 12) value = 1;
        if (value < 1) value = 12;
        ctx->set_month = value;

        int max_day = days_in_month(ctx->set_month, ctx->set_year);
        if (ctx->set_day > max_day) ctx->set_day = max_day;
    } else {
        int value = ctx->set_year + direction;
        if (value > 2099) value = 2024;
        if (value < 2024) value = 2099;
        ctx->set_year = value;

        int max_day = days_in_month(ctx->set_month, ctx->set_year);
        if (ctx->set_day > max_day) ctx->set_day = max_day;
    }
}

static void start_set_alarm(struct context *ctx) {
    ctx->alarm_field = 0;
    ctx->setting_alarm = true;
    lcd_clear();
}

static void save_set_alarm(struct context *ctx) {
    save_alarm_settings(ctx);

    ctx->setting_alarm = false;
    ctx->alarm_field = 0;

    lcd_print_line(0, "Alarm saved");
    lcd_clear_line(1);
    delay(700);
    lcd_clear();
}

static void change_set_alarm(struct context *ctx, int direction) {
    if (!ctx->setting_alarm) return;

    if (ctx->alarm_field == 0) {
        int value = ctx->alarm_hour + direction;
        if (value > 23) value = 0;
        if (value < 0) value = 23;
        ctx->alarm_hour = value;
    } else if (ctx->alarm_field == 1) {
        int value = ctx->alarm_minute + direction;
        if (value > 59) value = 0;
        if (value < 0) value = 59;
        ctx->alarm_minute = value;
    } else {
        ctx->alarm_enabled = !ctx->alarm_enabled;
    }
}

static void start_alarm_ring(struct context *ctx) {
    ctx->alarm_ringing = true;
    ctx->current_screen = ALARM_SCR;
    lcd_clear();
}

static void stop_alarm_ring(struct context *ctx) {
    ctx->alarm_ringing = false;
    noTone(BUZZER_PIN);
    ctx->current_screen = CLOCK_SCR;
    lcd_clear();
}

static void snooze_alarm(struct context *ctx) {
    struct dt current = now();
    int total_minutes = current.hours * 60 + current.minutes + 5;

    ctx->snooze_hour = (total_minutes / 60) % 24;
    ctx->snooze_minute = total_minutes % 60;
    ctx->snooze_active = true;

    ctx->alarm_ringing = false;
    noTone(BUZZER_PIN);
    ctx->current_screen = CLOCK_SCR;
    lcd_clear();
}

static void check_alarm(struct context *ctx) {
    if (ctx->alarm_ringing) return;

    struct dt current = now();
    int current_key = current.day * 1440 + current.hours * 60 + current.minutes;

    if (ctx->snooze_active && current.hours == ctx->snooze_hour && current.minutes == ctx->snooze_minute) {
        ctx->snooze_active = false;
        start_alarm_ring(ctx);
        return;
    }

    if (!ctx->alarm_enabled) return;

    if (current.hours == ctx->alarm_hour && current.minutes == ctx->alarm_minute && current_key != ctx->last_alarm_key) {
        ctx->last_alarm_key = current_key;
        start_alarm_ring(ctx);
    }
}

static unsigned long stopwatch_elapsed(struct context *ctx) {
    if (!ctx->stopwatch_running) {
        return ctx->stopwatch_elapsed_before_start;
    }

    return ctx->stopwatch_elapsed_before_start + (millis() - ctx->stopwatch_started_at);
}

static void toggle_stopwatch(struct context *ctx) {
    if (ctx->stopwatch_running) {
        ctx->stopwatch_elapsed_before_start = stopwatch_elapsed(ctx);
        ctx->stopwatch_running = false;
    } else {
        ctx->stopwatch_started_at = millis();
        ctx->stopwatch_running = true;
    }
}

static void reset_stopwatch(struct context *ctx) {
    ctx->stopwatch_running = false;
    ctx->stopwatch_started_at = millis();
    ctx->stopwatch_elapsed_before_start = 0;
}

static void start_timer(struct context *ctx) {
    unsigned long duration = ((unsigned long)ctx->timer_minutes * 60UL + ctx->timer_seconds) * 1000UL;
    if (duration == 0) return;

    ctx->timer_remaining_ms = duration;
    ctx->timer_ends_at = millis() + ctx->timer_remaining_ms;
    ctx->timer_running = true;
    ctx->timer_setting = false;
    ctx->timer_done = false;
}

static void pause_timer(struct context *ctx) {
    if (!ctx->timer_running) return;

    unsigned long now_ms = millis();
    ctx->timer_remaining_ms = ctx->timer_ends_at > now_ms ? ctx->timer_ends_at - now_ms : 0;
    ctx->timer_running = false;
}

static void resume_timer(struct context *ctx) {
    if (ctx->timer_running || ctx->timer_done || ctx->timer_remaining_ms == 0) return;

    ctx->timer_ends_at = millis() + ctx->timer_remaining_ms;
    ctx->timer_running = true;
}

static void reset_timer(struct context *ctx) {
    ctx->timer_running = false;
    ctx->timer_done = false;
    ctx->timer_setting = true;
    ctx->timer_field = 0;
    ctx->timer_remaining_ms = ((unsigned long)ctx->timer_minutes * 60UL + ctx->timer_seconds) * 1000UL;
    noTone(BUZZER_PIN);
}

static void change_timer_value(struct context *ctx, int direction) {
    if (!ctx->timer_setting) return;

    if (ctx->timer_field == 0) {
        int value = ctx->timer_minutes + direction;
        if (value > 99) value = 0;
        if (value < 0) value = 99;
        ctx->timer_minutes = value;
    } else {
        int value = ctx->timer_seconds + direction;
        if (value > 59) value = 0;
        if (value < 0) value = 59;
        ctx->timer_seconds = value;
    }

    ctx->timer_remaining_ms = ((unsigned long)ctx->timer_minutes * 60UL + ctx->timer_seconds) * 1000UL;
}

static void update_timer(struct context *ctx) {
    if (!ctx->timer_running) return;

    if ((long)(millis() - ctx->timer_ends_at) >= 0) {
        ctx->timer_running = false;
        ctx->timer_done = true;
        ctx->timer_remaining_ms = 0;
        ctx->current_screen = TIMER_SCR;
        lcd_clear();
    }
}

static void factory_reset(struct context *ctx) {
    ctx->alarm_hour = 7;
    ctx->alarm_minute = 0;
    ctx->alarm_enabled = false;
    ctx->snooze_active = false;
    save_alarm_settings(ctx);
    reset_saved_datetime();

    reset_stopwatch(ctx);
    ctx->timer_minutes = 1;
    ctx->timer_seconds = 0;
    reset_timer(ctx);

    lcd_print_line(0, "Factory reset");
    lcd_print_line(1, "Done");
    delay(900);
    lcd_clear();
}

static void handle_buttons(struct context *ctx) {
    bool mode = read_pressed(&ctx->mode_button);
    bool select = read_pressed(&ctx->select_button);
    bool up = read_pressed(&ctx->up_button);
    bool down = read_pressed(&ctx->down_button);

    if (ctx->alarm_ringing) {
        if (select) {
            snooze_alarm(ctx);
        } else if (mode || up || down) {
            stop_alarm_ring(ctx);
        }
        return;
    }

    if (ctx->timer_done && (mode || select || up || down)) {
        ctx->timer_done = false;
        reset_timer(ctx);
        lcd_clear();
        if (mode) next_screen(ctx);
        return;
    }

    if (mode) {
        next_screen(ctx);
        return;
    }

    if (select) {
        if (ctx->current_screen == SET_TIME_SCR) {
            if (!ctx->setting_time) {
                start_set_time(ctx);
            } else {
                ctx->time_field++;
                if (ctx->time_field > 1) save_set_time(ctx);
            }
        } else if (ctx->current_screen == SET_DATE_SCR) {
            if (!ctx->setting_date) {
                start_set_date(ctx);
            } else {
                ctx->date_field++;
                if (ctx->date_field > 2) save_set_date(ctx);
            }
        } else if (ctx->current_screen == SET_ALARM_SCR) {
            if (!ctx->setting_alarm) {
                start_set_alarm(ctx);
            } else {
                ctx->alarm_field++;
                if (ctx->alarm_field > 2) save_set_alarm(ctx);
            }
        } else if (ctx->current_screen == STOPWATCH_SCR) {
            toggle_stopwatch(ctx);
        } else if (ctx->current_screen == TIMER_SCR) {
            if (ctx->timer_setting) {
                if (ctx->timer_field == 0) {
                    ctx->timer_field = 1;
                } else {
                    start_timer(ctx);
                }
            } else if (ctx->timer_running) {
                pause_timer(ctx);
            } else {
                resume_timer(ctx);
            }
        } else if (ctx->current_screen == FACTORY_RESET_SCR) {
            factory_reset(ctx);
        }
    }

    if (up) {
        if (ctx->current_screen == SET_TIME_SCR) change_set_time(ctx, 1);
        else if (ctx->current_screen == SET_DATE_SCR) change_set_date(ctx, 1);
        else if (ctx->current_screen == SET_ALARM_SCR) change_set_alarm(ctx, 1);
        else if (ctx->current_screen == STOPWATCH_SCR) reset_stopwatch(ctx);
        else if (ctx->current_screen == TIMER_SCR) change_timer_value(ctx, 1);
    }

    if (down) {
        if (ctx->current_screen == SET_TIME_SCR) change_set_time(ctx, -1);
        else if (ctx->current_screen == SET_DATE_SCR) change_set_date(ctx, -1);
        else if (ctx->current_screen == SET_ALARM_SCR) change_set_alarm(ctx, -1);
        else if (ctx->current_screen == TIMER_SCR) change_timer_value(ctx, -1);
    }
}

static void update_buzzer_and_led(struct context *ctx) {
    bool ring = ctx->alarm_ringing || ctx->timer_done;

    if (ring) {
        if ((millis() / 250) % 2 == 0) {
            tone(BUZZER_PIN, 1000);
            digitalWrite(STATUS_LED_PIN, HIGH);
        } else {
            noTone(BUZZER_PIN);
            digitalWrite(STATUS_LED_PIN, LOW);
        }
        return;
    }

    noTone(BUZZER_PIN);
    digitalWrite(STATUS_LED_PIN, ctx->alarm_enabled ? HIGH : LOW);
}

static void show_clock(struct context *ctx) {
    struct dt current = now();
    char line[17];

    lcd_print_line(0, ctx->alarm_enabled ? "Time        AL" : "Time");
    snprintf(line, sizeof(line), "%02u:%02u:%02u",
             current.hours, current.minutes, current.seconds);

    lcd_set_cursor(1, 0);
    lcd_print(line);

    if (ctx->snooze_active) {
        lcd_set_cursor(1, 12);
        lcd_print("SNZ");
    } else {
        lcd_print("        ");
    }
}

static void show_date() {
    struct dt current = now();
    char line[17];

    lcd_print_line(0, "Date");
    snprintf(line, sizeof(line), "%02u.%02u.%04d",
             current.day, current.month, current.year);

    lcd_print_line(1, line);
}

static void show_env(struct context *ctx) {
    char line[17];
    int rounded_temperature = (int)(ctx->temperature + 0.5);

    lcd_print_line(0, "Temp/Humidity");
    snprintf(line, sizeof(line), "%dC %d%%", rounded_temperature, ctx->humidity);
    lcd_print_line(1, line);
}

static void show_set_time(struct context *ctx) {
    char line[17];

    if (!ctx->setting_time) {
        lcd_print_line(0, "Set Time");
        lcd_print_line(1, "SELECT to edit");
        return;
    }

    lcd_print_line(0, ctx->time_field == 0 ? "Edit hour" : "Edit minute");
    snprintf(line, sizeof(line), "%02u:%02u", ctx->set_hour, ctx->set_minute);
    lcd_print_line(1, line);
}

static void show_set_date(struct context *ctx) {
    char line[17];

    if (!ctx->setting_date) {
        lcd_print_line(0, "Set Date");
        lcd_print_line(1, "SELECT to edit");
        return;
    }

    if (ctx->date_field == 0) lcd_print_line(0, "Edit day");
    else if (ctx->date_field == 1) lcd_print_line(0, "Edit month");
    else lcd_print_line(0, "Edit year");

    snprintf(line, sizeof(line), "%02u.%02u.%04d",
             ctx->set_day, ctx->set_month, ctx->set_year);
    lcd_print_line(1, line);
}

static void show_set_alarm(struct context *ctx) {
    char line[17];

    if (!ctx->setting_alarm) {
        lcd_print_line(0, "Set Alarm");
        snprintf(line, sizeof(line), "%02u:%02u %s",
                 ctx->alarm_hour, ctx->alarm_minute, ctx->alarm_enabled ? "ON" : "OFF");
        lcd_print_line(1, line);
        return;
    }

    if (ctx->alarm_field == 0) lcd_print_line(0, "Edit alarm hour");
    else if (ctx->alarm_field == 1) lcd_print_line(0, "Edit alarm min");
    else lcd_print_line(0, "Alarm ON/OFF");

    snprintf(line, sizeof(line), "%02u:%02u %s",
             ctx->alarm_hour, ctx->alarm_minute, ctx->alarm_enabled ? "ON" : "OFF");
    lcd_print_line(1, line);
}

static void show_stopwatch(struct context *ctx) {
    unsigned long elapsed = stopwatch_elapsed(ctx);
    unsigned long hundredths = (elapsed / 10) % 100;
    unsigned long seconds = (elapsed / 1000) % 60;
    unsigned long minutes = (elapsed / 60000) % 100;
    char line[17];

    lcd_print_line(0, ctx->stopwatch_running ? "Stopwatch run" : "Stopwatch");
    snprintf(line, sizeof(line), "%02lu:%02lu.%02lu",
             minutes, seconds, hundredths);
    lcd_print_line(1, line);
}

static void show_timer(struct context *ctx) {
    char line[17];
    unsigned long remaining = ctx->timer_remaining_ms;

    if (ctx->timer_running) {
        unsigned long now_ms = millis();
        remaining = ctx->timer_ends_at > now_ms ? ctx->timer_ends_at - now_ms : 0;
    }

    if (ctx->timer_done) {
        lcd_print_line(0, "Timer done");
        lcd_print_line(1, "Any key stops");
        return;
    }

    if (ctx->timer_setting) {
        lcd_print_line(0, ctx->timer_field == 0 ? "Timer minutes" : "Timer seconds");
        snprintf(line, sizeof(line), "%02u:%02u", ctx->timer_minutes, ctx->timer_seconds);
        lcd_print_line(1, line);
        return;
    }

    lcd_print_line(0, ctx->timer_running ? "Timer running" : "Timer paused");
    snprintf(line, sizeof(line), "%02lu:%02lu",
             remaining / 60000UL, (remaining / 1000UL) % 60UL);
    lcd_print_line(1, line);
}

static void show_factory_reset() {
    lcd_print_line(0, "Factory Reset");
    lcd_print_line(1, "SELECT to reset");
}

static void show_alarm_ringing(struct context *ctx) {
    char line[17];

    snprintf(line, sizeof(line), "Wake up! %02u:%02u", ctx->alarm_hour, ctx->alarm_minute);
    lcd_print_line(0, line);
    lcd_print_line(1, "SEL snooze M off");
}

static void show_current_screen(struct context *ctx) {
    switch (ctx->current_screen) {
        case CLOCK_SCR:
            show_clock(ctx);
            break;
        case SHOW_DATE_SCR:
            show_date();
            break;
        case SHOW_ENV_SCR:
            show_env(ctx);
            break;
        case SET_TIME_SCR:
            show_set_time(ctx);
            break;
        case SET_DATE_SCR:
            show_set_date(ctx);
            break;
        case SET_ALARM_SCR:
            show_set_alarm(ctx);
            break;
        case STOPWATCH_SCR:
            show_stopwatch(ctx);
            break;
        case TIMER_SCR:
            show_timer(ctx);
            break;
        case FACTORY_RESET_SCR:
            show_factory_reset();
            break;
        case ALARM_SCR:
            show_alarm_ringing(ctx);
            break;
        default:
            ctx->current_screen = CLOCK_SCR;
            break;
    }
}

static enum screen run_screen(struct context *ctx, enum screen active_screen) {
    ctx->current_screen = active_screen;

    handle_buttons(ctx);
    read_sensor(ctx);
    update_timer(ctx);
    check_alarm(ctx);
    update_buzzer_and_led(ctx);
    show_current_screen(ctx);

    delay(40);
    return (enum screen)ctx->current_screen;
}

enum screen clock_screen(struct context *ctx) {
    return run_screen(ctx, CLOCK_SCR);
}

enum screen show_date_screen(struct context *ctx) {
    return run_screen(ctx, SHOW_DATE_SCR);
}

enum screen show_env_screen(struct context *ctx) {
    return run_screen(ctx, SHOW_ENV_SCR);
}

enum screen set_time_screen(struct context *ctx) {
    return run_screen(ctx, SET_TIME_SCR);
}

enum screen set_date_screen(struct context *ctx) {
    return run_screen(ctx, SET_DATE_SCR);
}

enum screen set_alarm_screen(struct context *ctx) {
    return run_screen(ctx, SET_ALARM_SCR);
}

enum screen stopwatch_screen(struct context *ctx) {
    return run_screen(ctx, STOPWATCH_SCR);
}

enum screen timer_screen(struct context *ctx) {
    return run_screen(ctx, TIMER_SCR);
}

enum screen factory_reset_screen(struct context *ctx) {
    return run_screen(ctx, FACTORY_RESET_SCR);
}

enum screen alarm_screen(struct context *ctx) {
    return run_screen(ctx, ALARM_SCR);
}
