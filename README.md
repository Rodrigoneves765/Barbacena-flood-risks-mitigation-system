# Barbacena-flood-risks-mitigation-system
An automated mechanical prototype designed to mitigate a 50-year-old urban flooding issue in Barbacena, Brazil, using ultrasonic sensor systems, Arduino, and IoT.

# Smart Storm Drain System (Pontilhão Inteligente) - v1.1

An industrial-grade IoT and edge-computing telemetry station engineered to mitigate flash flood disasters in urban critical zones. This project utilizes acoustic time-of-flight physics, real-time digital signal processing (DSP), and ultra-low-power long-range radio-frequency (LoRa) networks to monitor storm drain capacity dynamically.

Developed as part of an advanced embedded systems and civil engineering research portfolio targeting top-tier international STEM applications.

---

## Project Parameters & Location
* Research & Testing Site: Barbacena, Minas Gerais, Brazil
* Hardware Target: Heltec WiFi LoRa 32 (V3) [ESP32-S3 Architecture]
* Acoustic Sensor: JSN-SR04T (Waterproof Industrial Ultrasonic Transducer)
* Calibration Baseline: 46 cm Vertical Profile Hydraulic Simulator

---

## System Architecture & Advanced Features

The firmware (pontilhao_sensor_v1_1.ino) transitions away from basic hobbyist loops by integrating mathematical filtering and safety layers designed for harsh real-world urban infrastructure environments:

### 1. Digital Signal Processing (DSP) via Median Filtering
Acoustic readings inside closed storm drains are highly prone to multi-path reflections, residual echo noise, and water splashing anomalies. To prevent false positives, the system triggers N=5 rapid acoustic bursts per cycle with a 60ms anti-residual interval. The raw data points are injected into a local buffer array, programmatically structured using an internal Insertion Sort routine, and the Statistical Median is extracted. This guarantees zero ripple reporting.

### 2. High-Impedance Fail-Safe Watchdog
If a flash flood rises past the sensor's physical constraints, the ultrasonic wave enters the transducer's 20-25cm physical blind zone. In typical scripts, this causes timeout drops (returning raw zero or maximum distance values), which would falsely report a dry state. Our edge state machine monitors a dedicated sequential failure accumulator. If consecutive echoes drop while the previous state was critical, the kernel overrides the hardware gap, forces a persistent EMERGENCY state, and prints diagnostic warning flags.

### 3. Integrated Power Rail Shifting
To optimize thermal limits and power consumption, the firmware controls the onboard Vext power rail via safe software execution switching (vextLigar()), gating the internal I2C SSD1306 OLED graphical interface bus (SDA GPIO 17 / SCL GPIO 18) dynamically.

---

## Telemetric Calibration Matrix (46 cm Baseline)

The mathematical threshold gates are mapped linearly inside the C++ script to isolate the volumetric water columns without causing acoustic wave clipping:

| Target Distance (Sensor to Water) | Hydraulic Column Height | System Threat State | Local Graphic Output |
|:---|:---|:---|:---|
| 46cm to 38cm | 0cm to 8cm | NORMAL | Active Stream (Live Metric) |
| 38cm to 30cm | 8cm to 16cm | WARNING (ATENCAO) | Local Metric Buffer Logging |
| 30cm to 24cm | 16cm to 22cm | ALERT (ALERTA) | High Threat Flash Flagging |
| <= 24cm (or No Echo) | >= 22cm (Blind Boundary) | EMERGENCY | Critical Flood Overwrite |

---

## Hardware Safety Constraint Warning
CAUTION: Electrical I/O Level Matching Requirement. The industrial JSN-SR04T transceiver module operates on a 5V power rail and outputs a 5V logic pulse on its Echo pin. The ESP32-S3 processor on the Heltec V3 development board is NOT 5V tolerant and accepts a maximum of 3.3V on its GPIO array. Direct connection will cause terminal silicon degradation. A precision voltage divider network (e.g., 1 kΩ resistor in series coupled with a 2 kΩ resistor tied to Ground) or an active logic-level shifter must be hardwired onto the Echo line before firing the microcontroller unit.

---

## Repository Structure
* /src: Contains the production C++ firmware core (pontilhao_sensor_v1_1.ino).
* /sim: Schematics and export models mapped during the active software validation phase on Autodesk Tinkercad.
* /docs: Serial data capture logging history and physical calibration records mapped in Barbacena, MG.

---

## Compilation & Deployment Instructions
1. Open the Arduino IDE (v2.3.10+).
2. Mount the official Heltec ESP32 Board Core Index (https://heltec.cn) inside the Preferences panel.
3. Install the universal SSD1306Wire.h graphics driver via the automated Library Manager.
4. Select board target: Heltec WiFi LoRa 32(V3).
5. Allocate the recognized communication serial bridge interface port (Silicon Labs CP210x Driver).
6. Execute the Upload sequence. If the host controller enters a serial handshake lockout buffer (Connecting... failed), hold the physical PRG button on the Heltec V3 board for 3 seconds to force manual DFU bootloader deployment.
