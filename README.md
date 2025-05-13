
# 🎥 Lechnology Motorized Camera Slider

**An open-source, precision motorized camera slider** engineered with NEMA 23 stepper motors, Trinamic TMC2208 drivers, and robust aluminum extrusion rails. This high-performance build is powered by embedded C++ firmware running on an Arduino-compatible board, offering smooth motion control for cinematic shots and timelapses.

📺 **Click to watch the full build video below:**  
[![Watch on YouTube](https://img.youtube.com/vi/QBlhba4QOJE/hqdefault.jpg)](https://youtu.be/QBlhba4QOJE)

[![Watch on YouTube](https://img.shields.io/badge/Watch%20Video-%F0%9F%8E%A5-red?logo=youtube)](https://www.youtube.com/watch?v=-9Q4Nq9MRP0)

---

## 🚀 Features

- ⚙️ **Trinamic TMC2208 Motor Drivers** — Whisper-quiet stepper control with microstepping and stall detection  
- 🧠 **Embedded C++ Firmware** — Lightweight, real-time codebase written in modern C++ for performance and modularity  
- 🏗️ **NEMA 23 Stepper Motors** — High-torque motors ensure smooth, vibration-free motion  
- 🛤️ **20x60 Aluminum Extrusion Rail** — Stiff and stable track structure for long sliding spans  
- 📟 **OLED Status Display** — SSD1306 I²C screen for live feedback and settings interface
- 🎛️ **Rotary Encoder Interface** — Intuitive UI for speed, travel distance, and direction selection  
- 🧰 **3D-Printed Mounts and Carriage** — Custom-printed parts for easy integration and minimal backlash  

---

## 🧰 Bill of Materials

| Component              | Description                                        |
|------------------------|----------------------------------------------------|
| **Microcontroller**    | Arduino Nano or equivalent (5V logic recommended)  |
| **Motor Drivers**      | TMC2208 SilentStepStick (UART or standalone mode)  |
| **Stepper Motors**     | NEMA 23 (e.g. 2.8A 1.26Nm torque)                  |
| **Linear Rail**        | 20x60 mm Aluminum Extrusion                        |
| **Pulley System**      | GT2 Timing Belt and 20T Pulleys                    |
| **Display**            | SSD1306 OLED (I²C) —                               |
| **Input Device**       | Rotary Encoder with push-button                    |  
| **3D-Printed Parts**   | Carriage, End Stops, Motor Mounts                  |
| **Miscellaneous**      | Screws, V-slot rollers, wiring, connectors         |

---

## 💻 Firmware Overview

The firmware is written in **embedded C++**, structured with a modular and interrupt-safe architecture. Key features include:

- 🧠 **Stepper Motion Engine** — Timer-driven microstepping control with trapezoidal acceleration profiles  
- 📐 **Configurable Travel Bounds** — Software-defined soft stops and homing support  
- 🔁 **Rotary Encoder Interface** — Debounced encoder input with push-button support for navigation and value adjustment
- 📺 **SSD1306 OLED Interface** — Displays current position, target speed, and system status via I²C  
- 🧭 **User Interaction Layer** — Simple UI framework reused across Lechnology builds  

---

## 📜 License

This project is licensed under the **Lechnology Non-Commercial MIT + Beerware License**:

> If you use this and we meet in person, you're invited to buy me a coffee or beer ☕🍺  
> Commercial use is prohibited without prior approval.

See [LICENSE.md](LICENSE.md) for full terms.

---

## 🔗 Links

- 🔧 [GitHub Repo](https://github.com/marcuslechner/Camera-Slider)  
- 🎥 [Project Video](https://www.youtube.com/watch?v=-9Q4Nq9MRP0)  
- 📷 [More from Lechnology Engineering](https://youtube.com/@lechnologyengineering)
