# 🔷 AIoT Smart Home Automation System (ESP32 + MQTT + PWA)

## 📌 Project Overview

This project is an **AIoT-based Smart Home Automation System** developed using an **ESP32 microcontroller**, **PIR motion sensor**, **LDR sensor**, and a **web-based Progressive Web App (PWA) dashboard**.

The system combines **IoT (data collection + remote control)** with **AI-based decision logic**, making it an **AIoT system** suitable for **Industry 4.0 applications**.

---

## 🚀 Key Features

* 🌐 **WiFi-based control using ESP32**
* ☁️ **MQTT communication using HiveMQ Cloud**
* 📊 **Real-time dashboard monitoring**
* 🤖 **AI Mode (Automatic Decision Making)**
* 🎮 **Manual Mode (User Control)**
* 💡 **Automatic Light Control (PIR + LDR based)**
* ⏱️ **Smart Delay Logic (3 sec auto OFF)**
* ⚡ **Noise & False Trigger Handling**
* 📱 **Installable Web App (PWA)**

---

## 🧠 AI Logic (Core Working)

The system uses simple but effective AI decision rules:

| Condition                 | Action    |
| ------------------------- | --------- |
| Day (LDR detects light)   | Light OFF |
| Night + Motion            | Light ON  |
| Night + No Motion (3 sec) | Light OFF |

👉 Additional intelligence:

* Prevents false triggering from PIR sensor
* Handles relay noise and sensor instability
* Uses delay + state memory for stable output

---

## ⚙️ Working Principle

1. ESP32 reads:

   * PIR sensor (motion detection)
   * LDR sensor (day/night detection)

2. Data is processed using AI logic

3. Based on condition:

   * Relay (Light) is controlled automatically

4. System sends:

   * Sensor data → Dashboard
   * Device status → Dashboard

5. User can:

   * Switch between AI Mode & Manual Mode
   * Control devices manually from dashboard

---

## 🧩 Components Used

* ESP32 Development Board
* PIR Motion Sensor (HC-SR501)
* LDR Sensor
* Relay Module
* LEDs (for light and fan simulation)
* Resistors
* Jumper wires
* Power supply

---

## 🔌 Pin Configuration

| Component        | ESP32 Pin |
| ---------------- | --------- |
| Relay            | GPIO 19   |
| LED 1 (Light)    | GPIO 21   |
| LED 2 (Light)    | GPIO 22   |
| LED 3 (Fan Demo) | GPIO 23   |
| LED 4 (Fan Demo) | GPIO 25   |
| PIR Sensor       | GPIO 27   |
| LDR Sensor       | GPIO 34   |

---

## 🌐 MQTT Configuration

* Broker: **HiveMQ Cloud**
* Protocol: **MQTT over TLS (port 8883)**

### Topics Used:

```text
home/control   → Receive commands
home/status    → Device status updates
home/sensors   → Sensor data (PIR + LDR)
```

---

## 💻 Dashboard (PWA)

The dashboard is built using:

* HTML
* CSS
* JavaScript
* MQTT WebSocket

### Features:

* Real-time sensor data display
* Manual control buttons
* AI mode toggle
* Device status logs
* Installable as mobile app (PWA)

---

## 📱 PWA Features

* Installable on mobile/laptop
* Works like native app
* Offline support (basic UI)
* Fast loading using service worker

---

## 📁 Project Structure

```text
AIoT-Smart-Home-Automation-PWA/
│
├── README.md
├── LICENSE
├── .gitignore
│
├── esp32-code/
│   └── AIoT_HomeAutomation_ESP32.ino
│
├── dashboard/
│   ├── index.html
│   ├── manifest.json
│   ├── service-worker.js
│   ├── icon-192.png
│   └── icon-512.png
```

---

## 🛠️ How to Run the Project

### 1️⃣ ESP32 Setup

1. Open Arduino IDE
2. Install ESP32 board package
3. Open file:

   ```text
   esp32-code/AIoT_HomeAutomation_ESP32.ino
   ```
4. Update:

   * WiFi SSID & Password
   * MQTT Username & Password
5. Upload code to ESP32

---

### 2️⃣ Dashboard Setup

1. Open `dashboard/index.html` using:

   * Live Server (VS Code) OR
   * Browser (localhost)

2. Enter:

   * MQTT credentials
   * Connect

---

### 3️⃣ GitHub Hosting (Optional)

1. Upload project to GitHub
2. Go to:

   ```text
   Settings → Pages
   ```
3. Select:

   ```text
   Branch: main
   Folder: /dashboard
   ```
4. Access your app online

---

## ⚠️ Important Notes

* Wait **30 seconds** after powering PIR sensor
* Adjust PIR knobs:

  * TIME → minimum
  * SENS → medium/low
* Keep PIR away from relay wires (to avoid noise)

---

## 🔬 AIoT Justification

This project qualifies as **AIoT** because:

* Uses **sensor data (IoT)**
* Applies **decision logic (AI behavior)**
* Performs **automatic control without user input**

👉 Difference:

| IoT             | AIoT               |
| --------------- | ------------------ |
| Manual control  | Automatic decision |
| Data monitoring | Smart response     |
| User dependent  | Self-operating     |

---

## 📈 Future Scope

* Voice control (Google Assistant / Alexa)
* Mobile app (APK)
* Cloud data logging
* Machine learning-based prediction
* Energy optimization

---

## 👨‍🎓 Author

**Atharv Bunde**
Diploma in Mechatronics Engineering

---

## 📜 License

This project is for educational purposes.
