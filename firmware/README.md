# LoRa Attack Firmware

## Purpose

Study LoRa packet capture formats and FHSS detection offline. Live replay gated behind explicit lab authorization on own nodes only.

## Board

- **Board**: ESP32 + SX1276/SX1278 (LoRa)
- **FQBN**: `esp32:esp32:esp32`
- **Sketch**: `h14_lora_attack/h14_lora_attack.ino`

## Wiring

```
SX1278: CS->GPIO5, RST->GPIO14, IRQ->GPIO2, SCK->GPIO18, MOSI->GPIO23, MISO->GPIO19, VCC->3V3, GND->GND
```

## Build

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/h14_lora_attack
# upload (example, ESP32-C6):
# arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyACM0 firmware/h14_lora_attack
```

## Runtime

See the root README "IMPORTANT" section before powering on. This firmware is
for authorized own-lab study. Serial console exposes the interactive command
set described in the root README. All identifiers in the sketch are
placeholders (`lab-*` SSIDs, `00:11:22:33:44:55`, RFC 5737 / example.com).
