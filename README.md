# ESP8266 IoT Temperature & Humidity Monitor

Code for configuring ESP8266 NodeMCU project to monitor temperature and humidity with HTTP server interface.

## Features

- 🌡️ Real-time temperature monitoring (displayed in Fahrenheit)
- 💧 Real-time humidity monitoring (displayed as percentage)
- 🖥️ Beautiful web interface with graphical display
- 📊 REST API endpoint for data retrieval
- 🔄 Auto-refresh capability (updates every 5 seconds)
- 📱 Mobile-responsive design

## Hardware Requirements

- **ESP8266 NodeMCU** development board
- **DHT22** (AM2302) or **DHT11** temperature and humidity sensor
- Jumper wires
- USB cable for programming and power

### Wiring Diagram

```
DHT Sensor    ESP8266 NodeMCU
----------    ---------------
VCC       ->  3.3V
GND       ->  GND
DATA      ->  D4 (GPIO2)
```

## Software Requirements

### Arduino IDE Setup

1. **Install Arduino IDE** (version 1.8.x or higher)
   - Download from: https://www.arduino.cc/en/software

2. **Add ESP8266 Board Support**
   - Open Arduino IDE
   - Go to `File` → `Preferences`
   - Add to "Additional Board Manager URLs":
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Go to `Tools` → `Board` → `Boards Manager`
   - Search for "esp8266" and install "esp8266 by ESP8266 Community"

3. **Install Required Libraries**
   - Go to `Sketch` → `Include Library` → `Manage Libraries`
   - Install the following libraries:
     - **DHT sensor library** by Adafruit (also installs Adafruit Unified Sensor)
     - **ESP8266WiFi** (included with ESP8266 board package)
     - **ESP8266WebServer** (included with ESP8266 board package)

## Configuration

1. **Open the sketch** `iot_temp_sensor.ino` in Arduino IDE

2. **Configure WiFi credentials**:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";      // Replace with your WiFi SSID
   const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your WiFi password
   ```

3. **Configure sensor type** (if using DHT11 instead of DHT22):
   ```cpp
   #define DHTTYPE DHT11   // Change from DHT22 to DHT11
   ```

4. **Select board and port**:
   - Go to `Tools` → `Board` → `ESP8266 Boards` → `NodeMCU 1.0 (ESP-12E Module)`
   - Go to `Tools` → `Port` → Select your ESP8266's COM port

5. **Upload the sketch**:
   - Click the Upload button (→) or press `Ctrl+U`
   - Wait for compilation and upload to complete

## Usage

### Web Interface

1. After uploading, open the Serial Monitor (`Tools` → `Serial Monitor`)
2. Set baud rate to **115200**
3. Wait for the device to connect to WiFi
4. Note the IP address displayed (e.g., `192.168.1.100`)
5. Open a web browser and navigate to: `http://[IP_ADDRESS]`
6. View the graphical display of temperature and humidity

The web interface features:
- Large, easy-to-read temperature and humidity displays
- Beautiful gradient backgrounds
- Auto-refresh every 5 seconds
- Manual refresh button
- System information panel
- Mobile-responsive design

### API Endpoint

For integration with timeseries databases or other applications:

**Endpoint**: `http://[IP_ADDRESS]/api/data`

**Method**: GET

**Response** (JSON):
```json
{
  "temperature": 72.50,
  "humidity": 45.30,
  "unit_temperature": "F",
  "unit_humidity": "%",
  "timestamp": 12345678,
  "device": "ESP8266"
}
```

**Example with curl**:
```bash
curl http://192.168.1.100/api/data
```

**Example with Python**:
```python
import requests
import json

response = requests.get('http://192.168.1.100/api/data')
data = response.json()
print(f"Temperature: {data['temperature']}°F")
print(f"Humidity: {data['humidity']}%")
```

### Integration with Timeseries Databases

#### InfluxDB Example
```python
from influxdb import InfluxDBClient
import requests
import time

client = InfluxDBClient(host='localhost', port=8086, database='sensors')

while True:
    response = requests.get('http://192.168.1.100/api/data')
    data = response.json()
    
    json_body = [{
        "measurement": "climate",
        "tags": {"device": "basement_sensor"},
        "fields": {
            "temperature": data['temperature'],
            "humidity": data['humidity']
        }
    }]
    
    client.write_points(json_body)
    time.sleep(60)  # Poll every 60 seconds
```

#### Prometheus Example
You can scrape the metrics using a custom exporter that polls the `/api/data` endpoint.

## Troubleshooting

### WiFi Connection Issues
- Verify SSID and password are correct
- Check that 2.4GHz WiFi is enabled (ESP8266 doesn't support 5GHz)
- Ensure the WiFi network is reachable from the device location

### Sensor Reading Errors
- Check wiring connections
- Verify the correct DHT sensor type is configured (DHT11 vs DHT22)
- Ensure the sensor has proper power supply
- Try adding a 10kΩ pull-up resistor between DATA and VCC pins

### Upload Issues
- Select the correct board: NodeMCU 1.0 (ESP-12E Module)
- Check the correct COM port is selected
- Press the FLASH button on NodeMCU during upload if needed
- Reduce upload speed: `Tools` → `Upload Speed` → `115200`

## Serial Monitor Output

When running, you should see output like:
```
ESP8266 Temperature & Humidity Monitor
======================================
Connecting to WiFi: MyWiFiNetwork
.....
WiFi connected!
IP address: 192.168.1.100
HTTP server started
Access the web interface at: http://192.168.1.100
Temperature: 72.5°F  Humidity: 45.3%
Temperature: 72.6°F  Humidity: 45.2%
```

## Technical Specifications

- **Microcontroller**: ESP8266 (80/160 MHz)
- **WiFi**: 802.11 b/g/n (2.4 GHz)
- **Operating Voltage**: 3.3V
- **DHT22 Accuracy**: ±0.5°C temperature, ±2-5% humidity
- **DHT11 Accuracy**: ±2°C temperature, ±5% humidity
- **Update Rate**: Sensor readings every 2 seconds
- **Web UI Refresh**: Every 5 seconds (automatic)

## License

This project is open source and available for educational and personal use.

## Contributing

Contributions are welcome! Feel free to submit issues or pull requests.
