#include "wifi_server.h"
#include <WiFi.h>
#include <SD.h>
#include <DNSServer.h>
#include <math.h>

#include "display.h"
#include "storage.h"



WebServer server(80);
static DNSServer dnsServer;

static const char* AP_SSID = "Sensor-AP";
static const char* AP_PASS = "12345678";

static const byte DNS_PORT = 53;
static IPAddress apIP(192, 168, 4, 1);

// 从 main.cpp 引用
extern bool loggingEnabled;
extern int  logCount;
extern DisplayData dispData;
extern bool systemStarted; 



static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <title>ESP32-S3 Env Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <style>
    * { box-sizing: border-box; }

    body {
      margin: 0;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", system-ui, sans-serif;
      background: radial-gradient(circle at top, #0f172a 0, #020617 55%, #000 100%);
      color: #e5e7eb;
    }

    .container {
      max-width: 1100px;
      margin: 0 auto;
      padding: 24px 16px 40px;
    }

    header {
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;
      gap: 12px;
      margin-bottom: 20px;
    }

    .title {
      font-size: 24px;
      font-weight: 600;
    }

    .subtitle {
      font-size: 13px;
      opacity: 0.8;
    }

    .badge {
      padding: 6px 12px;
      border-radius: 999px;
      font-size: 12px;
      background: rgba(15, 23, 42, 0.9);
      border: 1px solid rgba(148, 163, 184, 0.5);
      line-height: 1.4;
    }

    .card-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
      gap: 16px;
    }

    .card {
      background: rgba(15, 23, 42, 0.95);
      border-radius: 16px;
      padding: 14px 16px 16px;
      box-shadow: 0 18px 40px rgba(0, 0, 0, 0.45);
      border: 1px solid rgba(30, 64, 175, 0.6);
    }

    .card h3 {
      margin: 0 0 6px;
      font-size: 13px;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      color: #9ca3af;
    }

    .big-value {
      font-size: 30px;
      font-weight: 600;
    }

    .unit {
      font-size: 16px;
      opacity: 0.7;
      margin-left: 4px;
    }

    .muted {
      font-size: 13px;
      opacity: 0.75;
    }

    .actions {
      margin-top: 22px;
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
    }

    button {
      border: none;
      border-radius: 999px;
      padding: 9px 16px;
      font-size: 14px;
      font-weight: 500;
      cursor: pointer;
      background: #0f172a;
      color: #e5e7eb;
      border: 1px solid rgba(148, 163, 184, 0.6);
      transition: background 0.18s, transform 0.18s, box-shadow 0.18s, border-color 0.18s;
    }

    button.primary {
      background: linear-gradient(135deg, #22c55e, #16a34a);
      border-color: rgba(34, 197, 94, 0.7);
    }

    button.secondary {
      background: linear-gradient(135deg, #0ea5e9, #2563eb);
      border-color: rgba(37, 99, 235, 0.9);
    }

    button.danger {
      background: linear-gradient(135deg, #f97316, #ef4444);
      border-color: rgba(239, 68, 68, 0.85);
    }

    button:hover {
      transform: translateY(-1px);
      box-shadow: 0 16px 30px rgba(0, 0, 0, 0.35);
    }

    .status-row {
      margin-top: 16px;
      display: flex;
      justify-content: space-between;
      flex-wrap: wrap;
      gap: 12px;
      font-size: 13px;
      opacity: 0.85;
    }

    .progress {
      margin-top: 6px;
      width: 100%;
      height: 7px;
      border-radius: 999px;
      background: rgba(15, 23, 42, 0.85);
      overflow: hidden;
    }

    .progress-bar {
      height: 100%;
      width: 0%;
      background: linear-gradient(90deg, #22c55e, #0ea5e9);
      transition: width 0.25s;
    }

    /* Logging 卡片：横向布局，左状态右按钮 */
    .card-logging {
      grid-column: span 2;
      display: flex;
      flex-direction: row;
      gap: 14px;
      align-items: flex-start;
    }

    .log-info {
      flex: 1;
      min-width: 0;
    }

    .log-buttons {
      display: flex;
      flex-direction: column;
      gap: 8px;
      min-width: 140px;
    }

    .log-buttons button {
      width: 100%;
      justify-content: center;
    }

    @media (max-width: 600px) {
      .title { font-size: 20px; }
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <div>
        <div class="title">ESP32-S3 Env Dashboard</div>
        <div class="subtitle">Live data from BME680 / ToF / GPS</div>
      </div>
      <div class="badge">
        Wi-Fi AP: <strong>Sensor-AP</strong><br />
        Password: <strong>12345678</strong>
      </div>
    </header>

    <section class="card-grid">
      <div class="card">
        <h3>Temperature</h3>
        <div class="big-value">
          <span id="val-temp">--</span><span class="unit">°C</span>
        </div>
        <div class="muted">Ambient temperature</div>
      </div>

      <div class="card">
        <h3>Humidity</h3>
        <div class="big-value">
          <span id="val-hum">--</span><span class="unit">%</span>
        </div>
        <div class="muted">Relative humidity</div>
      </div>

      <div class="card">
        <h3>Pressure</h3>
        <div class="big-value">
          <span id="val-pres">--</span><span class="unit">kPa</span>
        </div>
        <div class="muted">Barometric pressure</div>
      </div>

      <div class="card">
        <h3>Air quality (AQI*)</h3>
        <div class="big-value">
          <span id="val-gas">--</span><span class="unit">AQI</span>
        </div>
        <div class="muted">0 = good, 500 = bad (relative)</div>
      </div>

      <div class="card">
        <h3>Distance</h3>
        <div class="big-value">
          <span id="val-dist">--</span><span class="unit">mm</span>
        </div>
        <div class="muted">ToF sensor distance</div>
      </div>

      <div class="card">
        <h3>GPS</h3>
        <div class="muted" id="val-gps">Waiting for fix…</div>
        <div class="muted" style="margin-top: 8px;">
          Fix: <span id="val-gps-fix">0</span>
        </div>
      </div>

      <!-- Logging / SD 宽卡片：左状态右按钮 -->
      <div class="card card-logging">
        <div class="log-info">
          <h3>Logging / SD</h3>
          <div class="muted">State: <span id="val-log-state">Idle</span></div>
          <div class="muted">Samples: <span id="val-samples">0</span> / 100</div>
          <div class="muted">SD: <span id="val-sd">--</span></div>
          <div class="progress">
            <div id="log-progress" class="progress-bar"></div>
          </div>
        </div>
        <div class="log-buttons">
          <button class="secondary"
                  onclick="sendCommand('/api/log/start','Logging started')">
            Start logging
          </button>
          <button class="secondary"
                  onclick="sendCommand('/api/log/stop','Logging stopped')">
            Stop logging
          </button>
          <button class="danger"
                  onclick="sendCommand('/api/log/clear','Log cleared')">
            Clear SD log
          </button>
        </div>
      </div>
    </section>

    <section class="actions">
      <!-- 仅负责进入测量模式，相当于按下 BOOT -->
      <button class="primary" onclick="sendCommand('/api/system/start','Measurement started')">
        Start measurement
      </button>
      <button onclick="downloadCsv()">Download CSV</button>
    </section>

    <div class="status-row">
      <div>Last update: <span id="val-updated">--</span></div>
      <div id="action-status">Ready.</div>
    </div>
  </div>

  <script>
    async function fetchStatus() {
      try {
        const res = await fetch('/api/status');
        if (!res.ok) throw new Error('HTTP ' + res.status);
        const data = await res.json();

        if (typeof data.temperature === 'number') {
          document.getElementById('val-temp').textContent = data.temperature.toFixed(1);
        }

        if (typeof data.humidity === 'number') {
          document.getElementById('val-hum').textContent = data.humidity.toFixed(1);
        }

        if (typeof data.pressure === 'number') {
          document.getElementById('val-pres').textContent = data.pressure.toFixed(2);
        }

        if (typeof data.gas === 'number') {
          document.getElementById('val-gas').textContent = data.gas.toFixed(0);
        }

        if (typeof data.distance === 'number') {
          document.getElementById('val-dist').textContent = data.distance.toFixed(0);
        }

        document.getElementById('val-gps').textContent =
          data.gps_fix ? (data.gps_lat + ', ' + data.gps_lon) : 'No fix';

        document.getElementById('val-gps-fix').textContent = data.gps_fix ? '1' : '0';
        document.getElementById('val-sd').textContent = data.sd_status || '--';
        document.getElementById('val-log-state').textContent = data.logging ? 'Logging' : 'Idle';
        document.getElementById('val-samples').textContent =
          (data.samples != null) ? data.samples : 0;

        const pct = Math.max(0, Math.min(100, ((data.samples || 0) / 100) * 100));
        document.getElementById('log-progress').style.width = pct + '%';

        const now = new Date();
        document.getElementById('val-updated').textContent =
          now.toLocaleTimeString ? now.toLocaleTimeString() :
          (now.getHours() + ':' + now.getMinutes() + ':' + now.getSeconds());

        document.getElementById('action-status').textContent = 'OK';

      } catch (e) {
        document.getElementById('action-status').textContent = 'Lost connection…';
      }
    }

    async function sendCommand(path, description) {
      document.getElementById('action-status').textContent = 'Sending…';
      try {
        const res = await fetch(path, { method: 'POST' });
        if (res.ok) {
          document.getElementById('action-status').textContent = description;
          fetchStatus();
        } else {
          document.getElementById('action-status').textContent = 'Error: ' + res.status;
        }
      } catch (e) {
        document.getElementById('action-status').textContent = 'Request failed';
      }
    }

    function downloadCsv() {
      window.location.href = '/download';
    }

    setInterval(fetchStatus, 1000);
    window.addEventListener('load', fetchStatus);
  </script>
</body>
</html>
)rawliteral";



static void handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}


static void appendNumberField(String &json, const char* name, float value, int decimals) {
    json += "\"";
    json += name;
    json += "\":";

    if (isnan(value) || isinf(value)) {
        json += "null";
    } else {
        json += String(value, decimals);
    }
    json += ",";
}

static void handleStatus() {
    String json = "{";

    appendNumberField(json, "temperature", dispData.temperature, 2);
    appendNumberField(json, "humidity",    dispData.humidity,    2);
    appendNumberField(json, "pressure",    dispData.pressure,    3);
    appendNumberField(json, "gas",         dispData.gas,         3);
    appendNumberField(json, "distance",    dispData.distance,    1);

    json += "\"gps_lat\":\"";
    json += dispData.gps_lat;
    json += "\",";

    json += "\"gps_lon\":\"";
    json += dispData.gps_lon;
    json += "\",";

    json += "\"gps_fix\":";
    json += (dispData.gps_fix ? "true" : "false");
    json += ",";

    json += "\"sd_status\":\"";
    json += dispData.sd_status;
    json += "\",";

    json += "\"logging\":";
    json += (loggingEnabled ? "true" : "false");
    json += ",";

    json += "\"samples\":";
    json += String(logCount);

    json += "}";

    server.send(200, "application/json", json);
}

// Responsible solely for "entering measurement mode", equivalent to pressing the BOOT key
static void handleStartMeasurement() {
    systemStarted = true;
    server.send(200, "application/json", "{\"ok\":true,\"action\":\"start_measure\"}");
}

// Commencing recording
static void handleStartLogging() {
    loggingEnabled = true;
    logCount = 0;
    dispData.sd_status = "Logging 0/100";
    server.send(200, "application/json", "{\"ok\":true,\"action\":\"start\"}");
}

// Stop recording
static void handleStopLogging() {
    loggingEnabled = false;
    dispData.sd_status = "Stopped";
    server.send(200, "application/json", "{\"ok\":true,\"action\":\"stop\"}");
}

// Clear the log.csv file from the SD card
static void handleClearLog() {
    clearLogFile();
    loggingEnabled = false;
    logCount = 0;
    dispData.sd_status = "Cleared";
    server.send(200, "application/json", "{\"ok\":true,\"action\":\"clear\"}");
}

// Download CSV (either /download or /data)
static void handleDownload() {
    if (!SD.exists("/log.csv")) {
        server.send(404, "text/plain", "log.csv not found");
        return;
    }

    File file = SD.open("/log.csv", FILE_READ);
    if (!file) {
        server.send(500, "text/plain", "Failed to open log.csv");
        return;
    }

    server.sendHeader("Content-Type", "text/csv");
    server.sendHeader("Content-Disposition", "attachment; filename=log.csv");
    server.sendHeader("Connection", "close");
    server.streamFile(file, "text/csv");
    file.close();
}

// 404: Directly redirects to the dashboard, achieving "any URL being hijacked to the dashboard".
static void handleNotFound() {
    server.send_P(200, "text/html", INDEX_HTML);
}


void startWebServer() {
    server.on("/", HTTP_GET, handleRoot);

    // Real-time status
    server.on("/api/status", HTTP_GET, handleStatus);

    // Enter measurement mode (equivalent to BOOT)
    server.on("/api/system/start", HTTP_POST, handleStartMeasurement);

    // Logging 控制
    server.on("/api/log/start", HTTP_POST, handleStartLogging);
    server.on("/api/log/stop",  HTTP_POST, handleStopLogging);
    server.on("/api/log/clear", HTTP_POST, handleClearLog);

    // Download CSV
    server.on("/download", HTTP_GET, handleDownload);
    server.on("/data",     HTTP_GET, handleDownload);

    server.onNotFound(handleNotFound);

    server.begin();
}


void startWiFiAP() {
    WiFi.mode(WIFI_AP);

    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASS);

    IPAddress ip = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(ip);

    dnsServer.start(DNS_PORT, "*", apIP);
    startWebServer();
}

void wifiServerLoop() {
    dnsServer.processNextRequest();
    server.handleClient();
}

