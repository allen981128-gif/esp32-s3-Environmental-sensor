#pragma once
#include <Arduino.h>
#include <Adafruit_GPS.h>

// Define RX/TX pins for GPS UART
#define GPS_RX_PIN 39
#define GPS_TX_PIN 38

// Structure to store parsed GPS data
struct GPSData {
    String latitude;
    String longitude;
    bool fix;
    int satellites;
    int year;
    int month;
    int day;
    int hours;
    int minutes;
    int seconds;
};

// Declare GPS object
extern Adafruit_GPS GPS;

// Initialize GPS module
bool initGPS();

// Poll GPS UART for incoming characters
void gpsPoll();

// Read and parse latest GPS information
GPSData readGPS();



