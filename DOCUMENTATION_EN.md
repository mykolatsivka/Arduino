# Alarm Clock Project Documentation

This document is written as preparation for an oral project defense. It explains not only what the project does, but also how the code works, why each file exists, and how to explain the most important functions.

## 1. Project Overview

This project is an alarm clock built with Arduino UNO. It uses a 16x2 I2C LCD display to show information, a DS1302 RTC module for date and time handling, a DHT11 sensor for temperature and humidity, four buttons for user input, a buzzer for sound output, and an LED for alarm status indication.

Main features:

- display current time;
- display current date;
- display temperature and humidity;
- set current time using buttons;
- set current date using buttons;
- set alarm time;
- save alarm settings in EEPROM;
- play alarm sound using a buzzer;
- show alarm status using an LED;
- snooze alarm for 5 minutes;
- stopwatch;
- countdown timer;
- factory reset.

The project is implemented as a state machine. This means that the program has a set of states, also called screens, and only one screen is active at a time: clock, date, environment, set time, set alarm, timer, and so on.

## 2. Hardware Components

| Component | Purpose |
|---|---|
| Arduino UNO | Main microcontroller that runs the program |
| 16x2 I2C LCD | Displays time, date, sensor data, menus, and messages |
| DS1302 RTC | Real-time clock module |
| DHT11 | Temperature and humidity sensor |
| 4 buttons | User control and menu navigation |
| Buzzer | Alarm and timer sound output |
| LED | Alarm enabled indicator and ringing indicator |
| Breadboard | Circuit assembly without soldering |

## 3. Wiring

All pin definitions are stored in `include/config.h`. If asked where hardware pins are configured, the answer is: "All hardware pin definitions are centralized in `config.h`, so they do not need to be searched across the whole codebase."

| Device | Device Pin | Arduino Pin |
|---|---:|---:|
| LCD I2C | SDA | A4 |
| LCD I2C | SCL | A5 |
| LCD I2C | VCC | 5V |
| LCD I2C | GND | GND |
| DS1302 RTC | DAT | D7 |
| DS1302 RTC | CLK | D6 |
| DS1302 RTC | RST | D8 |
| DS1302 RTC | VCC | 5V |
| DS1302 RTC | GND | GND |
| DHT11 | DATA / S | D5 |
| DHT11 | VCC / + | 5V |
| DHT11 | GND / - | GND |
| MODE button | signal | D2 |
| SELECT button | signal | D3 |
| UP button | signal | D4 |
| DOWN button | signal | D9 |
| Buzzer | signal / + | D10 |
| Buzzer | GND / - | GND |
| LED | through resistor | D11 |

The buttons use `INPUT_PULLUP`.

This means:

- button not pressed - Arduino reads `HIGH`;
- button pressed - Arduino reads `LOW`.

This is important because the code detects a button press as a transition from `HIGH` to `LOW`.

## 4. Used Libraries

The libraries are configured in `platformio.ini`.

| Library | Purpose |
|---|---|
| `makuna/RTC` | DS1302 RTC support |
| `adafruit/DHT sensor library` | DHT11 sensor support |
| `adafruit/Adafruit Unified Sensor` | Dependency for the DHT library |
| `robtillaart/I2C_LCD` | Included in the skeleton project |
| `marcoschwartz/LiquidCrystal_I2C` | Actually used for the 16x2 I2C LCD |
| `EEPROM` | Built-in Arduino library for persistent settings |
| `Wire` | I2C communication for the LCD |

## 5. Project Structure

```txt
include/
  config.h
  context.h
  lcd_wrapper.h
  rtc_wrapper.h
  screens.h
  sensors.h

src/
  main.cpp
  lcd_wrapper.cpp
  rtc_wrapper.cpp
  sensors.cpp
  screens/
    init.cpp
    app.cpp

lib/
  helpers/

platformio.ini
README
```

Main structure idea:

- `main.cpp` only runs the state machine;
- `context.h` stores the program state;
- `screens.h` declares all screens;
- `src/screens/init.cpp` initializes the system;
- `src/screens/app.cpp` contains the menu, buttons, alarm, timer, and stopwatch logic;
- `lcd_wrapper.cpp` isolates LCD operations;
- `rtc_wrapper.cpp` isolates time, RTC, and EEPROM snapshot logic;
- `sensors.cpp` isolates DHT11 operations.

## 6. Button Controls

| Button | Main Action |
|---|---|
| MODE | Move to the next screen |
| SELECT | Enter edit mode, confirm, start, pause |
| UP | Increase value or reset stopwatch |
| DOWN | Decrease value |

The exact behavior depends on the active screen.

During alarm ringing:

- `SELECT` activates snooze;
- `MODE`, `UP`, or `DOWN` stops the alarm.

On the stopwatch screen:

- `SELECT` starts or pauses the stopwatch;
- `UP` resets it.

On the timer screen:

- `UP` and `DOWN` change minutes or seconds;
- `SELECT` moves from minutes to seconds and then starts the timer;
- while running, `SELECT` pauses or resumes the timer.

## 7. Program Screens

Screens are declared in `include/screens.h` using the `screen` enum.

| Screen | Purpose |
|---|---|
| `INIT_SCR` | Initializes the program |
| `CLOCK_SCR` | Shows current time |
| `SHOW_DATE_SCR` | Shows current date |
| `SHOW_ENV_SCR` | Shows temperature and humidity |
| `SET_TIME_SCR` | Sets current time |
| `SET_DATE_SCR` | Sets current date |
| `SET_ALARM_SCR` | Sets alarm |
| `STOPWATCH_SCR` | Stopwatch |
| `TIMER_SCR` | Countdown timer |
| `FACTORY_RESET_SCR` | Resets settings |
| `ALARM_SCR` | Active alarm ringing screen |

## 8. File `include/config.h`

This file contains all pin definitions and basic configuration values.

### `BAUD_RATE`

```cpp
#define BAUD_RATE 9600
```

Serial communication speed. Serial is initialized, although the project mainly uses the LCD.

### `RTC_DAT_PIN`, `RTC_CLK_PIN`, `RTC_RST_PIN`

```cpp
#define RTC_DAT_PIN 7
#define RTC_CLK_PIN 6
#define RTC_RST_PIN 8
```

Pins for the DS1302 RTC module. DS1302 does not use I2C. It uses separate `DAT`, `CLK`, and `RST` lines.

### `BUZZER_PIN`

```cpp
#define BUZZER_PIN 10
```

Pin used by the buzzer. The code uses `tone()` to generate sound on this pin.

### `DHT_PIN`

```cpp
#define DHT_PIN 5
```

Data pin for the DHT11 sensor.

### `BTN1_PIN`, `BTN2_PIN`, `BTN3_PIN`, `BTN4_PIN`

```cpp
#define BTN1_PIN 2
#define BTN2_PIN 3
#define BTN3_PIN 4
#define BTN4_PIN 9
```

In the code these are mapped to:

- `BTN_MODE`;
- `BTN_SELECT`;
- `BTN_UP`;
- `BTN_DOWN`.

### `STATUS_LED_PIN`

```cpp
#define STATUS_LED_PIN 11
```

Pin used by the status LED. The LED is on when the alarm is enabled and blinks while the alarm or timer is ringing.

### `LCD_I2C_ADDRESS`, `LCD_ROWS`, `LCD_COLS`

```cpp
#define LCD_I2C_ADDRESS 0x27
#define LCD_ROWS 2
#define LCD_COLS 16
```

The LCD I2C address was found using an I2C scanner. The display has 2 rows and 16 columns.

## 9. File `include/context.h`

This is one of the most important files. It defines the structures that store the program state.

### `struct button_state`

```cpp
struct button_state {
    byte pin;
    bool last_state;
    unsigned long last_change;
};
```

This structure stores the state of one button.

Fields:

- `pin` - Arduino pin number;
- `last_state` - previous button state, `HIGH` or `LOW`;
- `last_change` - time of the last accepted state change, used for debounce.

Debounce is necessary because a mechanical button can produce several quick signal changes during one press. Without debounce, one physical press could be detected as multiple presses.

### `struct context`

`context` is the main program state structure. It is passed to all screen functions.

The main idea is to keep program state in one object instead of using many unrelated global variables.

Important field groups:

#### Current screen

```cpp
byte current_screen;
```

Stores the currently active screen.

#### Buttons

```cpp
button_state mode_button;
button_state select_button;
button_state up_button;
button_state down_button;
```

Stores button states for debounce and press detection.

#### Sensor values

```cpp
float temperature;
int humidity;
unsigned long last_sensor_read;
```

Stores the latest valid sensor values and the time of the last DHT11 read.

#### Time editing

```cpp
bool setting_time;
byte time_field;
byte set_hour;
byte set_minute;
```

- `setting_time` tells whether the user is editing time;
- `time_field` tells which field is selected: hour or minute;
- `set_hour`, `set_minute` are temporary values before saving.

#### Date editing

```cpp
bool setting_date;
byte date_field;
byte set_day;
byte set_month;
int set_year;
```

Similar to time editing, but for day, month, and year.

#### Alarm state

```cpp
bool setting_alarm;
byte alarm_field;
byte alarm_hour;
byte alarm_minute;
bool alarm_enabled;
bool alarm_ringing;
int last_alarm_key;
```

- `alarm_hour`, `alarm_minute` store the alarm time;
- `alarm_enabled` tells whether the alarm is active;
- `alarm_ringing` tells whether the alarm is currently ringing;
- `last_alarm_key` prevents repeated alarm starts during the same minute.

#### Snooze

```cpp
bool snooze_active;
byte snooze_hour;
byte snooze_minute;
```

Stores whether snooze is active and the next snooze time.

#### Stopwatch

```cpp
bool stopwatch_running;
unsigned long stopwatch_started_at;
unsigned long stopwatch_elapsed_before_start;
```

Used for stopwatch timing.

#### Timer

```cpp
bool timer_setting;
byte timer_field;
byte timer_minutes;
byte timer_seconds;
bool timer_running;
bool timer_done;
unsigned long timer_ends_at;
unsigned long timer_remaining_ms;
```

Stores countdown timer state, values, and timing data.

## 10. File `include/screens.h`

This file declares the screen enum and screen function prototypes.

### `enum screen`

The enum lists all states of the program. Each state represents one screen or mode.

Using an enum makes the code more readable. Instead of numbers like `0`, `1`, or `2`, the code uses names like `CLOCK_SCR`, `SET_ALARM_SCR`, and `TIMER_SCR`.

### Screen function prototypes

Example:

```cpp
enum screen clock_screen(struct context *ctx);
enum screen set_alarm_screen(struct context *ctx);
```

Each screen function:

1. receives a pointer to `context`;
2. runs the logic for that screen;
3. returns the next active screen.

## 11. File `src/main.cpp`

This file is intentionally short.

### `int main()`

```cpp
int main() {
    struct context context;
    enum screen screen = INIT_SCR;

    for (;;) {
        switch (screen) {
            ...
        }
    }
}
```

What happens:

1. A `context` structure is created.
2. Initial screen is set to `INIT_SCR`.
3. An infinite loop starts.
4. The `switch` statement calls the function for the active screen.
5. Each screen function returns the next screen.

This is the main state machine dispatcher.

Defense explanation:

"`main.cpp` does not contain business logic. It only dispatches states. All real behavior is implemented in screen functions and wrapper modules."

## 12. File `src/screens/init.cpp`

This file handles system initialization.

### `context_init(struct context *ctx)`

This function fills the `context` structure with initial values.

It:

- sets the initial screen to `CLOCK_SCR`;
- initializes all four button states;
- clears temperature and humidity values;
- disables time, date, and alarm editing modes;
- sets default alarm to `07:00 OFF`;
- disables snooze;
- resets the stopwatch;
- sets the timer to `01:00`.

This is important because uninitialized variables can contain random memory values.

### `init_screen(struct context *ctx)`

This is the first screen called from `main.cpp`.

Important operations:

```cpp
init();
context_init(ctx);
Wire.begin();
Serial.begin(BAUD_RATE);
```

`init()` is the Arduino core initialization function. It is needed because this project uses a custom `main()` instead of the usual `setup()` and `loop()`.

Pins are configured:

```cpp
pinMode(BTN_MODE, INPUT_PULLUP);
pinMode(BTN_SELECT, INPUT_PULLUP);
pinMode(BTN_UP, INPUT_PULLUP);
pinMode(BTN_DOWN, INPUT_PULLUP);
pinMode(BUZZER_PIN, OUTPUT);
pinMode(STATUS_LED_PIN, OUTPUT);
```

Then hardware modules are initialized:

```cpp
lcd_init();
lcd_backlight(true);
clock_init();
sensors_init();
load_alarm_settings(ctx);
```

Finally, the LCD shows:

```txt
Alarm Clock
Ready
```

The function returns `CLOCK_SCR`, so the program moves to the clock screen.

## 13. File `src/lcd_wrapper.cpp`

This file hides direct use of the `LiquidCrystal_I2C` library.

The LCD object is created inside the file:

```cpp
static LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);
```

The `static` keyword means this object is private to `lcd_wrapper.cpp`.

### `lcd_init()`

Initializes the LCD:

```cpp
lcd.init();
```

### `lcd_clear()`

Clears the full display.

### `lcd_set_cursor(int y, int x)`

Sets the cursor position.

The wrapper uses parameters as `row, column`, while the library call uses `column, row`.

### `lcd_print(const char* text)`

Prints text at the current cursor position.

### `lcd_print_at(int y, int x, const char* text)`

Sets the cursor and prints text.

### `lcd_print_line(int y, const char* text)`

Prints a full 16-character padded row:

```cpp
snprintf(line, sizeof(line), "%-16s", text);
```

This prevents old characters from remaining on the LCD when a shorter string replaces a longer one.

### `lcd_clear_line(int y)`

Clears one LCD row.

### `lcd_backlight(bool state)`

Turns the LCD backlight on or off.

## 14. File `src/sensors.cpp`

This file handles DHT11 sensor operations.

### `static DHT dht(DHT_PIN, DHT_TYPE)`

Creates the DHT11 sensor object.

### `sensors_init()`

Starts the DHT sensor:

```cpp
dht.begin();
```

### `get_temperature()`

Returns temperature in degrees Celsius.

If the sensor read fails, the DHT library can return `NaN`. The validation is done in `read_sensor()` inside `app.cpp`.

### `get_humidity()`

Reads humidity.

If humidity is invalid, it returns `-1`.

The app checks:

```cpp
if (!isnan(new_temperature) && new_humidity >= 0)
```

So only valid sensor readings are stored in `context`.

## 15. File `src/rtc_wrapper.cpp`

This file handles time, date, the DS1302 RTC, and EEPROM time snapshot.

### Main idea

The project uses a DS1302 RTC module, but it also stores the last set date and time in EEPROM.

When the user saves time or date:

1. the internal software clock is updated;
2. the value is saved to EEPROM;
3. the value is written to DS1302.

When Arduino starts:

1. it first tries to load the EEPROM snapshot;
2. if the snapshot is valid, it starts from that value;
3. if there is no valid snapshot, it tries the RTC;
4. if RTC is invalid, it uses a fallback default time.

This was done to reliably restore the last configured time after restart.

### EEPROM addresses

```cpp
#define EEPROM_TIME_MAGIC_ADDR 10
#define EEPROM_TIME_YEAR_ADDR 11
#define EEPROM_TIME_MONTH_ADDR 12
#define EEPROM_TIME_DAY_ADDR 13
#define EEPROM_TIME_HOUR_ADDR 14
#define EEPROM_TIME_MINUTE_ADDR 15
#define EEPROM_TIME_SECOND_ADDR 16
#define EEPROM_TIME_MAGIC_VALUE 0x5C
```

EEPROM stores:

- year;
- month;
- day;
- hour;
- minute;
- second;
- magic byte.

The magic byte is used to check whether EEPROM contains valid saved data.

### `static ThreeWire rtc_wire(...)`

DS1302 is controlled through ThreeWire:

```cpp
static ThreeWire rtc_wire(RTC_DAT_PIN, RTC_CLK_PIN, RTC_RST_PIN);
```

Parameter order:

```txt
DAT, CLK, RST
```

### `static RtcDS1302<ThreeWire> rtc(rtc_wire)`

Creates the RTC object.

### `datetime_is_valid(const RtcDateTime &date_time)`

Checks whether date and time are valid.

It checks:

- year from 2024 to 2099;
- month from 1 to 12;
- day from 1 to 31;
- hour from 0 to 23;
- minute from 0 to 59;
- second from 0 to 59;
- internal `date_time.IsValid()`.

This prevents invalid RTC data from breaking the program.

### `fallback_datetime()`

Returns the default date and time:

```txt
22.05.2026 00:00:00
```

This is used when neither EEPROM nor RTC contains valid data.

### `set_app_clock(const RtcDateTime &date_time)`

Starts the internal software clock.

It stores:

- base time;
- the `millis()` value when the base time was set;
- a validity flag.

Then `safe_now()` can calculate current time using `millis()`.

### `save_clock_snapshot(const RtcDateTime &date_time)`

Saves date and time to EEPROM.

It uses `EEPROM.update()` instead of `EEPROM.write()`.

Difference:

- `write()` always writes;
- `update()` writes only if the value changed.

This is better because EEPROM has a limited number of write cycles.

### `load_clock_snapshot(RtcDateTime &date_time)`

Loads date and time from EEPROM.

Algorithm:

1. Check magic byte.
2. If magic byte is wrong, return `false`.
3. Read year, month, day, hour, minute, and second.
4. Build `RtcDateTime`.
5. Validate it.
6. If valid, write result into the output parameter and return `true`.

### `safe_now()`

This is the central safe time getter.

Algorithm:

1. If internal software clock is valid, return:

```cpp
app_clock_base + elapsed_seconds
```

2. Otherwise, read RTC.
3. If RTC data is valid, use RTC.
4. If RTC is invalid, use fallback time.

### `to_dt(const RtcDateTime &date_time)`

Converts the library type `RtcDateTime` into the project type `dt`.

This makes the rest of the program independent from the RTC library type.

### `clock_init()`

Initializes RTC and initial time.

Algorithm:

1. Start RTC using `rtc.Begin()`.
2. Disable write protection.
3. Start RTC.
4. Try to load time from EEPROM.
5. If EEPROM is valid, use it.
6. Otherwise, try RTC.
7. If RTC is invalid, use fallback time.

### `set_date(...)`

Changes only the date and keeps the current time.

### `set_time(...)`

Changes only the time and keeps the current date.

### `set_datetime(...)`

Main function for saving date and time.

It:

1. creates `RtcDateTime`;
2. updates the software clock;
3. saves EEPROM snapshot;
4. writes time to RTC;
5. starts RTC.

### `get_day()`, `get_month()`, `get_year()`

Return separate date parts.

### `get_hours()`, `get_minutes()`, `get_seconds()`

Return separate time parts.

### `now()`

Returns the full current date and time as `struct dt`.

This function is used by screens, alarm logic, date logic, time logic, and snooze.

### `reset_saved_datetime()`

Clears the EEPROM magic byte for the saved time snapshot.

It is used by Factory Reset.

## 16. File `src/screens/app.cpp`

This is the largest file. It contains the main application behavior.

### Alarm EEPROM addresses

```cpp
#define EEPROM_MAGIC_ADDR 0
#define EEPROM_ALARM_HOUR_ADDR 1
#define EEPROM_ALARM_MINUTE_ADDR 2
#define EEPROM_ALARM_ENABLED_ADDR 3
#define EEPROM_MAGIC_VALUE 0xA6
```

EEPROM stores:

- alarm hour;
- alarm minute;
- alarm enabled flag;
- magic byte.

### `days_in_month(byte month, int year)`

Returns the number of days in a month.

It handles:

- February;
- leap years;
- 30-day months;
- 31-day months.

This prevents invalid dates like April 31.

### `read_pressed(struct button_state *button)`

Reads a button with debounce.

Algorithm:

1. Read current pin state.
2. If the state changed and more than 60 ms passed, accept the change.
3. If the new state is `LOW`, this is a button press.
4. Return `true` only once per press.

### `save_alarm_settings(struct context *ctx)`

Saves alarm settings to EEPROM:

- magic byte;
- alarm hour;
- alarm minute;
- enabled state.

### `load_alarm_settings(struct context *ctx)`

Loads alarm settings from EEPROM.

If magic byte is missing, it sets default values:

```txt
07:00 OFF
```

It also validates hour and minute.

### `next_screen(struct context *ctx)`

Moves to the next screen.

If the current screen is the alarm screen or the last menu screen, it returns to `CLOCK_SCR`.

It also exits edit modes:

```cpp
ctx->setting_time = false;
ctx->setting_date = false;
ctx->setting_alarm = false;
```

### `read_sensor(struct context *ctx)`

Reads DHT11 every 2 seconds.

This is needed because DHT11 should not be read too frequently.

If values are valid, it stores them in `context`.

### `start_set_time(struct context *ctx)`

Starts time editing.

It reads current time using `now()` and copies hour and minute into temporary fields.

### `save_set_time(struct context *ctx)`

Saves edited time.

It keeps the current date and saves the new hour and minute through `set_datetime()`.

Then it shows:

```txt
Time saved
```

### `change_set_time(struct context *ctx, int direction)`

Changes hour or minute.

`direction` is:

- `1` for UP;
- `-1` for DOWN.

Hours wrap between 0 and 23. Minutes wrap between 0 and 59.

### `start_set_date(struct context *ctx)`

Starts date editing by copying current date into temporary fields.

### `save_set_date(struct context *ctx)`

Saves edited date while keeping the current time.

### `change_set_date(struct context *ctx, int direction)`

Changes day, month, or year.

It validates the day when month or year changes.

### `start_set_alarm(struct context *ctx)`

Starts alarm editing and selects the hour field.

### `save_set_alarm(struct context *ctx)`

Saves alarm settings to EEPROM and shows:

```txt
Alarm saved
```

### `change_set_alarm(struct context *ctx, int direction)`

Changes:

- alarm hour;
- alarm minute;
- alarm ON/OFF.

### `start_alarm_ring(struct context *ctx)`

Starts alarm ringing state.

It:

- sets `alarm_ringing = true`;
- switches screen to `ALARM_SCR`;
- clears LCD.

The buzzer is handled separately in `update_buzzer_and_led()`.

### `stop_alarm_ring(struct context *ctx)`

Stops the alarm sound and returns to the clock screen.

### `snooze_alarm(struct context *ctx)`

Postpones the alarm by 5 minutes.

It:

1. reads current time;
2. adds 5 minutes;
3. stores snooze hour and minute;
4. enables snooze;
5. stops current alarm sound;
6. returns to clock screen.

### `check_alarm(struct context *ctx)`

Checks whether the alarm should start.

It:

1. ignores the check if alarm is already ringing;
2. reads current time;
3. checks snooze time;
4. checks regular alarm time if alarm is enabled;
5. starts alarm if hour and minute match.

`last_alarm_key` prevents repeated alarm starts in the same minute.

### `stopwatch_elapsed(struct context *ctx)`

Calculates elapsed stopwatch time.

If paused, it returns the saved elapsed time. If running, it adds time passed since the last start.

### `toggle_stopwatch(struct context *ctx)`

Starts or pauses the stopwatch.

### `reset_stopwatch(struct context *ctx)`

Resets stopwatch to zero.

### `start_timer(struct context *ctx)`

Starts the countdown timer.

It calculates duration:

```cpp
duration = (minutes * 60 + seconds) * 1000
```

Then it stores:

```cpp
timer_ends_at = millis() + timer_remaining_ms;
```

### `pause_timer(struct context *ctx)`

Pauses the timer and stores remaining milliseconds.

### `resume_timer(struct context *ctx)`

Resumes the paused timer.

### `reset_timer(struct context *ctx)`

Resets the timer into setting mode and turns off the buzzer.

### `change_timer_value(struct context *ctx, int direction)`

Changes timer minutes or seconds.

Minutes wrap from 0 to 99. Seconds wrap from 0 to 59.

### `update_timer(struct context *ctx)`

Checks if the timer finished.

If finished:

- `timer_running = false`;
- `timer_done = true`;
- remaining time becomes 0;
- screen switches to `TIMER_SCR`;
- LCD is cleared.

### `factory_reset(struct context *ctx)`

Resets:

- alarm to `07:00 OFF`;
- snooze;
- saved time snapshot;
- stopwatch;
- timer to `01:00`.

It then shows:

```txt
Factory reset
Done
```

### `handle_buttons(struct context *ctx)`

Main button handling function.

It:

1. reads all four buttons;
2. handles alarm ringing controls;
3. handles timer-done controls;
4. handles MODE screen switching;
5. handles SELECT actions depending on active screen;
6. handles UP and DOWN depending on active screen.

This is one of the central functions of the program.

### `update_buzzer_and_led(struct context *ctx)`

Controls buzzer and LED.

If alarm or timer is ringing:

- buzzer toggles every 250 ms;
- LED blinks together with the buzzer.

If nothing is ringing:

- buzzer is off;
- LED is on only when alarm is enabled.

### `show_clock(struct context *ctx)`

Shows current time.

First row:

- `Time` if alarm is disabled;
- `Time        AL` if alarm is enabled.

Second row:

- time in `HH:MM:SS` format;
- `SNZ` if snooze is active.

### `show_date()`

Shows date in:

```txt
DD.MM.YYYY
```

### `show_env(struct context *ctx)`

Shows temperature and humidity.

Temperature is rounded to integer value.

### `show_set_time(struct context *ctx)`

Shows the time setting screen.

Before editing:

```txt
Set Time
SELECT to edit
```

During editing:

- `Edit hour`;
- or `Edit minute`.

### `show_set_date(struct context *ctx)`

Shows date setting screen for day, month, and year.

### `show_set_alarm(struct context *ctx)`

Shows alarm setting screen.

Format:

```txt
HH:MM ON
```

or

```txt
HH:MM OFF
```

### `show_stopwatch(struct context *ctx)`

Shows stopwatch in:

```txt
MM:SS.HH
```

where `HH` means hundredths of a second.

### `show_timer(struct context *ctx)`

Shows timer state:

- setting minutes;
- setting seconds;
- running;
- paused;
- finished.

### `show_factory_reset()`

Shows:

```txt
Factory Reset
SELECT to reset
```

### `show_alarm_ringing(struct context *ctx)`

Shows:

```txt
Wake up! HH:MM
SEL snooze M off
```

Meaning:

- `SELECT` - snooze;
- `MODE` - turn alarm off.

### `show_current_screen(struct context *ctx)`

Calls the correct display function depending on `current_screen`.

### `run_screen(struct context *ctx, enum screen active_screen)`

Main common logic for all screens.

It:

1. sets active screen;
2. handles buttons;
3. reads sensor;
4. updates timer;
5. checks alarm;
6. updates buzzer and LED;
7. redraws the current screen;
8. delays for 40 ms;
9. returns current screen.

This avoids duplicating the same logic in each screen function.

### Individual screen functions

Functions like:

```cpp
enum screen clock_screen(struct context *ctx)
```

simply call `run_screen()` with their own screen value.

This matches the expected project skeleton structure.

## 17. Full Program Flow

1. `main()` starts with `INIT_SCR`.
2. `init_screen()` initializes Arduino, LCD, RTC, DHT11, buttons, buzzer, LED, and EEPROM settings.
3. Program moves to `CLOCK_SCR`.
4. The active screen function is called repeatedly.
5. Screen function calls `run_screen()`.
6. `run_screen()` handles buttons, sensor, alarm, timer, LED, buzzer, and LCD.
7. If MODE is pressed, screen changes.
8. If alarm time matches current time, program switches to `ALARM_SCR`.

## 18. Time Setting Flow

1. User goes to `Set Time`.
2. User presses `SELECT`.
3. Hour editing starts.
4. `UP/DOWN` changes hour.
5. `SELECT` moves to minute editing.
6. `UP/DOWN` changes minutes.
7. `SELECT` saves time.

Saving is performed through:

```cpp
set_datetime(...)
```

This stores time in the software clock, EEPROM snapshot, and RTC.

## 19. Alarm Setting Flow

1. User goes to `Set Alarm`.
2. `SELECT` starts hour editing.
3. `UP/DOWN` changes hour.
4. `SELECT` moves to minute editing.
5. `UP/DOWN` changes minute.
6. `SELECT` moves to ON/OFF.
7. `UP/DOWN` toggles ON/OFF.
8. `SELECT` saves alarm.

Alarm data is stored in EEPROM.

## 20. Alarm Logic

`check_alarm()` checks current time continuously.

If:

```txt
current.hours == alarm_hour
current.minutes == alarm_minute
alarm_enabled == true
```

then `start_alarm_ring()` is called.

After that:

- screen becomes `ALARM_SCR`;
- `update_buzzer_and_led()` starts the buzzer;
- LED blinks.

## 21. Snooze Logic

When alarm is ringing, pressing `SELECT` calls `snooze_alarm()`.

The function adds 5 minutes to current time and stores it in:

```cpp
snooze_hour
snooze_minute
```

Then the alarm stops.

When snooze time arrives, `check_alarm()` starts the alarm again.

## 22. Stopwatch Logic

Stopwatch uses `millis()`, not RTC.

When started, it stores:

```cpp
stopwatch_started_at = millis();
```

When paused, elapsed time is accumulated.

When reset, elapsed time becomes zero.

## 23. Timer Logic

Timer also uses `millis()`.

When started:

```cpp
timer_ends_at = millis() + timer_remaining_ms;
```

`update_timer()` checks whether current `millis()` reached `timer_ends_at`.

If yes:

- timer finishes;
- `timer_done = true`;
- buzzer starts.

## 24. EEPROM Explanation

EEPROM is used for two things:

1. Saving alarm settings.
2. Saving the last configured date and time.

The code uses `EEPROM.update()` to reduce unnecessary EEPROM writes.

Magic bytes are used to check whether saved data is valid.

## 25. RTC Explanation

The project uses DS1302 RTC.

It is connected using:

```txt
DAT
CLK
RST
```

not I2C.

RTC is initialized in `clock_init()`.

When time is set, `set_datetime()` writes it to RTC using:

```cpp
rtc.SetDateTime(updated);
```

The code also validates RTC data to avoid invalid values.

## 26. State Machine Explanation

The program is implemented as a state machine.

The list of states is declared in `screens.h`.

`main.cpp` contains an infinite loop and a switch statement that calls the active state function.

Each screen function returns the next state.

Advantages:

- easy to add new screens;
- easier to read;
- each mode is separated;
- avoids one huge `loop()` function.

## 27. Additional Features

The project includes the following additional features:

### Snooze

Postpones the alarm for 5 minutes.

### Stopwatch

Stopwatch with start, pause, and reset.

### Countdown Timer

Timer that rings when it reaches zero.

### Factory Reset

Resets alarm, timer, stopwatch, and saved time.

If exactly three additional features are required, use:

1. Snooze.
2. Stopwatch.
3. Countdown timer.

Factory reset can be mentioned as an extra helper feature.

## 28. Possible Defense Questions

### Why use `INPUT_PULLUP`?

Because Arduino has internal pull-up resistors. This avoids external pull-down resistors. A released button reads `HIGH`, and a pressed button reads `LOW`.

### Why is debounce needed?

Mechanical buttons can bounce electrically. Debounce prevents one physical press from being detected multiple times.

### Why use `context`?

It stores the whole program state in one structure and passes it between screen functions.

### Why is `main.cpp` short?

Because the project is modular. `main.cpp` only dispatches states, while logic is implemented in separate modules.

### Why use an LCD wrapper?

So screen logic does not depend directly on the LCD library. If the LCD library changes, only the wrapper needs to be updated.

### Why is DHT11 not read every loop?

DHT11 should not be read too frequently. The program reads it about every 2 seconds.

### How does the alarm avoid restarting many times in the same minute?

It uses `last_alarm_key`, which stores the day and minute of the last alarm trigger.

### How does the LED work?

If alarm is enabled, LED is on. If alarm or timer is ringing, LED blinks together with buzzer.

### Why use `millis()` instead of `delay()` for stopwatch and timer?

`millis()` allows non-blocking time measurement. The program can still read buttons and update the display.

## 29. Demonstration Checklist

Before defense, test:

- LCD shows startup message;
- `MODE` cycles through all screens;
- `Time` shows time;
- `Date` shows date;
- `Temp/Humidity` shows DHT11 values;
- `Set Time` changes hour and minute;
- `Set Date` changes day, month, and year;
- after restart, last configured time is restored;
- `Set Alarm` sets alarm time;
- LED turns on when alarm is enabled;
- alarm rings at configured time;
- `SELECT` during ringing activates snooze;
- `MODE` during ringing stops the alarm;
- stopwatch starts, pauses, and resets;
- timer can be configured, started, paused, and rings at zero;
- factory reset resets settings.

## 30. One-Minute Project Explanation

This is an Arduino UNO alarm clock. It uses an LCD display, DS1302 RTC, DHT11 sensor, buttons, buzzer, and LED. The program is implemented as a state machine where every mode is a separate screen. `main.cpp` only dispatches screens, while the main logic is in `screens/app.cpp`. Program state is stored in the `context` structure. Time and date are handled by `rtc_wrapper`, LCD output by `lcd_wrapper`, and DHT11 by `sensors`. The user can set time, date, and alarm using buttons. Alarm settings are stored in EEPROM, and when the alarm rings, the buzzer sounds and the LED blinks. Additional features are snooze, stopwatch, and countdown timer.
