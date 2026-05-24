# Dokumentacia projektu Alarm Clock

Tento dokument sluzi ako priprava na ustnu obhajobu projektu. Nevysvetluje iba to, co projekt robi, ale aj ako funguje kod, preco existuju jednotlive subory a ako sa daju vysvetlit najdolezitejsie funkcie.

## 1. Strucny opis projektu

Projekt je budik vytvoreny na Arduino UNO. Pouziva 16x2 I2C LCD displej na zobrazovanie informacii, RTC modul DS1302 na pracu s datumom a casom, senzor DHT11 na teplotu a vlhkost, styri tlacidla na ovladanie, buzzer na zvukovy signal a LED diodu na indikaciu aktivneho budika.

Hlavne funkcie:

- zobrazenie aktualneho casu;
- zobrazenie aktualneho datumu;
- zobrazenie teploty a vlhkosti;
- nastavenie casu pomocou tlacidiel;
- nastavenie datumu pomocou tlacidiel;
- nastavenie budika;
- ulozenie nastaveni budika do EEPROM;
- zvukovy signal budika cez buzzer;
- LED indikacia aktivneho budika;
- snooze, teda odlozenie budika o 5 minut;
- stopky;
- odpocitavaci casovac;
- factory reset.

Projekt je implementovany ako stavovy automat. To znamena, ze program ma viacero stavov, teda obrazoviek, a v kazdom okamihu je aktivna iba jedna obrazovka: hodiny, datum, senzorove udaje, nastavenie casu, nastavenie budika, casovac a podobne.

## 2. Pouzite komponenty

| Komponent | Ucel |
|---|---|
| Arduino UNO | Hlavny mikrokontroler, na ktorom bezi program |
| LCD 16x2 I2C | Zobrazenie casu, datumu, senzorov, menu a sprav |
| DS1302 RTC | Modul realneho casu |
| DHT11 | Senzor teploty a vlhkosti |
| 4 tlacidla | Ovladanie menu a nastaveni |
| Buzzer | Zvukovy vystup pre budik a casovac |
| LED | Indikacia zapnuteho budika a zvonenia |
| Breadboard | Zapojenie obvodu bez pajkovania |

## 3. Zapojenie

Vsetky piny su definovane v subore `include/config.h`. Ak sa niekto spyta, kde su nastavene hardverove piny, odpoved je: "Vsetky hardverove piny su centralizovane v `config.h`, aby ich nebolo potrebne hladat v celom kode."

| Zariadenie | Pin zariadenia | Pin Arduino |
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
| MODE tlacidlo | signal | D2 |
| SELECT tlacidlo | signal | D3 |
| UP tlacidlo | signal | D4 |
| DOWN tlacidlo | signal | D9 |
| Buzzer | signal / + | D10 |
| Buzzer | GND / - | GND |
| LED | cez rezistor | D11 |

Tlacidla pouzivaju `INPUT_PULLUP`.

To znamena:

- tlacidlo nie je stlacene - Arduino cita `HIGH`;
- tlacidlo je stlacene - Arduino cita `LOW`.

Je to dolezite, pretoze v kode je stlacenie tlacidla detegovane ako prechod zo stavu `HIGH` do stavu `LOW`.

## 4. Pouzite kniznice

Kniznice su nastavene v subore `platformio.ini`.

| Kniznica | Ucel |
|---|---|
| `makuna/RTC` | Podpora RTC modulu DS1302 |
| `adafruit/DHT sensor library` | Podpora senzora DHT11 |
| `adafruit/Adafruit Unified Sensor` | Zavislost pre DHT kniznicu |
| `robtillaart/I2C_LCD` | Kniznica zo skeleton projektu |
| `marcoschwartz/LiquidCrystal_I2C` | Realne pouzita kniznica pre 16x2 I2C LCD |
| `EEPROM` | Vstavana Arduino kniznica pre trvale ulozenie nastaveni |
| `Wire` | I2C komunikacia pre LCD |

## 5. Struktura projektu

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

Hlavna myslienka struktury:

- `main.cpp` iba spusta stavovy automat;
- `context.h` uchovava stav programu;
- `screens.h` deklaruje vsetky obrazovky;
- `src/screens/init.cpp` inicializuje system;
- `src/screens/app.cpp` obsahuje logiku menu, tlacidiel, budika, casovaca a stopiek;
- `lcd_wrapper.cpp` izoluje pracu s LCD displejom;
- `rtc_wrapper.cpp` izoluje pracu s casom, RTC a EEPROM snapshotom;
- `sensors.cpp` izoluje pracu so senzorom DHT11.

## 6. Ovladanie tlacidlami

| Tlacidlo | Hlavna akcia |
|---|---|
| MODE | Prechod na dalsiu obrazovku |
| SELECT | Vstup do editacie, potvrdenie, start, pauza |
| UP | Zvysenie hodnoty alebo reset stopiek |
| DOWN | Znizenie hodnoty |

Presne spravanie zavisi od aktivnej obrazovky.

Pocas zvonenia budika:

- `SELECT` aktivuje snooze;
- `MODE`, `UP` alebo `DOWN` vypne budik.

Na obrazovke stopiek:

- `SELECT` spusti alebo pozastavi stopky;
- `UP` resetuje stopky.

Na obrazovke casovaca:

- `UP` a `DOWN` menia minuty alebo sekundy;
- `SELECT` prechadza z minut na sekundy a potom spusti casovac;
- pocas behu `SELECT` pozastavi alebo obnovi casovac.

## 7. Obrazovky programu

Obrazovky su deklarovane v `include/screens.h` pomocou enumu `screen`.

| Obrazovka | Ucel |
|---|---|
| `INIT_SCR` | Inicializacia programu |
| `CLOCK_SCR` | Zobrazenie aktualneho casu |
| `SHOW_DATE_SCR` | Zobrazenie aktualneho datumu |
| `SHOW_ENV_SCR` | Zobrazenie teploty a vlhkosti |
| `SET_TIME_SCR` | Nastavenie casu |
| `SET_DATE_SCR` | Nastavenie datumu |
| `SET_ALARM_SCR` | Nastavenie budika |
| `STOPWATCH_SCR` | Stopky |
| `TIMER_SCR` | Odpocitavaci casovac |
| `FACTORY_RESET_SCR` | Reset nastaveni |
| `ALARM_SCR` | Obrazovka aktivneho zvonenia |

## 8. Subor `include/config.h`

Tento subor obsahuje definicie pinov a zakladne konfiguracne hodnoty.

### `BAUD_RATE`

```cpp
#define BAUD_RATE 9600
```

Rychlost seriovej komunikacie. Serial sa inicializuje, aj ked projekt pouziva hlavne LCD displej.

### `RTC_DAT_PIN`, `RTC_CLK_PIN`, `RTC_RST_PIN`

```cpp
#define RTC_DAT_PIN 7
#define RTC_CLK_PIN 6
#define RTC_RST_PIN 8
```

Piny pre DS1302 RTC modul. DS1302 nepouziva I2C. Pouziva samostatne linky `DAT`, `CLK` a `RST`.

### `BUZZER_PIN`

```cpp
#define BUZZER_PIN 10
```

Pin pre buzzer. Program pouziva funkciu `tone()` na generovanie zvuku.

### `DHT_PIN`

```cpp
#define DHT_PIN 5
```

Datovy pin pre DHT11.

### `BTN1_PIN`, `BTN2_PIN`, `BTN3_PIN`, `BTN4_PIN`

```cpp
#define BTN1_PIN 2
#define BTN2_PIN 3
#define BTN3_PIN 4
#define BTN4_PIN 9
```

V kode su tieto piny premenovane na:

- `BTN_MODE`;
- `BTN_SELECT`;
- `BTN_UP`;
- `BTN_DOWN`.

### `STATUS_LED_PIN`

```cpp
#define STATUS_LED_PIN 11
```

Pin pre LED diodu. LED svieti, ked je budik zapnuty, a blika pocas zvonenia budika alebo casovaca.

### `LCD_I2C_ADDRESS`, `LCD_ROWS`, `LCD_COLS`

```cpp
#define LCD_I2C_ADDRESS 0x27
#define LCD_ROWS 2
#define LCD_COLS 16
```

I2C adresa LCD bola najdena pomocou I2C scanneru. Displej ma 2 riadky a 16 stlpcov.

## 9. Subor `include/context.h`

Toto je jeden z najdolezitejsich suborov. Definuje struktury, ktore uchovavaju stav programu.

### `struct button_state`

```cpp
struct button_state {
    byte pin;
    bool last_state;
    unsigned long last_change;
};
```

Tato struktura uchovava stav jedneho tlacidla.

Polia:

- `pin` - cislo Arduino pinu;
- `last_state` - predchadzajuci stav tlacidla, `HIGH` alebo `LOW`;
- `last_change` - cas poslednej akceptovanej zmeny, pouziva sa pre debounce.

Debounce je potrebny, pretoze mechanicke tlacidlo moze pri jednom stlaceni vytvorit viac rychlych zmien signalu.

### `struct context`

`context` je hlavna struktura stavu programu. Odovzdava sa do vsetkych funkcii obrazoviek.

Hlavna myslienka je uchovavat stav programu v jednom objekte namiesto mnohych nesuvisiacich globalnych premennych.

Dolezite skupiny poli:

#### Aktualna obrazovka

```cpp
byte current_screen;
```

Uchovava aktualne aktivnu obrazovku.

#### Tlacidla

```cpp
button_state mode_button;
button_state select_button;
button_state up_button;
button_state down_button;
```

Uchovavaju stav tlacidiel pre debounce a detekciu stlacenia.

#### Hodnoty zo senzora

```cpp
float temperature;
int humidity;
unsigned long last_sensor_read;
```

Uchovavaju posledne platne hodnoty teploty, vlhkosti a cas posledneho citania DHT11.

#### Editacia casu

```cpp
bool setting_time;
byte time_field;
byte set_hour;
byte set_minute;
```

- `setting_time` hovori, ci pouzivatel prave upravuje cas;
- `time_field` hovori, ktore pole je vybrane: hodina alebo minuta;
- `set_hour`, `set_minute` su docasne hodnoty pred ulozenim.

#### Editacia datumu

```cpp
bool setting_date;
byte date_field;
byte set_day;
byte set_month;
int set_year;
```

Podobne ako pri case, ale pre den, mesiac a rok.

#### Stav budika

```cpp
bool setting_alarm;
byte alarm_field;
byte alarm_hour;
byte alarm_minute;
bool alarm_enabled;
bool alarm_ringing;
int last_alarm_key;
```

- `alarm_hour`, `alarm_minute` uchovavaju cas budika;
- `alarm_enabled` hovori, ci je budik zapnuty;
- `alarm_ringing` hovori, ci budik prave zvoni;
- `last_alarm_key` zabranuje opakovanemu spusteniu budika v tej istej minute.

#### Snooze

```cpp
bool snooze_active;
byte snooze_hour;
byte snooze_minute;
```

Uchovava, ci je snooze aktivny a kedy ma znovu spustit budik.

#### Stopky

```cpp
bool stopwatch_running;
unsigned long stopwatch_started_at;
unsigned long stopwatch_elapsed_before_start;
```

Pouziva sa na meranie casu stopiek.

#### Casovac

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

Uchovava stav odpocitavacieho casovaca, jeho hodnoty a casove udaje.

## 10. Subor `include/screens.h`

Tento subor deklaruje enum obrazoviek a prototypy funkcii obrazoviek.

### `enum screen`

Enum obsahuje vsetky stavy programu. Kazdy stav predstavuje jednu obrazovku alebo rezim.

Pouzitie enumu zlepsuje citatelnost kodu. Namiesto cisel ako `0`, `1` alebo `2` sa pouzivaju nazvy ako `CLOCK_SCR`, `SET_ALARM_SCR` a `TIMER_SCR`.

### Prototypy funkcii obrazoviek

Priklad:

```cpp
enum screen clock_screen(struct context *ctx);
enum screen set_alarm_screen(struct context *ctx);
```

Kazda funkcia obrazovky:

1. prijima ukazovatel na `context`;
2. vykona logiku danej obrazovky;
3. vrati dalsiu aktivnu obrazovku.

## 11. Subor `src/main.cpp`

Tento subor je zamerne kratky.

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

Co sa deje:

1. Vytvori sa struktura `context`.
2. Pociatocna obrazovka je `INIT_SCR`.
3. Spusti sa nekonecny cyklus.
4. Prikaz `switch` zavola funkciu aktivnej obrazovky.
5. Kazda funkcia obrazovky vrati dalsiu obrazovku.

Toto je hlavny dispatcher stavoveho automatu.

Vysvetlenie na obhajobe:

"`main.cpp` neobsahuje hlavnu aplikačnu logiku. Iba prepina stavy. Skutocne spravanie je implementovane vo funkciach obrazoviek a wrapper moduloch."

## 12. Subor `src/screens/init.cpp`

Tento subor zabezpecuje inicializaciu systemu.

### `context_init(struct context *ctx)`

Tato funkcia vyplni strukturu `context` pociatocnymi hodnotami.

Nastavuje:

- pociatocnu obrazovku `CLOCK_SCR`;
- stavy vsetkych styroch tlacidiel;
- nulove hodnoty teploty a vlhkosti;
- vypnute rezimy editacie casu, datumu a budika;
- predvoleny budik `07:00 OFF`;
- vypnuty snooze;
- resetovane stopky;
- casovac na `01:00`.

Je to dolezite, pretoze neinicializovane premenne mozu obsahovat nahodne hodnoty.

### `init_screen(struct context *ctx)`

Toto je prva obrazovka volana z `main.cpp`.

Dolezite operacie:

```cpp
init();
context_init(ctx);
Wire.begin();
Serial.begin(BAUD_RATE);
```

`init()` je inicializacna funkcia Arduino jadra. Je potrebna, pretoze projekt pouziva vlastny `main()` namiesto klasickych `setup()` a `loop()`.

Nastavuju sa piny:

```cpp
pinMode(BTN_MODE, INPUT_PULLUP);
pinMode(BTN_SELECT, INPUT_PULLUP);
pinMode(BTN_UP, INPUT_PULLUP);
pinMode(BTN_DOWN, INPUT_PULLUP);
pinMode(BUZZER_PIN, OUTPUT);
pinMode(STATUS_LED_PIN, OUTPUT);
```

Potom sa inicializuju hardverove moduly:

```cpp
lcd_init();
lcd_backlight(true);
clock_init();
sensors_init();
load_alarm_settings(ctx);
```

Na konci LCD zobrazi:

```txt
Alarm Clock
Ready
```

Funkcia vrati `CLOCK_SCR`, takze program prejde na obrazovku hodin.

## 13. Subor `src/lcd_wrapper.cpp`

Tento subor skryva priame pouzitie kniznice `LiquidCrystal_I2C`.

LCD objekt sa vytvara vnutri suboru:

```cpp
static LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);
```

Klucove slovo `static` znamena, ze objekt je sukromny pre `lcd_wrapper.cpp`.

### `lcd_init()`

Inicializuje LCD:

```cpp
lcd.init();
```

### `lcd_clear()`

Vymaze cely displej.

### `lcd_set_cursor(int y, int x)`

Nastavi poziciu kurzora.

Wrapper pouziva parametre ako `riadok, stlpec`, zatial co kniznica pouziva `stlpec, riadok`.

### `lcd_print(const char* text)`

Vypise text na aktualnu poziciu kurzora.

### `lcd_print_at(int y, int x, const char* text)`

Nastavi kurzor a vypise text.

### `lcd_print_line(int y, const char* text)`

Vypise cely 16-znakovy riadok doplneny medzerami:

```cpp
snprintf(line, sizeof(line), "%-16s", text);
```

Tym sa predchadza tomu, aby na LCD zostali stare znaky po dlhsom texte.

### `lcd_clear_line(int y)`

Vymaze jeden riadok LCD.

### `lcd_backlight(bool state)`

Zapne alebo vypne podsvietenie LCD.

## 14. Subor `src/sensors.cpp`

Tento subor zabezpecuje pracu so senzorom DHT11.

### `static DHT dht(DHT_PIN, DHT_TYPE)`

Vytvara objekt senzora DHT11.

### `sensors_init()`

Spusti senzor DHT:

```cpp
dht.begin();
```

### `get_temperature()`

Vrati teplotu v stupnoch Celzia.

Ak citanie zlyha, DHT kniznica moze vratit `NaN`. Kontrola sa vykonava vo funkcii `read_sensor()` v `app.cpp`.

### `get_humidity()`

Cita vlhkost.

Ak je hodnota neplatna, vrati `-1`.

Program kontroluje:

```cpp
if (!isnan(new_temperature) && new_humidity >= 0)
```

Teda do `context` sa ulozia iba platne hodnoty.

## 15. Subor `src/rtc_wrapper.cpp`

Tento subor zabezpecuje cas, datum, DS1302 RTC a EEPROM snapshot casu.

### Hlavna myslienka

Projekt pouziva DS1302 RTC modul, ale zaroven uklada posledny nastaveny datum a cas do EEPROM.

Ked pouzivatel ulozi cas alebo datum:

1. aktualizuje sa interny softverovy cas;
2. hodnota sa ulozi do EEPROM;
3. hodnota sa zapise do DS1302.

Ked Arduino startuje:

1. najprv sa pokusi nacitat EEPROM snapshot;
2. ak je snapshot platny, program startuje z tejto hodnoty;
3. ak snapshot neexistuje, pokusi sa nacitat RTC;
4. ak RTC nie je platny, pouzije zalozny defaultny cas.

Toto sluzi na spolahlive obnovenie posledne nastaveneho casu po restarte.

### EEPROM adresy

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

EEPROM uklada:

- rok;
- mesiac;
- den;
- hodinu;
- minutu;
- sekundu;
- magic byte.

Magic byte sluzi na overenie, ci EEPROM obsahuje platne ulozene data.

### `static ThreeWire rtc_wire(...)`

DS1302 je ovladany cez ThreeWire:

```cpp
static ThreeWire rtc_wire(RTC_DAT_PIN, RTC_CLK_PIN, RTC_RST_PIN);
```

Poradie parametrov:

```txt
DAT, CLK, RST
```

### `static RtcDS1302<ThreeWire> rtc(rtc_wire)`

Vytvara RTC objekt.

### `datetime_is_valid(const RtcDateTime &date_time)`

Kontroluje, ci su datum a cas platne.

Kontroluje:

- rok od 2024 do 2099;
- mesiac od 1 do 12;
- den od 1 do 31;
- hodinu od 0 do 23;
- minutu od 0 do 59;
- sekundu od 0 do 59;
- vnutornu kontrolu `date_time.IsValid()`.

Tym sa zabrani tomu, aby neplatne RTC data pokazili program.

### `fallback_datetime()`

Vrati zalozny datum a cas:

```txt
22.05.2026 00:00:00
```

Pouzije sa, ked EEPROM ani RTC neobsahuju platne data.

### `set_app_clock(const RtcDateTime &date_time)`

Spusti interny softverovy cas.

Uklada:

- zakladny cas;
- hodnotu `millis()` v momente nastavenia;
- priznak platnosti.

Potom `safe_now()` dokaze vypocitat aktualny cas pomocou `millis()`.

### `save_clock_snapshot(const RtcDateTime &date_time)`

Ulozi datum a cas do EEPROM.

Pouziva `EEPROM.update()` namiesto `EEPROM.write()`.

Rozdiel:

- `write()` zapisuje vzdy;
- `update()` zapisuje iba vtedy, ked sa hodnota zmenila.

Je to lepsie, pretoze EEPROM ma obmedzeny pocet zapisov.

### `load_clock_snapshot(RtcDateTime &date_time)`

Nacita datum a cas z EEPROM.

Algoritmus:

1. Skontroluje magic byte.
2. Ak magic byte nesedi, vrati `false`.
3. Nacita rok, mesiac, den, hodinu, minutu a sekundu.
4. Vytvori `RtcDateTime`.
5. Overi platnost.
6. Ak je platny, zapise vysledok do vystupneho parametra a vrati `true`.

### `safe_now()`

Toto je centralna bezpecna funkcia na ziskanie casu.

Algoritmus:

1. Ak je interny softverovy cas platny, vrati:

```cpp
app_clock_base + elapsed_seconds
```

2. Inak cita RTC.
3. Ak su RTC data platne, pouzije RTC.
4. Ak RTC nie je platne, pouzije fallback cas.

### `to_dt(const RtcDateTime &date_time)`

Konvertuje typ z kniznice `RtcDateTime` na projektovy typ `dt`.

Tym zvysok programu nie je priamo zavisly od typu z RTC kniznice.

### `clock_init()`

Inicializuje RTC a pociatocny cas.

Algoritmus:

1. Spusti RTC pomocou `rtc.Begin()`.
2. Vypne write protection.
3. Spusti RTC.
4. Pokusi sa nacitat cas z EEPROM.
5. Ak je EEPROM platny, pouzije ho.
6. Inak skusi RTC.
7. Ak RTC nie je platne, pouzije fallback cas.

### `set_date(...)`

Zmeni iba datum a ponecha aktualny cas.

### `set_time(...)`

Zmeni iba cas a ponecha aktualny datum.

### `set_datetime(...)`

Hlavna funkcia na ulozenie datumu a casu.

Robi:

1. vytvori `RtcDateTime`;
2. aktualizuje softverovy cas;
3. ulozi EEPROM snapshot;
4. zapise cas do RTC;
5. spusti RTC.

### `get_day()`, `get_month()`, `get_year()`

Vracaju jednotlive casti datumu.

### `get_hours()`, `get_minutes()`, `get_seconds()`

Vracaju jednotlive casti casu.

### `now()`

Vrati cely aktualny datum a cas ako `struct dt`.

Pouziva sa v obrazovkach, logike budika, datumu, casu a snooze.

### `reset_saved_datetime()`

Vymaze magic byte EEPROM snapshotu casu.

Pouziva sa pri Factory Reset.

## 16. Subor `src/screens/app.cpp`

Toto je najvacsi subor. Obsahuje hlavne spravanie aplikacie.

### EEPROM adresy pre budik

```cpp
#define EEPROM_MAGIC_ADDR 0
#define EEPROM_ALARM_HOUR_ADDR 1
#define EEPROM_ALARM_MINUTE_ADDR 2
#define EEPROM_ALARM_ENABLED_ADDR 3
#define EEPROM_MAGIC_VALUE 0xA6
```

EEPROM uklada:

- hodinu budika;
- minutu budika;
- priznak zapnutia budika;
- magic byte.

### `days_in_month(byte month, int year)`

Vrati pocet dni v mesiaci.

Riesi:

- februar;
- prestupne roky;
- mesiace s 30 dnami;
- mesiace s 31 dnami.

Zabranuje neplatnym datumom ako 31. april.

### `read_pressed(struct button_state *button)`

Cita tlacidlo s debounce.

Algoritmus:

1. Precita aktualny stav pinu.
2. Ak sa stav zmenil a preslo viac ako 60 ms, zmena sa akceptuje.
3. Ak je novy stav `LOW`, ide o stlacenie.
4. Vrati `true` iba raz na jedno stlacenie.

### `save_alarm_settings(struct context *ctx)`

Ulozi nastavenia budika do EEPROM:

- magic byte;
- hodinu budika;
- minutu budika;
- stav zapnutia.

### `load_alarm_settings(struct context *ctx)`

Nacita nastavenia budika z EEPROM.

Ak magic byte chyba, nastavi predvolene hodnoty:

```txt
07:00 OFF
```

Zaroven overi hodinu a minutu.

### `next_screen(struct context *ctx)`

Prejde na dalsiu obrazovku.

Ak je aktualna obrazovka budik alebo posledna obrazovka menu, vrati sa na `CLOCK_SCR`.

Zaroven ukonci editacne rezimy:

```cpp
ctx->setting_time = false;
ctx->setting_date = false;
ctx->setting_alarm = false;
```

### `read_sensor(struct context *ctx)`

Cita DHT11 kazde 2 sekundy.

Je to potrebne, pretoze DHT11 by sa nemal citat prilis casto.

Ak su hodnoty platne, ulozia sa do `context`.

### `start_set_time(struct context *ctx)`

Zacne editaciu casu.

Nacita aktualny cas pomocou `now()` a skopiruje hodinu a minutu do docasnych poli.

### `save_set_time(struct context *ctx)`

Ulozi upraveny cas.

Ponecha aktualny datum a ulozi novu hodinu a minutu pomocou `set_datetime()`.

Potom zobrazi:

```txt
Time saved
```

### `change_set_time(struct context *ctx, int direction)`

Meni hodinu alebo minutu.

`direction` je:

- `1` pre UP;
- `-1` pre DOWN.

Hodiny sa cyklia medzi 0 a 23. Minuty sa cyklia medzi 0 a 59.

### `start_set_date(struct context *ctx)`

Zacne editaciu datumu skopirovanim aktualneho datumu do docasnych poli.

### `save_set_date(struct context *ctx)`

Ulozi upraveny datum a ponecha aktualny cas.

### `change_set_date(struct context *ctx, int direction)`

Meni den, mesiac alebo rok.

Pri zmene mesiaca alebo roka overuje platnost dna.

### `start_set_alarm(struct context *ctx)`

Zacne editaciu budika a vyberie pole hodiny.

### `save_set_alarm(struct context *ctx)`

Ulozi nastavenia budika do EEPROM a zobrazi:

```txt
Alarm saved
```

### `change_set_alarm(struct context *ctx, int direction)`

Meni:

- hodinu budika;
- minutu budika;
- ON/OFF stav budika.

### `start_alarm_ring(struct context *ctx)`

Spusti stav zvonenia budika.

Nastavi:

- `alarm_ringing = true`;
- obrazovku na `ALARM_SCR`;
- vymaze LCD.

Buzzer sa ovlada samostatne vo funkcii `update_buzzer_and_led()`.

### `stop_alarm_ring(struct context *ctx)`

Zastavi zvuk budika a vrati sa na obrazovku hodin.

### `snooze_alarm(struct context *ctx)`

Odlozi budik o 5 minut.

Robi:

1. nacita aktualny cas;
2. prida 5 minut;
3. ulozi snooze hodinu a minutu;
4. aktivuje snooze;
5. zastavi aktualne zvonenie;
6. vrati sa na obrazovku hodin.

### `check_alarm(struct context *ctx)`

Kontroluje, ci sa ma budik spustit.

Robi:

1. ak budik uz zvoni, nic nerobi;
2. nacita aktualny cas;
3. skontroluje snooze cas;
4. ak je budik zapnuty, skontroluje bezny cas budika;
5. ak hodina a minuta sedia, spusti budik.

`last_alarm_key` zabranuje opakovanemu spusteniu budika v tej istej minute.

### `stopwatch_elapsed(struct context *ctx)`

Vypocita uplynuty cas stopiek.

Ak su stopky pozastavene, vrati ulozeny uplynuty cas. Ak bezia, pripocita cas od posledneho startu.

### `toggle_stopwatch(struct context *ctx)`

Spusti alebo pozastavi stopky.

### `reset_stopwatch(struct context *ctx)`

Vynuluje stopky.

### `start_timer(struct context *ctx)`

Spusti odpocitavaci casovac.

Vypocita trvanie:

```cpp
duration = (minutes * 60 + seconds) * 1000
```

Potom ulozi:

```cpp
timer_ends_at = millis() + timer_remaining_ms;
```

### `pause_timer(struct context *ctx)`

Pozastavi casovac a ulozi zostavajuce milisekundy.

### `resume_timer(struct context *ctx)`

Obnovi pozastaveny casovac.

### `reset_timer(struct context *ctx)`

Resetuje casovac do rezimu nastavovania a vypne buzzer.

### `change_timer_value(struct context *ctx, int direction)`

Meni minuty alebo sekundy casovaca.

Minuty sa cyklia od 0 do 99. Sekundy od 0 do 59.

### `update_timer(struct context *ctx)`

Kontroluje, ci casovac skoncil.

Ak skoncil:

- `timer_running = false`;
- `timer_done = true`;
- zostavajuci cas je 0;
- obrazovka sa prepne na `TIMER_SCR`;
- LCD sa vymaze.

### `factory_reset(struct context *ctx)`

Resetuje:

- budik na `07:00 OFF`;
- snooze;
- ulozeny snapshot casu;
- stopky;
- casovac na `01:00`.

Potom zobrazi:

```txt
Factory reset
Done
```

### `handle_buttons(struct context *ctx)`

Hlavna funkcia na spracovanie tlacidiel.

Robi:

1. cita vsetky styri tlacidla;
2. spracuje ovladanie pocas zvonenia budika;
3. spracuje ovladanie po skonceni casovaca;
4. spracuje prepnutie obrazovky cez MODE;
5. spracuje akcie SELECT podla aktivnej obrazovky;
6. spracuje UP a DOWN podla aktivnej obrazovky.

Toto je jedna z centralnych funkcii programu.

### `update_buzzer_and_led(struct context *ctx)`

Ovlada buzzer a LED.

Ak zvoni budik alebo casovac:

- buzzer sa prepina kazdych 250 ms;
- LED blika spolu s buzzerom.

Ak nic nezvoni:

- buzzer je vypnuty;
- LED svieti iba vtedy, ked je budik zapnuty.

### `show_clock(struct context *ctx)`

Zobrazuje aktualny cas.

Prvy riadok:

- `Time`, ak je budik vypnuty;
- `Time        AL`, ak je budik zapnuty.

Druhy riadok:

- cas vo formate `HH:MM:SS`;
- `SNZ`, ak je aktivny snooze.

### `show_date()`

Zobrazuje datum vo formate:

```txt
DD.MM.YYYY
```

### `show_env(struct context *ctx)`

Zobrazuje teplotu a vlhkost.

Teplota sa zaokruhluje na cele cislo.

### `show_set_time(struct context *ctx)`

Zobrazuje obrazovku nastavenia casu.

Pred editaciou:

```txt
Set Time
SELECT to edit
```

Pocas editacie:

- `Edit hour`;
- alebo `Edit minute`.

### `show_set_date(struct context *ctx)`

Zobrazuje obrazovku nastavenia datumu pre den, mesiac a rok.

### `show_set_alarm(struct context *ctx)`

Zobrazuje obrazovku nastavenia budika.

Format:

```txt
HH:MM ON
```

alebo

```txt
HH:MM OFF
```

### `show_stopwatch(struct context *ctx)`

Zobrazuje stopky vo formate:

```txt
MM:SS.HH
```

kde `HH` znamena stotiny sekundy.

### `show_timer(struct context *ctx)`

Zobrazuje stav casovaca:

- nastavovanie minut;
- nastavovanie sekund;
- bezi;
- pozastaveny;
- dokonceny.

### `show_factory_reset()`

Zobrazuje:

```txt
Factory Reset
SELECT to reset
```

### `show_alarm_ringing(struct context *ctx)`

Zobrazuje:

```txt
Wake up! HH:MM
SEL snooze M off
```

Vyznama:

- `SELECT` - snooze;
- `MODE` - vypnut budik.

### `show_current_screen(struct context *ctx)`

Vola spravnu zobrazovaciu funkciu podla `current_screen`.

### `run_screen(struct context *ctx, enum screen active_screen)`

Spolocna hlavna logika pre vsetky obrazovky.

Robi:

1. nastavi aktivnu obrazovku;
2. spracuje tlacidla;
3. precita senzor;
4. aktualizuje casovac;
5. skontroluje budik;
6. aktualizuje buzzer a LED;
7. prekresli aktualnu obrazovku;
8. pocka 40 ms;
9. vrati aktualnu obrazovku.

Tym sa zabranilo duplikovaniu rovnakej logiky v kazdej obrazovke.

### Jednotlive screen funkcie

Funkcie ako:

```cpp
enum screen clock_screen(struct context *ctx)
```

iba volaju `run_screen()` s vlastnou hodnotou obrazovky.

Tym sa dodrziava struktura ocakavana skeleton projektom.

## 17. Celkovy tok programu

1. `main()` zacne so stavom `INIT_SCR`.
2. `init_screen()` inicializuje Arduino, LCD, RTC, DHT11, tlacidla, buzzer, LED a EEPROM nastavenia.
3. Program prejde do `CLOCK_SCR`.
4. Aktivna screen funkcia sa opakovane vola.
5. Screen funkcia vola `run_screen()`.
6. `run_screen()` spracuje tlacidla, senzor, budik, casovac, LED, buzzer a LCD.
7. Ak sa stlaci MODE, obrazovka sa zmeni.
8. Ak cas budika zodpoveda aktualnemu casu, program prejde do `ALARM_SCR`.

## 18. Tok nastavenia casu

1. Pouzivatel prejde na `Set Time`.
2. Stlaci `SELECT`.
3. Zacne sa editacia hodiny.
4. `UP/DOWN` meni hodinu.
5. `SELECT` prejde na editaciu minut.
6. `UP/DOWN` meni minuty.
7. `SELECT` ulozi cas.

Ulozenie sa vykonava cez:

```cpp
set_datetime(...)
```

Tym sa cas ulozi do softveroveho casu, EEPROM snapshotu a RTC.

## 19. Tok nastavenia budika

1. Pouzivatel prejde na `Set Alarm`.
2. `SELECT` zacne editaciu hodiny.
3. `UP/DOWN` meni hodinu.
4. `SELECT` prejde na editaciu minut.
5. `UP/DOWN` meni minutu.
6. `SELECT` prejde na ON/OFF.
7. `UP/DOWN` prepina ON/OFF.
8. `SELECT` ulozi budik.

Data budika sa ukladaju do EEPROM.

## 20. Logika budika

`check_alarm()` neustale kontroluje aktualny cas.

Ak:

```txt
current.hours == alarm_hour
current.minutes == alarm_minute
alarm_enabled == true
```

zavola sa `start_alarm_ring()`.

Potom:

- obrazovka sa zmeni na `ALARM_SCR`;
- `update_buzzer_and_led()` spusti buzzer;
- LED zacne blikat.

## 21. Logika Snooze

Ked budik zvoni, stlacenie `SELECT` zavola `snooze_alarm()`.

Funkcia prida 5 minut k aktualnemu casu a ulozi ho do:

```cpp
snooze_hour
snooze_minute
```

Potom budik prestane zvonit.

Ked nastane snooze cas, `check_alarm()` znovu spusti budik.

## 22. Logika stopiek

Stopky pouzivaju `millis()`, nie RTC.

Pri starte sa ulozi:

```cpp
stopwatch_started_at = millis();
```

Pri pauze sa akumuluje uplynuty cas.

Pri resete sa cas vynuluje.

## 23. Logika casovaca

Casovac tiez pouziva `millis()`.

Pri starte:

```cpp
timer_ends_at = millis() + timer_remaining_ms;
```

`update_timer()` kontroluje, ci aktualne `millis()` dosiahlo `timer_ends_at`.

Ak ano:

- casovac skonci;
- `timer_done = true`;
- buzzer sa spusti.

## 24. Vysvetlenie EEPROM

EEPROM sa pouziva na dve veci:

1. Ulozenie nastaveni budika.
2. Ulozenie posledne nastaveneho datumu a casu.

Kod pouziva `EEPROM.update()`, aby sa znizil pocet zbytocnych zapisov.

Magic byte sa pouziva na kontrolu, ci su ulozene data platne.

## 25. Vysvetlenie RTC

Projekt pouziva DS1302 RTC.

Je pripojeny cez:

```txt
DAT
CLK
RST
```

nie cez I2C.

RTC sa inicializuje vo funkcii `clock_init()`.

Ked sa nastavuje cas, `set_datetime()` ho zapise do RTC pomocou:

```cpp
rtc.SetDateTime(updated);
```

Kod zaroven overuje RTC data, aby sa predislo neplatnym hodnotam.

## 26. Vysvetlenie stavoveho automatu

Program je implementovany ako stavovy automat.

Zoznam stavov je deklarovany v `screens.h`.

`main.cpp` obsahuje nekonecny cyklus a switch, ktory vola funkciu aktivneho stavu.

Kazda funkcia obrazovky vracia dalsi stav.

Vyhody:

- jednoducho sa pridavaju nove obrazovky;
- kod je citatelnejsi;
- kazdy rezim je oddeleny;
- netreba jeden obrovsky `loop()`.

## 27. Doplnkove funkcie

Projekt obsahuje tieto doplnkove funkcie:

### Snooze

Odlozi budik o 5 minut.

### Stopky

Stopky so startom, pauzou a resetom.

### Odpocitavaci casovac

Casovac, ktory zazvoni po dosiahnuti nuly.

### Factory Reset

Resetuje budik, casovac, stopky a ulozeny cas.

Ak treba uviest presne tri doplnkove funkcie, pouzi:

1. Snooze.
2. Stopky.
3. Odpocitavaci casovac.

Factory reset moze byt spomenuty ako pomocna doplnkova funkcia.

## 28. Mozne otazky na obhajobe

### Preco sa pouziva `INPUT_PULLUP`?

Arduino ma vstavane pull-up rezistory. Netreba teda externe pull-down rezistory. Uvolnene tlacidlo cita `HIGH`, stlacene tlacidlo cita `LOW`.

### Preco je potrebny debounce?

Mechanicke tlacidla mozu elektricky zakmitat. Debounce zabrani tomu, aby jedno fyzicke stlacenie bolo detegovane viac krat.

### Preco sa pouziva `context`?

Uchovava cely stav programu v jednej strukture a odovzdava ho medzi funkciami obrazoviek.

### Preco je `main.cpp` kratky?

Pretoze projekt je modularny. `main.cpp` iba prepina stavy, logika je v samostatnych moduloch.

### Preco pouzit LCD wrapper?

Aby logika obrazoviek nebola priamo zavisla od LCD kniznice. Ak by sa zmenila LCD kniznica, upravi sa iba wrapper.

### Preco sa DHT11 necita v kazdom cykle?

DHT11 by sa nemal citat prilis casto. Program ho cita priblizne kazde 2 sekundy.

### Ako budik zabranjuje viacnasobnemu spusteniu v tej istej minute?

Pouziva `last_alarm_key`, ktory uchovava den a minutu posledneho spustenia budika.

### Ako funguje LED?

Ak je budik zapnuty, LED svieti. Ak zvoni budik alebo casovac, LED blika spolu s buzzerom.

### Preco sa pre stopky a casovac pouziva `millis()` namiesto `delay()`?

`millis()` umoznuje neblokujuci vypocet casu. Program moze stale citat tlacidla a aktualizovat displej.

## 29. Checklist na demonstraciu

Pred obhajobou skontroluj:

- LCD zobrazuje startovaciu spravu;
- `MODE` prechadza cez vsetky obrazovky;
- `Time` zobrazuje cas;
- `Date` zobrazuje datum;
- `Temp/Humidity` zobrazuje hodnoty z DHT11;
- `Set Time` meni hodinu a minutu;
- `Set Date` meni den, mesiac a rok;
- po restarte sa obnovi posledny nastaveny cas;
- `Set Alarm` nastavi cas budika;
- LED sa zapne, ked je budik aktivny;
- budik zazvoni v nastavenom case;
- `SELECT` pocas zvonenia aktivuje snooze;
- `MODE` pocas zvonenia vypne budik;
- stopky sa spustia, pozastavia a resetuju;
- casovac sa da nastavit, spustit, pozastavit a po skonceni zazvoni;
- factory reset resetuje nastavenia.

## 30. Vysvetlenie projektu za jednu minutu

Toto je budik na Arduino UNO. Pouziva LCD displej, RTC DS1302, senzor DHT11, tlacidla, buzzer a LED. Program je implementovany ako stavovy automat, kde kazdy rezim je samostatna obrazovka. `main.cpp` iba prepina obrazovky, hlavna logika je v `screens/app.cpp`. Stav programu je ulozeny v strukture `context`. Cas a datum spracovava `rtc_wrapper`, LCD vystup `lcd_wrapper` a DHT11 `sensors`. Pouzivatel moze nastavit cas, datum a budik pomocou tlacidiel. Nastavenia budika su ulozene v EEPROM a pri zvoneni sa spusti buzzer a LED blika. Doplnkove funkcie su snooze, stopky a odpocitavaci casovac.
