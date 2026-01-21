/* 
 * Example Configuration File
 * Copy this file and rename it to match your setup
 * 
 * This file shows the configuration options available in iot_temp_sensor.ino
 * Modify the values in the main .ino file according to your setup
 */

// WiFi Configuration Example
// Replace these values in the main iot_temp_sensor.ino file

// const char* ssid = "MyHomeNetwork";       // Your WiFi network name
// const char* password = "MySecurePass123"; // Your WiFi password

// Sensor Configuration
// Choose your sensor type by uncommenting the appropriate line:

// #define DHTTYPE DHT22   // Use this for DHT22 (AM2302) sensor
// #define DHTTYPE DHT11   // Use this for DHT11 sensor

// Pin Configuration
// Default pin is D4 (GPIO2) on NodeMCU
// Change if you've wired the sensor to a different pin:

// #define DHTPIN 2        // GPIO2 (D4 on NodeMCU)

// Advanced Settings

// Sensor read interval (milliseconds)
// const unsigned long READ_INTERVAL = 2000;

// Web server port (default: 80 for HTTP)
// You can change this if port 80 is already in use
// const int SERVER_PORT = 80;
