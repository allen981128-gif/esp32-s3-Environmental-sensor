#include "bme680_sensor.h"
#include <Adafruit_BME680.h>
#include <math.h>

// Create BME680 sensor object in I2C mode
Adafruit_BME680 bme;
static float aqi_filtered = NAN;   // 记住平滑后的 AQI

// Initialize BME680 sensor
bool initBME680() {

    // Begin BME680 at default I2C address 0x76
    if (!bme.begin(0x76)) {
        return false;  
    }

    // Set temperature oversampling
    bme.setTemperatureOversampling(BME680_OS_8X);

    // Set humidity oversampling
    bme.setHumidityOversampling(BME680_OS_2X);

    // Set pressure oversampling
    bme.setPressureOversampling(BME680_OS_4X);

    // Enable IIR filter
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

    // Set gas heater temperature and duration
    bme.setGasHeater(320, 150);

    // Initialization successful
    return true;
}

// Read BME680 data
BME680Data readBME680() {

    // Create data structure
    BME680Data data;

    // Perform sensor reading
    bme.performReading();

    // Read temperature (°C)
    data.temperature = bme.temperature - 5;

    // Read humidity (%)
    data.humidity = bme.humidity;

    // Read pressure (convert hPa -> kPa)
    data.pressure = bme.pressure / 1000.0;

    float gas_ohm = bme.gas_resistance;  // Ohm

// Reference point: Outdoor 43kΩ -> AQI 41
    const float REF_RES_OHM    = 43000.0f;
    const float REF_AQI        = 41.0f;

// Indoor benchmark: 24kΩ -> AQI 80
    const float INDOOR_RES_OHM = 24000.0f;
    const float INDOOR_AQI     = 80.0f;

    const float B_COEFF = (INDOOR_AQI - REF_AQI) /
                      logf(REF_RES_OHM / INDOOR_RES_OHM);

// Preventing outliers
    if (gas_ohm < 100.0f) gas_ohm = 100.0f;

// 1) First calculate the "instantaneous AQI"
    float aqi_raw = REF_AQI + B_COEFF * logf(REF_RES_OHM / gas_ohm);

// Restricted to 0–500
    if (aqi_raw < 0.0f)   aqi_raw = 0.0f;
    if (aqi_raw > 500.0f) aqi_raw = 500.0f;

// 2) Then perform "exponential smoothing": the new value only gradually replaces the old value
    const float ALPHA = 0.1f;  // The smaller the value, the smoother the result (try values between 0.05 and 0.2)

    if (isnan(aqi_filtered)) {
        aqi_filtered = aqi_raw;                // Initialise with the current value for the first time
    } else {
        aqi_filtered += ALPHA * (aqi_raw - aqi_filtered);
    }

// 3) Store the smoothed AQI in data.gas
    data.gas = aqi_filtered;

    // Return sensor data
    return data;
}
