#ifndef CONTEXT_H
#define CONTEXT_H

#include <Arduino.h>

struct button_state {
    byte pin;
    bool last_state;
    unsigned long last_change;
};

struct context {
    byte current_screen;

    button_state mode_button;
    button_state select_button;
    button_state up_button;
    button_state down_button;

    float temperature;
    int humidity;
    unsigned long last_sensor_read;

    bool setting_time;
    byte time_field;
    byte set_hour;
    byte set_minute;

    bool setting_date;
    byte date_field;
    byte set_day;
    byte set_month;
    int set_year;

    bool setting_alarm;
    byte alarm_field;
    byte alarm_hour;
    byte alarm_minute;
    bool alarm_enabled;
    bool alarm_ringing;
    int last_alarm_key;

    bool snooze_active;
    byte snooze_hour;
    byte snooze_minute;

    bool stopwatch_running;
    unsigned long stopwatch_started_at;
    unsigned long stopwatch_elapsed_before_start;

    bool timer_setting;
    byte timer_field;
    byte timer_minutes;
    byte timer_seconds;
    bool timer_running;
    bool timer_done;
    unsigned long timer_ends_at;
    unsigned long timer_remaining_ms;
};

#endif // CONTEXT_H
