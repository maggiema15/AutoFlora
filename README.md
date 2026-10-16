# 🌱 AutoFlora — STM32 Smart Plant Care System

🚧 **Project Status: Under Active Development**  

AutoFlora is an embedded smart plant-care system built around an **STM32 microcontroller**.  
The system monitors environmental conditions and soil moisture, then automatically waters a plant when necessary while incorporating safety mechanisms such as reservoir-level detection and controlled pump operation.

This project was created to gain hands-on experience with embedded firmware, sensor interfacing, ADC, I2C, GPIO, hardware control, and real-time system design.

---

## Features

- 🌡️ Temperature monitoring
- 💧 Relative humidity monitoring
- ☀️ Ambient light measurement
- 🌱 Soil moisture sensing
- 🚰 Water reservoir level detection
- ⚙️ STM32-controlled water pump
- 🔌 MOSFET-based pump power control
- 🖥️ UART serial monitoring and debugging
- 🛡️ Low-water protection
- 🌊 Automatic watering logic
- ⏱️ Controlled watering and cooldown periods

---

## System Overview

AutoFlora continuously collects data from several sensors:

- **AHT20** — temperature and relative humidity
- **BH1750** — ambient light intensity
- **Capacitive soil moisture sensor** — soil moisture
- **Float switch** — reservoir water level

The STM32 processes these measurements and determines whether watering is required.

When the soil becomes too dry, the controller can activate a water pump through a **DFRobot DFR0457 MOSFET power controller**. The pump is controlled separately from the low-power sensor circuitry so that the STM32 GPIO only provides the control signal rather than powering the pump directly.

The system is designed to eventually use a state-based watering strategy:

`MONITOR → DRY CONFIRMATION → WATERING → SOAK → COOLDOWN`

This prevents a noisy sensor reading from immediately triggering the pump and reduces the risk of overwatering.

---

## Hardware

| Component | Purpose |
|---|---|
| STM32 Development Board | Main microcontroller |
| AHT20 | Temperature & humidity sensing |
| BH1750 | Ambient light sensing |
| Capacitive Soil Moisture Sensor | Soil moisture measurement |
| Water-Level Float Switch | Detects low reservoir level |
| DFRobot DFR0457 | MOSFET power controller for pump |
| DC Water Pump | Automatic watering |
| Breadboard | Prototyping |
| USB-to-TTL Adapter | UART debugging |
| ST-Link V2 | STM32 programming/debugging |
| Jumper Wires | Electrical connections |

---

## Interfaces

### I2C

The AHT20 and BH1750 share the same I2C bus.

| Signal | STM32 Pin |
|---|---|
| SCL | PB6 |
| SDA | PB7 |
| Power | 3.3 V |
| Ground | GND |

The devices can share the bus because they use different I2C addresses.

### UART

UART is currently used for system logging and sensor debugging.

| Connection | Configuration |
|---|---|
| STM32 UART TX | PA9 |
| USB-TTL RX | PA9 |
| Ground | Common GND |
| Baud Rate | 115200 |

### Soil Moisture

The capacitive soil sensor provides an analog output that is sampled using the STM32 ADC.

Initial measurements observed during testing:

- Dry air: ~3040 ADC
- Water: ~1550 ADC
- Moist soil: ~2200–3000 ADC

These values will be calibrated using real soil conditions before being used for automatic watering decisions.

### Water-Level Detection

A float switch is used to determine whether sufficient water remains in the reservoir.

If the reservoir is empty or below the required level, the firmware will prevent pump activation.
