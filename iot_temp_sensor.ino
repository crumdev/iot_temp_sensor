/*
 * ESP8266 NodeMCU Temperature and Humidity Monitor
 * 
 * This sketch reads temperature and humidity from a DHT sensor
 * and provides HTTP endpoints to view and retrieve the data.
 * 
 * Hardware:
 * - ESP8266 NodeMCU board
 * - DHT11 or DHT22 temperature and humidity sensor
 * - Connect DHT data pin to D4 (GPIO2)
 * - Connect DHT VCC to 3.3V
 * - Connect DHT GND to GND
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// WiFi Configuration
const char* ssid = "YOUR_WIFI_SSID";      // Replace with your WiFi SSID
const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your WiFi password

// DHT Sensor Configuration
#define DHTPIN 2        // GPIO2 (D4 on NodeMCU)
#define DHTTYPE DHT22   // DHT 22 (AM2302), change to DHT11 if using DHT11

DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);

// Global variables to store sensor readings
float temperatureF = 0;
float humidity = 0;
unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 2000; // Read sensor every 2 seconds

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println();
  Serial.println("ESP8266 Temperature & Humidity Monitor");
  Serial.println("======================================");
  
  // Initialize DHT sensor
  dht.begin();
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Configure HTTP server routes
  server.on("/", handleRoot);
  server.on("/api/data", handleApiData);
  server.onNotFound(handleNotFound);
  
  // Start HTTP server
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Access the web interface at: http://" + WiFi.localIP().toString());
}

void loop() {
  server.handleClient();
  
  // Read sensor data at specified interval
  if (millis() - lastReadTime >= READ_INTERVAL) {
    readSensorData();
    lastReadTime = millis();
  }
}

void readSensorData() {
  // Read humidity
  float h = dht.readHumidity();
  
  // Read temperature in Celsius
  float t = dht.readTemperature();
  
  // Check if any reads failed
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Convert temperature to Fahrenheit
  temperatureF = (t * 9.0 / 5.0) + 32.0;
  humidity = h;
  
  // Print to serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperatureF);
  Serial.print("°F  Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
}

// Handler for the root page (graphical display)
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Temperature & Humidity Monitor</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); margin: 0; padding: 20px; min-height: 100vh; display: flex; justify-content: center; align-items: center; }";
  html += ".container { background: white; border-radius: 20px; padding: 40px; box-shadow: 0 20px 60px rgba(0,0,0,0.3); max-width: 600px; width: 100%; }";
  html += "h1 { color: #333; text-align: center; margin-bottom: 40px; font-size: 2em; }";
  html += ".sensor-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; margin-bottom: 30px; }";
  html += ".sensor-card { background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%); border-radius: 15px; padding: 30px; text-align: center; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }";
  html += ".sensor-card.temp { background: linear-gradient(135deg, #fa709a 0%, #fee140 100%); }";
  html += ".sensor-card.humidity { background: linear-gradient(135deg, #30cfd0 0%, #330867 100%); }";
  html += ".sensor-icon { font-size: 3em; margin-bottom: 10px; }";
  html += ".sensor-value { font-size: 3em; font-weight: bold; color: #fff; text-shadow: 2px 2px 4px rgba(0,0,0,0.2); }";
  html += ".sensor-unit { font-size: 1.2em; color: #fff; margin-top: 5px; opacity: 0.9; }";
  html += ".sensor-label { font-size: 1.2em; color: #fff; margin-top: 10px; font-weight: 500; }";
  html += ".info { background: #f0f4f8; border-radius: 10px; padding: 20px; margin-top: 20px; }";
  html += ".info h3 { margin-top: 0; color: #555; }";
  html += ".info p { margin: 5px 0; color: #666; }";
  html += ".refresh-btn { background: #667eea; color: white; border: none; padding: 12px 30px; border-radius: 25px; font-size: 1em; cursor: pointer; margin-top: 20px; width: 100%; transition: background 0.3s; }";
  html += ".refresh-btn:hover { background: #5568d3; }";
  html += ".auto-refresh { text-align: center; color: #666; margin-top: 10px; font-size: 0.9em; }";
  html += "@media (max-width: 600px) { .sensor-grid { grid-template-columns: 1fr; } }";
  html += "</style>";
  html += "<script>";
  html += "function refreshData() {";
  html += "  fetch('/api/data')";
  html += "    .then(response => response.json())";
  html += "    .then(data => {";
  html += "      document.getElementById('temp').innerText = data.temperature.toFixed(1);";
  html += "      document.getElementById('humidity').innerText = data.humidity.toFixed(1);";
  html += "      document.getElementById('lastUpdate').innerText = new Date().toLocaleTimeString();";
  html += "    })";
  html += "    .catch(error => console.error('Error:', error));";
  html += "}";
  html += "setInterval(refreshData, 5000);"; // Auto-refresh every 5 seconds
  html += "</script>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>🌡️ Temperature & Humidity Monitor</h1>";
  html += "<div class='sensor-grid'>";
  
  // Temperature card
  html += "<div class='sensor-card temp'>";
  html += "<div class='sensor-icon'>🌡️</div>";
  html += "<div class='sensor-value' id='temp'>" + String(temperatureF, 1) + "</div>";
  html += "<div class='sensor-unit'>°F</div>";
  html += "<div class='sensor-label'>Temperature</div>";
  html += "</div>";
  
  // Humidity card
  html += "<div class='sensor-card humidity'>";
  html += "<div class='sensor-icon'>💧</div>";
  html += "<div class='sensor-value' id='humidity'>" + String(humidity, 1) + "</div>";
  html += "<div class='sensor-unit'>%</div>";
  html += "<div class='sensor-label'>Humidity</div>";
  html += "</div>";
  
  html += "</div>";
  
  // Info section
  html += "<div class='info'>";
  html += "<h3>📊 System Information</h3>";
  html += "<p><strong>Device:</strong> ESP8266 NodeMCU</p>";
  html += "<p><strong>Sensor:</strong> DHT22</p>";
  html += "<p><strong>API Endpoint:</strong> <a href='/api/data'>/api/data</a></p>";
  html += "<p><strong>Last Update:</strong> <span id='lastUpdate'>" + String(millis() / 1000) + "s</span></p>";
  html += "</div>";
  
  html += "<button class='refresh-btn' onclick='refreshData()'>🔄 Refresh Now</button>";
  html += "<div class='auto-refresh'>Auto-refreshes every 5 seconds</div>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Handler for the API endpoint (JSON data)
void handleApiData() {
  String json = "{";
  json += "\"temperature\":" + String(temperatureF, 2) + ",";
  json += "\"humidity\":" + String(humidity, 2) + ",";
  json += "\"unit_temperature\":\"F\",";
  json += "\"unit_humidity\":\"%\",";
  json += "\"timestamp\":" + String(millis()) + ",";
  json += "\"device\":\"ESP8266\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

// Handler for 404 errors
void handleNotFound() {
  String message = "404 - Not Found\n\n";
  message += "URI: " + server.uri() + "\n";
  message += "Method: " + (server.method() == HTTP_GET ? "GET" : "POST") + "\n";
  
  server.send(404, "text/plain", message);
}
