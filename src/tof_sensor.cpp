#include "tof_sensor.h"
#include <vl53l4cx_class.h>

// Define XSHUT pin for enabling the sensor
#define TOF_XSHUT_PIN 10

// Create the ToF sensor object
VL53L4CX sensor_vl53l4cx(&Wire, TOF_XSHUT_PIN);

// Initialize the ToF sensor
bool initToF() {

    // Begin sensor I2C communication
    sensor_vl53l4cx.begin();

    // Initialize sensor with default I2C address (0x29)
    if (sensor_vl53l4cx.InitSensor(0x29) != 0) {
        return false;
    }

    // Start continuous measurement
    sensor_vl53l4cx.VL53L4CX_StartMeasurement();

    // Initialization successful
    return true;
}

// Read distance from ToF sensor
ToFData readToF() {

    // Create result structure
    ToFData data;
    data.distance = -1;  // Default when no valid reading

    // Variable to check new data availability
    uint8_t newDataReady = 0;

    // Structure holding multi-range results
    VL53L4CX_MultiRangingData_t rangingData;

    // Check if new measurement is available
    if (sensor_vl53l4cx.VL53L4CX_GetMeasurementDataReady(&newDataReady) == 0 && newDataReady) {

        // Retrieve measurement data
        sensor_vl53l4cx.VL53L4CX_GetMultiRangingData(&rangingData);

        // If valid object detected and measurement is OK
        if (rangingData.NumberOfObjectsFound > 0 &&
            rangingData.RangeData[0].RangeStatus == 0) {

            // Store measured distance in millimeters
            data.distance = rangingData.RangeData[0].RangeMilliMeter;
        }

        // Clear interrupt and restart next measurement
        sensor_vl53l4cx.VL53L4CX_ClearInterruptAndStartMeasurement();
    }

    // Return the distance reading
    return data;
}
