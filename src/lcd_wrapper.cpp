#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "config.h"
#include "lcd_wrapper.h"

static LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

void lcd_init() {
    lcd.init();
}

void lcd_clear() {
    lcd.clear();
}

void lcd_set_cursor(int y, int x) {
    lcd.setCursor(x, y);
}

void lcd_print(const char* text) {
    lcd.print(text);
}

void lcd_print_at(int y, int x, const char* text) {
    lcd_set_cursor(y, x);
    lcd_print(text);
}

void lcd_print_line(int y, const char* text) {
    char line[17];
    snprintf(line, sizeof(line), "%-16s", text);
    lcd_set_cursor(y, 0);
    lcd_print(line);
}

void lcd_clear_line(int y) {
    lcd_print_line(y, "");
}

void lcd_backlight(bool state) {
    if (state) {
        lcd.backlight();
    } else {
        lcd.noBacklight();
    }
}
