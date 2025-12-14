#pragma once

#include <WebServer.h>

// Global WebServer, already in use within main.cpp
extern WebServer server;

// Start WiFi AP + DNS + WebServer
void startWiFiAP();

// Start only the HTTP server (typically for internal use)
void startWebServer();

// Called within the loop() function to handle DNS and HTTP requests
void wifiServerLoop();
