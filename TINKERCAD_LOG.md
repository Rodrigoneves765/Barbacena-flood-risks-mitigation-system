# Virtual Prototyping & Simulation Log (Tinkercad) - v1.1

A software-in-the-loop validation phase for the Smart Storm Drain System (Pontilhão Inteligente). Before connecting the firmware to the physical Heltec V3 and the JSN-SR04T, the v1.1 state machine was tested inside Autodesk Tinkercad to verify the median filter, the failure watchdog, and the threshold gates in a safe virtual environment.

**Log date:** October 6, 2026 (08:35 PM)

---

## Simulation Parameters
* Simulation Platform: Autodesk Tinkercad Circuits
* Microcontroller Target: Arduino Uno R3 (simulation stand-in for the Heltec V3)
* Acoustic Sensor: HC-SR04 (simulation stand-in for the JSN-SR04T)
* Firmware Under Test: pontilhao_sensor_v1_1.ino (Simulation Mode)
* Pin Mapping: TRIG on GPIO 12, ECHO on GPIO 13
* Calibration Baseline: 46 cm Vertical Profile Hydraulic Simulator

---

## Circuit Description

The 4-pin ultrasonic module (VCC, TRIG, ECHO, GND) is plugged into the upper rail of the breadboard. The raw 5V Echo pulse is routed straight into Column 14, which acts as the input of a 5V to 3.3V voltage divider built with a 1 kΩ resistor (R1) and a 2 kΩ resistor. The divider isolates a safe 3.33V logic return junction, reproducing the protection network that the physical Heltec V3 build will require.

---

## Debugging Report: Persistent SENSOR FALHOU State

| Stage | Description |
|:---|:---|
| Symptom | The Serial Monitor locked into the `SENSOR FALHOU` fail-safe state right after starting the simulation |
| Diagnosis | Wave-propagation timeout. The virtual HC-SR04 starts with its target object at 100 cm by default |
| Root Cause | 100 cm exceeds the firmware limits (`TIMEOUT_ECHO_US` = 6000 μs and `PROFUNDIDADE_TOTAL` = 46 cm), so the echo never returns inside the allowed window and the failure routine triggers |
| Fix | Manually slid the interactive target down to 44.7 cm on the simulation canvas |
| Result | The valid reading cleared the failure counter, the loop synchronized, and the monitor streamed a steady `Status: NORMAL` |

---

## Visual Documentation

### 1. Hardware Setup

<img width="1600" height="781" alt="Figure 15" src="https://github.com/user-attachments/assets/6683c6fc-ea06-46af-a4b0-e9e7e44fd473" />

*Figure 15: First setup of the hardware in Autodesk Tinkercad. A 1k ohm resistor (R1) is placed vertically across the middle gap of the breadboard, as the first part of the 3.3V voltage divider.*

<img width="1600" height="787" alt="Figure 16" src="https://github.com/user-attachments/assets/a4164f3d-f988-455a-8d6f-f83b24eb130f" />

*Figure 16: Layout showing how the signals are wired. The 4 pin ultrasonic sensor (VCC, TRIG, ECHO, GND) is plugged into the top part of the breadboard, and the raw 5V Echo signal goes straight into column 14, which is the input of the voltage divider.*

### 2. Fault Condition

<img width="1600" height="783" alt="Figure 17" src="https://github.com/user-attachments/assets/69ca0a55-2eca-4f18-821b-26ace985e8dd" />

*Figure 17: The Arduino connected and running firmware v1.1. The grounds are all connected correctly, but the Serial Monitor starts out showing "SENSOR FALHOU" because of a timeout in the echo reading.*

<img width="1600" height="918" alt="Figure 18" src="https://github.com/user-attachments/assets/02dee4ea-b0f9-4661-acdb-494ce1d9e554" />

*Figure 18: Looking at what caused the error. The simulation screen shows the sensor's default target distance of 100 cm, which is more than the code can handle in its time window, so the failure routine kicks in.*

### 3. Validated Operation

<img width="1600" height="787" alt="Figure 19" src="https://github.com/user-attachments/assets/7163edd4-5f4e-4b5d-b732-45c7bf071628" />

*Figure 19: The loop working and the status logic confirmed. After sliding the target down to 17.6 inches (44.7 cm), the failure counter resets right away, the live readings come back, and the Serial Monitor shows "STATUS: NORMAL".*

---

## Expected State Behavior (46 cm Baseline)

Moving the virtual target between these distances is the quickest way to check every state of the machine:

| Target Distance (Sensor to Water) | System Threat State |
|:---|:---|
| 46cm to 38cm | NORMAL |
| 38cm to 30cm | WARNING (ATENCAO) |
| 30cm to 24cm | ALERT (ALERTA) |
| <= 24cm (or No Echo after a critical state) | EMERGENCY |

---

## Simulation Notes
* The Arduino preprocessor places auto-generated function prototypes above the `enum Estado` declaration. A function that returns `Estado` therefore causes the compile error `'Estado' does not name a type`. In the simulation build, `classificar()` returns `int` and is cast back with `(Estado)` inside `loop()`.
* The virtual sensor has an adjustable target distance that only appears while the simulation is running.

---

## Next Steps
- [ ] Export the finished schematics to the `/sim` folder in the repository
- [ ] Buy the real resistors (1 kΩ and 2 kΩ) in Barbacena tomorrow morning
- [ ] Build the physical voltage divider before connecting the Echo line to the Heltec V3 GPIO
