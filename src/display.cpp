#include "display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define OLED resolution and address
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3D

// Create OLED object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Initialize the OLED
void initDisplay() {
    // Start the display
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

    // Clear screen
    display.clearDisplay();

    // Set text size and color
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Show initial message
    display.setCursor(0, 0);
    display.println("OLED Ready");
    display.display();
}

// Display a single-line message
void displayMessage(const String &msg) {
    // Clear the screen
    display.clearDisplay();

    // Set cursor to top-left
    display.setCursor(0, 0);

    // Print the message
    display.println(msg);

    // Apply changes
    display.display();
}

// Show welcome screen before system enters run mode
void showWelcomeScreen() {
    // Clear screen
    display.clearDisplay();

    // Position cursor
    display.setCursor(0, 0);

    // Show system information
    display.println("ESP32-S3 Logger");
    display.println("");
    display.println("WiFi: Sensor-AP");
    display.println("PASS: 12345678");
    display.println("");
    display.println("http://192.168.4.1");
    display.println("");
    display.println("Press BOOT to start");

    // Apply changes
    display.display();
}

// Update OLED with live sensor data
void updateDisplay(const DisplayData &data) {
    // Clear the display
    display.clearDisplay();

    // Position cursor
    display.setCursor(0, 0);

    // Display temperature and humidity
    display.printf("T: %.1fC\n", data.temperature);
    display.printf("H:%.0f%%\n", data.humidity);

    // Display pressure and gas resistance
    display.printf("P: %.1fkPa\n", data.pressure);
    display.printf("G:%.1f\n", data.gas);
    // Display ToF distance
    if (data.distance >= 0)
        display.printf("D: %d mm\n", data.distance);
    else
        display.println("D: -- mm");

    // Display GPS data
    if (data.gps_fix) {
        display.printf("GPS:%s,%s\n", 
                       data.gps_lat.c_str(), 
                       data.gps_lon.c_str());
    } else {
        display.printf("GPS: searching (%d)\n", data.gps_satellites);
    }

    // Display SD card status
    display.printf("SD: %s\n", data.sd_status.c_str());

    // Apply changes
    display.display();
}
