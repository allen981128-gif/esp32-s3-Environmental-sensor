#pragma once
#include <Arduino.h>

// Structure holding all data shown on OLED
struct DisplayData {
    float temperature;
    float humidity;
    float pressure;
    float gas;
    int distance;
    String gps_lat;
    String gps_lon;
    bool gps_fix;
    uint8_t gps_satellites;
    String sd_status;
};

// Initialize the OLED display
void initDisplay();

// Show a message on the OLED
void displayMessage(const String &msg);

// Update the OLED with current sensor data
void updateDisplay(const DisplayData &data);

// Show welcome screen before system starts
void showWelcomeScreen();


