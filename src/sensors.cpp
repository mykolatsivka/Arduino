#include <Arduino.h>
#include <DHT.h>

#include "config.h"
#include "sensors.h"

#define DHT_TYPE DHT11

static DHT dht(DHT_PIN, DHT_TYPE);

void sensors_init() {
    dht.begin();
}

float get_temperature() {
    return dht.readTemperature();
}

int get_humidity() {
    float humidity = dht.readHumidity();

    if (isnan(humidity)) {
        return -1;
    }

    return (int)humidity;
}
