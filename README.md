# Node Smart Clock

**Node Smart Clock** is an open-source, multi-functional desk clock powered by the **ESP32-S3**. It is designed as a central hub for makers, entrepreneurs, and crypto enthusiasts, combining advanced environmental sensing with real-time data tracking.

---

## Key Features

- **Environmental Monitoring:** Track CO2 levels, Air Quality (ENS160), and temperature in real-time.
- **Business & Crypto Tracker:** Live tracking for cryptocurrency prices, social media follower counts, and Shopify order notifications.
- **Smart Display:** 2.8" TFT display with **automatic brightness adjustment** based on ambient light (GY-302) to save battery.
- **Visual Notifications:** An array of **8 individually addressable LEDs** on top for animations, progress bars, and alerts.
- **Precise Timekeeping:** Dedicated **PCF8563T RTC chip** with a coin-cell backup, ensuring your alarms work even if the main power is lost.
- **Energy Efficient:** Optimized for **Deep Sleep** with a side-mounted **touch sensor** for intuitive wake/sleep control.

## Design & Construction

The project blends modern electronics with premium natural materials:

- **3D Printed Chassis:** A minimalist housing designed for durability using **brass heat inserts**.
- **Hybrid Materials:** Front and back panels made of **real wood**, suitable for custom laser engraving.
- **Custom PCB:** A professional-grade board integrating the ESP32-S3, 18650 battery charging, and all sensors.

## Hardware (BOM)

| Component   | Type            | Function               | Estimated Price |
| :---------- | :-------------- | :--------------------- | :-------------- |
| **MCU**     | ESP32-S3        | Core processor & WiFi  | ~$5.00          |
| **Sensor**  | ENS160          | Air Quality & CO2      | ~$3.00          |
| **RTC**     | PCF8563T        | High-precision clock   | ~$0.40          |
| **Battery** | 18650 (3300mAh) | Long-lasting power     | ~$5.00          |
| **Display** | 2.8" TFT        | User interface         | ~$15.00         |
| **LEDs**    | 8x Addressable  | Status & Notifications | ~$0.20          |

## Software & Development

This project is fully **Open-Source**.

- **Framework:** Arduino IDE / PlatformIO.
- **Graphics:** Driven by the **LVGL** library for a smooth UI experience.
- **Connectivity:** Integrated WiFi for NTP time sync and API data fetching.

### Installation

1. Clone the repository:  
   `git clone https://github.com/your-username/node-smart-clock.git`
2. Open the project in **PlatformIO**.
3. Configure your `config.h` with your WiFi credentials and API keys (Shopify/Crypto).
4. Upload to your ESP32-S3.

---

## Support the Project

If you like the **Node Smart Clock**, there are several ways you can support the development:

<!-- * **MakerWorld Crowdfunding:** Support our campaign to help us scale PCB production and DIY kits! -->
* **Buy Me a Coffee:** Support my work directly at [buymeacoffee.com/martin.sram3k](https://buymeacoffee.com/martin.sram3k).
* **Follow My Journey:** Check out all my projects and social media via my [Linktree](https://linktr.ee/martin.sram3k).

Your backing helps us continue developing new open-source features and high-quality hardware for the community.

---

**License:** MIT – Feel free to share, modify, and build!
