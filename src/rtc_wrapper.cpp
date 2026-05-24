#include <Arduino.h>
#include <EEPROM.h>
#include <RtcDS1302.h>
#include <ThreeWire.h>

#include "config.h"
#include "rtc_wrapper.h"

#define EEPROM_TIME_MAGIC_ADDR 10
#define EEPROM_TIME_YEAR_ADDR 11
#define EEPROM_TIME_MONTH_ADDR 12
#define EEPROM_TIME_DAY_ADDR 13
#define EEPROM_TIME_HOUR_ADDR 14
#define EEPROM_TIME_MINUTE_ADDR 15
#define EEPROM_TIME_SECOND_ADDR 16
#define EEPROM_TIME_MAGIC_VALUE 0x5C

#define DEFAULT_YEAR 2026
#define DEFAULT_MONTH 5
#define DEFAULT_DAY 22
#define DEFAULT_HOUR 0
#define DEFAULT_MINUTE 0
#define DEFAULT_SECOND 0

static ThreeWire rtc_wire(RTC_DAT_PIN, RTC_CLK_PIN, RTC_RST_PIN);
static RtcDS1302<ThreeWire> rtc(rtc_wire);

static bool app_clock_valid = false;
static RtcDateTime app_clock_base;
static unsigned long app_clock_set_at = 0;

static bool datetime_is_valid(const RtcDateTime &date_time) {
    if (!date_time.IsValid()) return false;
    if (date_time.Year() < 2024 || date_time.Year() > 2099) return false;
    if (date_time.Month() < 1 || date_time.Month() > 12) return false;
    if (date_time.Day() < 1 || date_time.Day() > 31) return false;
    if (date_time.Hour() > 23) return false;
    if (date_time.Minute() > 59) return false;
    if (date_time.Second() > 59) return false;

    return true;
}

static RtcDateTime fallback_datetime() {
    return RtcDateTime(DEFAULT_YEAR, DEFAULT_MONTH, DEFAULT_DAY,
                       DEFAULT_HOUR, DEFAULT_MINUTE, DEFAULT_SECOND);
}

static void set_app_clock(const RtcDateTime &date_time) {
    app_clock_base = date_time;
    app_clock_set_at = millis();
    app_clock_valid = true;
}

static void save_clock_snapshot(const RtcDateTime &date_time) {
    if (!datetime_is_valid(date_time)) return;

    EEPROM.update(EEPROM_TIME_YEAR_ADDR, date_time.Year() - 2000);
    EEPROM.update(EEPROM_TIME_MONTH_ADDR, date_time.Month());
    EEPROM.update(EEPROM_TIME_DAY_ADDR, date_time.Day());
    EEPROM.update(EEPROM_TIME_HOUR_ADDR, date_time.Hour());
    EEPROM.update(EEPROM_TIME_MINUTE_ADDR, date_time.Minute());
    EEPROM.update(EEPROM_TIME_SECOND_ADDR, date_time.Second());
    EEPROM.update(EEPROM_TIME_MAGIC_ADDR, EEPROM_TIME_MAGIC_VALUE);
}

static bool load_clock_snapshot(RtcDateTime &date_time) {
    if (EEPROM.read(EEPROM_TIME_MAGIC_ADDR) != EEPROM_TIME_MAGIC_VALUE) {
        return false;
    }

    int year = EEPROM.read(EEPROM_TIME_YEAR_ADDR) + 2000;
    byte month = EEPROM.read(EEPROM_TIME_MONTH_ADDR);
    byte day = EEPROM.read(EEPROM_TIME_DAY_ADDR);
    byte hour = EEPROM.read(EEPROM_TIME_HOUR_ADDR);
    byte minute = EEPROM.read(EEPROM_TIME_MINUTE_ADDR);
    byte second = EEPROM.read(EEPROM_TIME_SECOND_ADDR);

    RtcDateTime saved(year, month, day, hour, minute, second);
    if (!datetime_is_valid(saved)) {
        return false;
    }

    date_time = saved;
    return true;
}

static RtcDateTime safe_now() {
    if (app_clock_valid) {
        unsigned long elapsed_seconds = (millis() - app_clock_set_at) / 1000UL;
        return app_clock_base + (int32_t)elapsed_seconds;
    }

    RtcDateTime current = rtc.GetDateTime();

    if (datetime_is_valid(current)) {
        set_app_clock(current);
        return current;
    }

    RtcDateTime fallback = fallback_datetime();
    set_app_clock(fallback);
    return fallback;
}

static struct dt to_dt(const RtcDateTime &date_time) {
    struct dt result;

    result.day = date_time.Day();
    result.month = date_time.Month();
    result.year = date_time.Year();
    result.hours = date_time.Hour();
    result.minutes = date_time.Minute();
    result.seconds = date_time.Second();

    return result;
}

void clock_init() {
    rtc.Begin();
    rtc.SetIsWriteProtected(false);
    rtc.SetIsRunning(true);

    RtcDateTime saved_clock;
    if (load_clock_snapshot(saved_clock)) {
        set_app_clock(saved_clock);
        return;
    }

    RtcDateTime current = rtc.GetDateTime();
    if (datetime_is_valid(current)) {
        set_app_clock(current);
        save_clock_snapshot(current);
        return;
    }

    RtcDateTime fallback = fallback_datetime();
    rtc.SetDateTime(fallback);
    set_app_clock(fallback);
    save_clock_snapshot(fallback);
}

void set_date(const byte day, const byte month, const int year) {
    struct dt current = now();
    set_datetime(day, month, year, current.hours, current.minutes, current.seconds);
}

void set_time(const byte hours, const byte minutes, const byte seconds) {
    struct dt current = now();
    set_datetime(current.day, current.month, current.year, hours, minutes, seconds);
}

void set_datetime(const byte day, const byte month, const int year, const byte hours, const byte minutes, const byte seconds) {
    RtcDateTime updated(year, month, day, hours, minutes, seconds);

    set_app_clock(updated);
    save_clock_snapshot(updated);

    rtc.SetIsWriteProtected(false);
    rtc.SetIsRunning(true);
    rtc.SetDateTime(updated);
    rtc.SetIsRunning(true);
}

byte get_day() {
    return now().day;
}

byte get_month() {
    return now().month;
}

int get_year() {
    return now().year;
}

byte get_hours() {
    return now().hours;
}

byte get_minutes() {
    return now().minutes;
}

byte get_seconds() {
    return now().seconds;
}

struct dt now() {
    return to_dt(safe_now());
}

void reset_saved_datetime() {
    EEPROM.update(EEPROM_TIME_MAGIC_ADDR, 0);
}
