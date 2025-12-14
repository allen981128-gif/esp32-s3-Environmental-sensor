#pragma once
#include <Arduino.h>

// Initialize SD card
bool initSD();

// Write one line of CSV data into log.csv
void logData(const String &line);

// Delete existing log.csv and create a new empty file with header
void clearLogFile();
