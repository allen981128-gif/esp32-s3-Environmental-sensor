#pragma once
#include <Arduino.h>

// Structure to store BME680 sensor readings
struct BME680Data {
    float temperature;
    float humidity;
    float pressure;
    float gas;
};

// Initialize BME680 module
bool initBME680();

// Read temperature, humidity, pressure, and gas resistance
BME680Data readBME680();

