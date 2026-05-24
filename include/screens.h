#ifndef SCREENS_H
#define SCREENS_H

#include "context.h"

/**
 * List of available screens / states.
 */
enum screen {
    INIT_SCR,
    CLOCK_SCR,
    SHOW_DATE_SCR,
    SHOW_ENV_SCR,
    SET_TIME_SCR,
    SET_DATE_SCR,
    SET_ALARM_SCR,
    STOPWATCH_SCR,
    TIMER_SCR,
    FACTORY_RESET_SCR,
    ALARM_SCR
};

enum screen init_screen(struct context *ctx);
enum screen clock_screen(struct context *ctx);
enum screen show_date_screen(struct context *ctx);
enum screen show_env_screen(struct context *ctx);
enum screen set_time_screen(struct context *ctx);
enum screen set_date_screen(struct context *ctx);
enum screen set_alarm_screen(struct context *ctx);
enum screen stopwatch_screen(struct context *ctx);
enum screen timer_screen(struct context *ctx);
enum screen factory_reset_screen(struct context *ctx);
enum screen alarm_screen(struct context *ctx);

#endif // SCREENS_H
