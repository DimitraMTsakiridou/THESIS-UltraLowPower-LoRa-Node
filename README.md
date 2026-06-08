# Custom Ultra-Low Power LoRa Node for Autonomous Telemetry

Aiming to address the challenges of remote water quality monitoring and bypass the complexity of standard LoRaWAN networks, this repository contains the from-scratch design, end-to-end implementation, and extensive experimental evaluation of a custom, energy-autonomous LoRa telemetry node.

<p align="center">
  <img src="./Images/two_leds.jpg" width="45%" alt="Custom STM32WL55 PCB">
  &nbsp; &nbsp;
  <img src="./Images/whole_system.jpg" width="45%" alt="Solar Panel and Antenna Field Deployment">
</p>

## 🚀 Key Achievements & Architecture

### 🛠️ Hardware Design
* **Custom PCB:** Designed a 4-layer Printed Circuit Board (PCB) based on the **STM32WL55 System-on-Chip**.
* **Zero Leakage:** Incorporated a PMOS load switch to completely cut off power to the water quality sensors (Atlas Scientific) during sleep periods, eliminating leakage currents.

### 💻 Firmware & Security
* **Protocol:** Developed a lightweight, custom Peer-to-Peer (P2P) communication protocol.
* **Cybersecurity:** Shifted data security to the application layer, utilizing hardware-based **AES-256 encryption** and dynamic frame counters to thwart deliberate replay attacks.

### ⚡ Performance & Energy Efficiency
* **Energy-Neutral Operation:** The custom board records an average consumption of merely **261 μA** per 15-minute duty cycle. This ensures a theoretical battery autonomy of over 400 days. Coupled with a 5W solar panel, the system becomes completely energy-neutral.
* **Long-Range Telemetry:** Established a reliable wireless link over a distance of **11 km** under Line-of-Sight (LoS) conditions, demonstrating outstanding noise immunity (SNR -18 dB).

### 🧪 SDR Validation & Stress Testing
The reliability of the physical layer and the system's cybersecurity were validated in a laboratory environment using Software-Defined Radio (**USRP B210**) equipment. 
* Achieved a **90.4% Packet Delivery Ratio (PDR)** under severe congestion stress tests.
* Successfully thwarted deliberate replay and interception attacks.

---
**Keywords:** Water Quality Telemetry, Internet of Things (IoT), LoRa, LPWAN, Custom Hardware (PCB), Energy Harvesting, Cybersecurity, Software-Defined Radio (SDR).
