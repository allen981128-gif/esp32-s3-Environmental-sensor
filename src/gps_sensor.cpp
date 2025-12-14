#include "gps_sensor.h"

// UART1: Feather ESP32-S3 RX=38 TX=39
HardwareSerial GPS_Serial(1);
Adafruit_GPS GPS(&GPS_Serial);

// Initialize GPS UART and NMEA output settings
bool initGPS() {

    GPS_Serial.begin(9600, SERIAL_8N1, 38, 39);   // RX=38, TX=39
    delay(300);

    // Output RMC + GGA only (sufficient for fix/time/lat/lon)
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

    // Set update rate (1Hz)
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

    return true;
}


void gpsPoll() {

    while (GPS_Serial.available()) {

        char c = GPS.read();    // Read incoming character

        // If a full NMEA sentence has been received, parse it immediately
        if (GPS.newNMEAreceived()) {
            GPS.parse(GPS.lastNMEA());
        }
    }
}



// Return the most recently parsed GPS data
GPSData readGPS() {

    GPSData data = {"--", "--", 0, false};

    // Always use LAST parsed values (gpsPoll() has already parsed them)
    data.fix        = GPS.fix;
    data.satellites = GPS.satellites;

    if (GPS.fix) {

        data.latitude  = String(GPS.latitudeDegrees, 2) + GPS.lat;
        data.longitude = String(GPS.longitudeDegrees, 2) + GPS.lon;

        data.year   = GPS.year;
        data.month  = GPS.month;
        data.day    = GPS.day;

        data.hours   = GPS.hour;
        data.minutes = GPS.minute;
        data.seconds = GPS.seconds;
    }

    return data;
}
