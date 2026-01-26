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
#include <WiFiClient.h>
#include <DHT.h>
#include <time.h>
#include <TZ.h>
#include <LittleFS.h>
#include <config.h>

// const char* ssid = "SSID";      // Replace with your WiFi SSID
// const char* password = "password"; // Replace with your WiFi password

// TCP Server Configuration
// const char* tcpServer = "192.168.1.1";
const int tcpPort = 2003;

// NTP Configuration
#define MY_TZ "EST5EDT,M3.2.0,M11.1.0" // Eastern Time Zone (change as needed)
const char* ntpServer = "pool.ntp.org";

// DHT Sensor Configuration
#define DHTPIN 2        // GPIO2 (D4 on NodeMCU)
#define DHTTYPE DHT11   // DHT11 sensor

DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);
WiFiClient tcpClient;

// Global variables to store sensor readings
float temperatureF = 0;
float humidity = 0;
unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 3000; // Read sensor every 3 seconds (DHT11 needs longer intervals)

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println();
  Serial.println("ESP8266 Temperature & Humidity Monitor");
  Serial.println("======================================");
  
  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed!");
  } else {
    Serial.println("LittleFS mounted successfully");
  }
  
  // Initialize DHT sensor
  dht.begin();
  
  // Configure WiFi for 2.4GHz (STATION mode, no 11n)
  WiFi.mode(WIFI_STA);
  WiFi.setPhyMode(WIFI_PHY_MODE_11G); // Force 2.4GHz only (802.11g)
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  
  // Connect to WiFi with retry logic
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  unsigned long startAttemptTime = millis();
  const unsigned long wifiTimeout = 15000; // 15 seconds (increased from 10)
  int connectionAttempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
    delay(500);
    Serial.print(".");
    connectionAttempts++;
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.println(WiFi.RSSI());
  } else {
    Serial.println("WiFi connection failed!");
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());
    Serial.println("Status codes: 0=IDLE, 1=NO_SSID, 2=SCAN_FAILED, 3=CONNECTED, 4=CONNECT_FAILED, 5=DISCONNECTED, 6=WRONG_PASSWORD");
    Serial.println("Troubleshooting: Verify SSID/password, ensure router is 2.4GHz compatible");
  }
  
  // Configure time with NTP
  Serial.println("Synchronizing time with NTP server...");
  configTime(MY_TZ, ntpServer);
  
  // Wait for time to sync (optional but recommended)
  time_t now = time(nullptr);
  int retries = 0;
  while (now < 24 * 3600 && retries < 20) {  // Wait until time is set (after Jan 1, 1970)
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    retries++;
  }
  Serial.println();
  if (now > 24 * 3600) {
    Serial.println("Time synchronized!");
    Serial.println(getFormattedTime());
  } else {
    Serial.println("Time sync failed, will retry in background");
  }
  
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
  
  // Send data to TCP server
  // if graphiteServerAddress is undefined or empty, skip sending
  sendDataToGraphite();
}

// Get formatted timestamp
String getFormattedTime() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  return String(buffer);
}

// Get Unix timestamp
long getUnixTimestamp() {
  return time(nullptr);
}

void sendDataToGraphite() {
  // Skip sending if no Graphite endpoint is configured
  if (graphiteServerAddress == nullptr || graphiteServerAddress[0] == '\0') {
    return;
  }

  if (tcpClient.connect(graphiteServerAddress, tcpPort)) {
    // Get timestamp
    long timestamp = getUnixTimestamp();
    String timeStr = getFormattedTime();
    
    // Create strings with temperature and humidity (using Unix timestamp for Graphite format)
    String temp = "local.temp " + String(temperatureF, 1) + " " + String(timestamp);
    String hum = "local.humidity " + String(humidity, 1) + " " + String(timestamp);
    
    tcpClient.println(temp);
    tcpClient.println(hum);
    tcpClient.stop();
    
    Serial.print("[");
    Serial.print(timeStr);
    Serial.print("] Sent to TCP server: ");
    Serial.print("Temp=");
    Serial.print(temperatureF, 1);
    Serial.print("F, Humidity=");
    Serial.print(humidity, 1);
    Serial.println("%");
  } else {
    Serial.println("Failed to connect to TCP server");
  }
}

// Handler for the root page - read HTML from file
void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    Serial.println("Failed to open index.html");
    server.send(404, "text/html", "<h1>404 - index.html not found</h1><p>Please upload the filesystem image using PlatformIO's 'Upload Filesystem Image' task.");
    return;
  }
  
  server.streamFile(file, "text/html");
  file.close();
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
  message += "URI: ";
  message += server.uri();
  message += "\n";
  message += "Method: ";
  message += (server.method() == HTTP_GET ? "GET" : "POST");
  message += "\n";
  
  server.send(404, "text/plain", message);
}
