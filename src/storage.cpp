#include "storage.h"
#include <SD.h>

// Initialize SD card
bool initSD() {
    // Attempt to start SD card on default CS pin
    if (!SD.begin(9)) {
        return false;
    }

    // Ensure log file exists by creating initial header
    if (!SD.exists("/log.csv")) {
        clearLogFile();
    }

    // SD initialization successful
    return true;
}

// Append one line of CSV data into log.csv
void logData(const String &line) {

    // Open log file in append mode
    File file = SD.open("/log.csv", FILE_APPEND);

    // Return if file could not be opened
    if (!file) return;

    // Write the CSV line
    file.println(line);

    // Close the file
    file.close();
}

// Delete current log.csv and recreate it with a header row
void clearLogFile() {

    // Remove existing log file if it exists
    if (SD.exists("/log.csv")) {
        SD.remove("/log.csv");
    }

    // Create new log file with header
    File file = SD.open("/log.csv", FILE_WRITE);

    // Return if file could not be opened
    if (!file) return;

    // Write CSV header
    file.println("timestamp,temperature,humidity,pressure,gas,distance,latitude,longitude,fix");

    // Close file
    file.close();
}
