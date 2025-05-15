# 🔐 esp8266-lock-portal

A simple Wi-Fi-based ESP8266 web portal to control a relay for unlocking a locker or triggering any electronic mechanism — with cooldown protection and a clean user interface.

## 🚀 Features

- Creates a Wi-Fi hotspot with a captive portal
- Web interface with "OPEN" button
- URL command support via `/cmd?val=ACTIVATE`
- Cooldown mechanism (default 5 seconds) to avoid multiple triggers
- Optional relay feedback timer (non-blocking)
- Visual feedback via HTML + JavaScript
- Compatible with electric locks, solenoids, or other relay-controlled devices

---

## 🛠️ What You Need

- ESP8266 board (e.g., NodeMCU, Wemos D1 Mini)
- Relay module (5V or 3.3V compatible)
- Electric locker or solenoid
- USB cable + power supply
- Optional: LED for status/debug

### 🔌 Basic Wiring

| ESP8266 Pin | Connects To         |
|-------------|---------------------|
| D1 (GPIO5)  | Relay IN            |
| GND         | Relay GND           |
| 3V3 or 5V   | Relay VCC (check module) |
| Relay COM/NO | Electric Lock/Solenoid |

> ⚠️ The relay is triggered momentarily (~200ms) to simulate a button press and then deactivates to protect the device.

---

## 📲 How It Works

1. On boot, the ESP8266 creates a hotspot called `ESP-CMD-PORTAL`
2. User connects and is redirected to a control interface
3. Pressing "OPEN" or visiting `http://192.168.4.1/cmd?val=ACTIVATE` triggers the relay
4. If triggered too soon, a cooldown message is shown

---

## 🧪 Example Use Cases

- Triggering a smart locker
- Simulating a physical button press
- Activating a small device on demand

---

## ⚙️ Setup Instructions

1. Clone this repo:
```bash
git clone https://github.com/yourusername/esp8266-lock-portal.git

2. Open esp8266-lock-portal.ino in the Arduino IDE

3. Install the following libraries (if not already installed):
   - `ESP8266WiFi`
   - `ESP8266WebServer`
   - `DNSServer`

4. In the **Tools** menu:
   - Select your board: `NodeMCU 1.0 (ESP-12E Module)`
   - Choose the correct **port**

5. Click **Upload** to flash the firmware

6. Connect to the Wi-Fi hotspot `ESP-CMD-PORTAL`

7. Open your browser and navigate to `http://192.168.4.1`:
   - Use the **"OPEN"** button in the web interface  
   **or**
   - Send a direct command via URL:  
     `http://192.168.4.1/cmd?val=ACTIVATE`