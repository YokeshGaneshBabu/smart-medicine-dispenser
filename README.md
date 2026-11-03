# Smart Medicine Dispenser

An ESP32-based smart medicine dispenser designed to improve medication adherence through automated compartment access, audio reminders, IR-based dose detection, and real-time web monitoring.

## Overview

The system guides users through four daily medication slots using automated servo-controlled compartments, visual indicators, and voice prompts in their native language.

It combines embedded control, real-time clock functionality, sensor-based detection, and a lightweight web dashboard to provide an accessible, standalone medication management solution.

## Features

- **Automated Dispensing:** Servo motors control access to individual medicine compartments.
- **Real-Time Clock:** DS3231 RTC provides date and time information.
- **Dose Detection:** IR sensors detect hand entry and register medication-taking events.
- **Audio Guidance:** DFPlayer Mini plays prerecorded voice reminders.
- **Visual Indicators:** LEDs distinguish before-food and after-food medication.
- **LCD Interface:** Displays system status, time, and dose information.
- **Web Dashboard:** Monitor medication status through a browser over Wi-Fi.
- **Standalone Operation:** Battery-powered hardware with USB Type-C charging support.

## System Architecture

| Component | Function |
|---|---|
| ESP32 DevKit | Main controller |
| DS3231 RTC | Real-time clock |
| Servo Motors (3) | Compartment actuation |
| IR Sensors (3) | Hand detection |
| DFPlayer Mini | Audio playback |
| Speaker (3W) | Voice reminders |
| LCD (I2C) | User interface |
| LEDs (2) | Dose indication |
| Li-ion Batteries | Power supply |
| TP4056 | Battery charging |
| LM2596 | Voltage regulation |

## Medication Workflow

The firmware implements four medication slots:

1. Morning dose
2. Afternoon dose (Before Food)
3. Afternoon dose (After Food)
4. Night dose

Each dispensing operation follows this sequence:

1. The corresponding servo opens the compartment.
2. The LCD displays the medication reminder.
3. The audio module plays a voice prompt.
4. The IR sensor detects hand entry.
5. The medication status is updated.
6. The compartment closes after the detection sequence or timeout.

A software-based delay prevents repeated detection during a single hand-entry event.

## Web Dashboard

The ESP32 hosts a lightweight web interface accessible through its local IP address.

The dashboard provides:

- Total medication slots
- Doses marked as taken
- Doses not yet marked as taken
- Individual medication slot status
- Current date and time

The interface automatically refreshes every 10 seconds.

## Software and Libraries

- Embedded C / Arduino Framework
- ESP32 Arduino Core
- `Wire.h`
- `RTClib.h`
- `LiquidCrystal_I2C.h`
- `ESP32Servo.h`
- `DFRobotDFPlayerMini.h`
- `HardwareSerial.h`
- `WiFi.h`
- `WebServer.h`

## Getting Started

### 1. Hardware Setup

Connect the ESP32 to the RTC, LCD, servo motors, IR sensors, LEDs, and DFPlayer Mini according to the pin configuration in the firmware.

### 2. Software Setup

Install the Arduino IDE and the required ESP32 board support package.

Install the libraries listed above through the Arduino Library Manager.

### 3. Configure Wi-Fi

Update the Wi-Fi credentials in the firmware:

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

### 4. Upload Firmware

1. Connect the ESP32 to your computer.
2. Select the appropriate ESP32 board and COM port.
3. Upload `smart-medicine-dispenser.ino`.
4. Open the Serial Monitor at 115200 baud.

### 5. Access the Dashboard

After successful Wi-Fi connection, obtain the ESP32's local IP address from the Serial Monitor and open it in a browser:

```text
http://ESP32_LOCAL_IP
```


## Future Improvements

- Configurable medication schedules through the web interface.
- Persistent medication history and dose logging.
- Low-battery alerts and power optimization.
- Missed-dose notifications.
- Improved pill-level detection and inventory tracking.
- Remote monitoring and caregiver notifications.

---

*Developed as an embedded systems project at VIT Chennai.*
