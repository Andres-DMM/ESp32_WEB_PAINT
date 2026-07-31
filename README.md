# 🎨 ESP32 Web Paint — GC9A01A Round Display

> **Draw on a web application in your browser and watch it render instantly on a physical round TFT display!**

This project turns an ESP32 microcontroller and a GC9A01A round LCD display into a real-time, interactive mini-canvas. Connect your phone or desktop browser to the ESP32's local web server to draw in any color, adjust brush sizes, and clear the screen dynamically.

---

## ✨ Features

- **🌐 Self-Hosted Web Interface:** No external servers required—the ESP32 serves its own responsive drawing app.
- **📱 Touch & Mouse Support:** Smooth drawing mechanics using HTML5 Canvas with support for touch devices.
- **🎨 Full RGB Color Picker:** Choose any color and watch it convert seamlessly to RGB565 format for the TFT screen.
- **⚡ Batched Point Streaming:** Efficient JSON batching over HTTP POST minimizes latency and prevents web server memory congestion.
- **⭕ Round Display Friendly:** HTML5 Canvas visual boundary matches the GC9A01A's 240x240 circular form factor.

---

## 🛠️ Hardware Requirements

| Component | Description | Quantity |
| :--- | :--- | :---: |
| **ESP32 DevKit V1** | Standard 30-pin / 38-pin ESP32 board | 1 |
| **GC9A01A Display** | 1.28-inch 240x240 Round SPI TFT Module | 1 |
| **Jumper Wires** | Female-to-Female or Breadboard Wires | ~8 |
| **Micro-USB Cable** | Power & Programming | 1 |

---

## 🔌 Wiring Diagram

Connect your GC9A01A module to the ESP32 using the standard Hardware SPI pins:

| GC9A01A Pin | ESP32 Pin | Notes |
| :--- | :--- | :--- |
| **VCC** | `5V` (or `3V3`) | Power supply |
| **GND** | `GND` | Ground connection |
| **CS** | `GPIO 5` | Chip Select |
| **DC** | `GPIO 2` | Data / Command Control |
| **RST** | `GPIO 4` | Hardware Reset |
| **MOSI (SDA)** | `GPIO 23` | SPI Data Out |
| **SCK (SCL)** | `GPIO 18` | SPI Clock Line |
| **BL** | `3.3V` | Backlight control (Optional: PWM via GPIO 22) |

---

## 📚 Required Libraries

Install the following dependencies through the **Arduino IDE Library Manager** (`Ctrl+Shift+I` or `Cmd+Shift+I`):

1. **`Adafruit_GFX`** by *Adafruit*
2. **`Adafruit_GC9A01A`** by *Adafruit*
3. **`ArduinoJson`** (v6.x) by *Benoit Blanchon*

---

## 🚀 Getting Started

### 1. Configure WiFi Credentials
Open the code in Arduino IDE and update the network credentials with your local WiFi details:

```cpp
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";



### 2. Flash the Firmware
Select board: Tools > Board > ESP32 Arduino > ESP32 Dev Module.

Select the appropriate COM Port.

Hit Upload.

### 3. Connect & Paint
Open the Serial Monitor (115200 baud rate).

Wait for the ESP32 to connect to WiFi. The assigned IP address will display on both the Serial Monitor and the GC9A01A LCD.

Open any modern browser on a phone or desktop connected to the same WiFi network and navigate to:

Plaintext
http://<ESP32-IP-ADDRESS>
Start drawing!

📡 API Endpoint Overview
Endpoint	Method	Description
/	GET	Serves the HTML/CSS/JS frontend canvas app.
/draw	POST	Accepts JSON payload containing color, line width, and array of (x,y) coordinates.
/clear	POST	Clears the display screen with solid black background.
Payload Example (POST /draw)
JSON
{
  "color": "#ff0055",
  "size": 5,
  "points": [
    [120, 120],
    [121, 122],
    [125, 128]
  ]
}
### 📜 License
Distributed under the MIT License. Feel free to modify and use it in your own micro-controller projects!
