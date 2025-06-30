# HydraBand-Lite-Sense2Scale
TRL-8 Ready Hydration &amp; Heat Stress Monitoring Band – submitted for Sense2Scale 2025
**Submitted for Sense2Scale: India's TRL-8 Sensor Challenge (2025)**  
A low-cost, reusable, offline-capable hydration + heat stress monitoring band built for outdoor workers, students, athletes, and elderly individuals in high-heat conditions.

## Project Goals
- Detect dehydration using skin moisture (GSR), skin temperature (DS18B20), and ambient humidity (DHT22)
- Detect heat stress using combined ambient heat index + physiological response
- Offline alerts via LED and buzzer, no app or literacy required
- Real-time logging for TRL-8 readiness
- Ready to swap with Indian sensor chips

## Sensor Stack

| Sensor       | Function                          |
|--------------|-----------------------------------|
| GSR Module   | Skin conductance = hydration risk |
| DS18B20      | Skin surface temperature          |
| DHT22        | Ambient temperature & humidity    |
| ESP32        | Logic control + OTA capable       |
| LED/Buzzer   | On-device alert system            |

## Heat Stress Detection Logic

HydraBand Lite detects early heat stress by combining:
- Ambient temperature > 35°C
- Humidity < 40%
- Skin temp rising (>36°C)
- GSR value < 550 (high sweat)

This 3-sensor combination triggers alerts **before** visible symptoms, making it a life-saving solution for laborers, sanitation workers, and school children.

## Circuit Diagram

Available in `/images/wiring_diagram.png`  
Pins:
- GSR → GPIO 34
- DS18B20 → GPIO 21
- DHT22 → GPIO 19
- LED → GPIO 13
- Buzzer → GPIO 12

## Alert Logic

```
IF (GSR < 550) AND (Ambient Temp > 35°C) AND (Humidity < 40%)
THEN → Activate LED + Buzzer alert (High Risk)
```

## TRL-8 Testing Plan
- Logged every 10 min over 48h
- CSV format with timestamps
- Sample log sheet in `/logs/HydraBand_TRL8_Log_Template.xlsx`
- Status: Safe / Mild / High Risk

## Files Included
- Code: `/code/hydra_band.ino`
- Proposal: `/docs/HydraBand_Proposal.pdf`
- Wokwi: [Simulation Link](https://wokwi.com/projects/demo-link)
- Logs: `/logs/`
- Diagrams: `/images/`

##  Future Ready
This modular system is ready for sensor swap-in with upcoming Indian-made bio-impedance and analog front-end sensor chips.

---
Made with care by Team AquaVeda
