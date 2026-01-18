# Node Smart Clock proof of concept

**Node Smart Clock proof of concept** is an proof of concept for my project node smart clock, its on bred board where i have esp32 dev board and H7BC ENS160+AHT21 sensor witch display and ttp touch button just to know if my vision is possible.

---

## Wiring diagram

### TFT display 
- GND -> esp32 GND
- VCC -> esp32 3.3
- SCL -> esp32 D18
- SDA -> esp32 D23
- RES -> esp32 D16
- DC -> esp32 D05
- CS -> esp32 D17
- BLK -> esp32 D04
- A -> esp32 D26
- B -> esp32 D27
- PUSH -> esp32 D14
- KO -> esp32 D25

### ENS160+AHT21 sensor
- 3.3 -> esp32 3.3
- GND -> esp32 GND
- SCL -> esp32 D22
- SDA -> esp32 D21
- ADD -> esp32 GND

### TTP223
- GND -> esp32 GND
- VCC -> esp32 3.3
- I/O -> esp32 D15

## Code

[https://github.com/martinsram3k/Node-Smart-Clock/tree/main/proof_of_concept/code]

## Gallery

<img src="img/IMG_20260118_214504446.jpg" width="90%"></img>



## Support the Project

If you like the **Node Smart Clock**, there are several ways you can support the development:

<!-- * **MakerWorld Crowdfunding:** Support our campaign to help us scale PCB production and DIY kits! -->

- **Buy Me a Coffee:** Support my work directly at [buymeacoffee.com/martin.sram3k](https://buymeacoffee.com/martin.sram3k).
- **Follow My Journey:** Check out all my projects and social media via my [Linktree](https://linktr.ee/martin.sram3k).

Your backing helps us continue developing new open-source features and high-quality hardware for the community.

---

**License:** MIT – Feel free to share, modify, and build!
