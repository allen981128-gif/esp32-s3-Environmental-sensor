#pragma once
#include <Arduino.h>

// Structure holding a single ToF distance reading
struct ToFData {
    int distance;
};

// Initialize VL53L4CX ToF sensor
bool initToF();

// Read current ToF distance measurement
ToFData readToF();

