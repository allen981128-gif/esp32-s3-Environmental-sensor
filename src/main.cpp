#include <Arduino.h>
#include <Wire.h>
#include "wifi_server.h"
#include "bme680_sensor.h"
#include "tof_sensor.h"
#include "gps_sensor.h"
#include "display.h"
#include "storage.h"

// Define BOOT button pin
#define BUTTON_PIN 0

// Track logging state
bool loggingEnabled = false;

// Track number of logged samples
int logCount = 0;

// Track button press timing
unsigned long buttonPressStart = 0;

// Track if long press event was handled
bool longPressHandled = false;

// Track last SD log timestamp
unsigned long lastLogTime = 0;

// Track last OLED update timestamp
unsigned long lastDisplayUpdate = 0;

// Logging interval (50 Hz)
#define LOG_INTERVAL_MS 20

// Track whether system has started
bool systemStarted = false;

// Structure used for OLED display
DisplayData dispData;

// Handle BOOT button events
void handleButton() {

    // Read current button state
    bool pressed = (digitalRead(BUTTON_PIN) == LOW);

    // If button is pressed
    if (pressed) {

        // Store the press start time
        if (buttonPressStart == 0)
            buttonPressStart = millis();

        // Compute how long the button has been held
        unsigned long held = millis() - buttonPressStart;

        // If button is held >2 seconds, clear SD card
        if (held > 2000 && !longPressHandled) {
            clearLogFile();
            dispData.sd_status = "Cleared";
            longPressHandled = true;
        }
    }

    // If button is released
    else {

        // If press began previously
        if (buttonPressStart != 0) {

            // Compute total press duration
            unsigned long held = millis() - buttonPressStart;

            // If short press, toggle logging
            if (held < 2000 && !longPressHandled) {
                loggingEnabled = !loggingEnabled;

                // Reset counter and show SD status
                if (loggingEnabled) {
                    logCount = 0;
                    dispData.sd_status = "Logging 0/100";
                }
                else {
                    dispData.sd_status = "Stopped";
                }
            }
        }

        // Reset button tracking
        buttonPressStart = 0;
        longPressHandled = false;
    }
}

// Setup function
void setup() {

    // Start I2C bus
    Wire.begin();

    // Start serial port
    Serial.begin(115200);

    // Short startup delay
    delay(200);

    // Configure BOOT button
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Enable sensor power rail
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, HIGH);

    // Print startup instructions
    Serial.println("ESP32-S3 Sensor Logger");
    Serial.println("WiFi: Sensor-AP");
    Serial.println("Password: 12345678");
    Serial.println("Visit: http://192.168.4.1");
    Serial.println("Press BOOT to start the system");

    // Initialize OLED
    initDisplay();

    // Show welcome screen with WiFi info
    showWelcomeScreen();

    // Start WiFi Access Point
    startWiFiAP();

    // Initialize sensors (print errors only)
    if (!initBME680()) Serial.println("ERROR: BME680 init failed");
    if (!initToF())    Serial.println("ERROR: ToF init failed");
    if (!initGPS())    Serial.println("ERROR: GPS init failed");

    // Initialize SD card
    if (!initSD()) {
        dispData.sd_status = "ERROR";
        Serial.println("ERROR: SD card init failed");
    }
    else {
        dispData.sd_status = "Idle";
    }
}

// Loop function
void loop() {

    // If system has not started yet
    if (!systemStarted) {

        // Wait for BOOT press to enter run mode
        if (digitalRead(BUTTON_PIN) == LOW) {
            delay(200);
            systemStarted = true;

            // Clear OLED for normal display
            displayMessage("");

            Serial.println("System started\n");
        }

        // Keep serving WiFi clients while waiting
        wifiServerLoop();
        return;
    }

    // Handle BOOT button actions
    handleButton();

    // Poll GPS UART
    gpsPoll();

    // Read sensors
    BME680Data env = readBME680();
    ToFData tof = readToF();
    GPSData gps = readGPS();

    // Handle SD logging
    if (loggingEnabled && millis() - lastLogTime >= LOG_INTERVAL_MS) {

        // Update log time
        lastLogTime = millis();

        // Build timestamp string
        String timestamp = "--";
        if (gps.fix) {
            char buf[32];
            sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
                    gps.year + 2000,
                    gps.month,
                    gps.day,
                    gps.hours,
                    gps.minutes,
                    gps.seconds);
            timestamp = String(buf);
        }

        // Build CSV row
        String line = "";
        line += timestamp + ",";
        line += String(env.temperature) + ",";
        line += String(env.humidity) + ",";
        line += String(env.pressure) + ",";
        line += String(env.gas) + ",";
        line += String(tof.distance) + ",";
        line += gps.latitude + ",";
        line += gps.longitude + ",";
        line += (gps.fix ? "1" : "0");

        // Write to SD
        logData(line);

        // Update counter
        logCount++;

        // Update OLED status
        dispData.sd_status = "Logging " + String(logCount) + "/100";

        // Stop after 100 samples
        if (logCount >= 100) {
            loggingEnabled = false;
            dispData.sd_status = "Done";
        }
    }

    // Update OLED every 0.5 seconds
    if (millis() - lastDisplayUpdate > 500) {

        // Copy sensor data
        dispData.temperature = env.temperature;
        dispData.humidity = env.humidity;
        dispData.pressure = env.pressure;
        dispData.gas = env.gas;
        dispData.distance = tof.distance;
        dispData.gps_lat = gps.latitude;
        dispData.gps_lon = gps.longitude;
        dispData.gps_fix = gps.fix;

        // Update OLED
        updateDisplay(dispData);

        // Update timestamp
        lastDisplayUpdate = millis();

        // Print one compact line of sensor output
        Serial.print("T:");
        Serial.print(env.temperature);
        Serial.print(" H:");
        Serial.print(env.humidity);
        Serial.print(" P:");
        Serial.print(env.pressure);
        Serial.print(" G:");
        Serial.print(env.gas);
        Serial.print(" D:");
        Serial.print(tof.distance);
        Serial.print(" LAT:");
        Serial.print(gps.latitude);
        Serial.print(" LON:");
        Serial.print(gps.longitude);
        Serial.print(" FIX:");
        Serial.print(gps.fix);
        Serial.print(" SD:");
        Serial.println(dispData.sd_status);
    }

    // Handle WiFi client requests
    server.handleClient();
    wifiServerLoop();

    // Short delay for stability
    delay(5);
}
