> **⚠️ EDUCATIONAL USE ONLY — AUTHORIZED TESTING ONLY.**
> This project exists for education, research, and **defense of systems you own
> or hold explicit written authorization to assess**. Unauthorized use is
> prohibited and may be illegal. Read [ETHICS.md](ETHICS.md) and
> [SCOPE.md](SCOPE.md) before use. Use at your own risk; **AS IS**, no warranty.

# H14 — LoRa RF Security Toolkit

![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)
![GitHub stars](https://img.shields.io/github/stars/5h4d0wn1k/h14-lora-attack)
![Last commit](https://img.shields.io/github/last-commit/5h4d0wn1k/h14-lora-attack)
![GitHub issues](https://img.shields.io/github/issues/5h4d0wn1k/h14-lora-attack)

**LoRa RF security toolkit** for the ESP32 + SX1276/SX1278 — packet capture, replay, frequency scanning, and frequency-hopping-spread-spectrum (FHSS) detection on 433/868 MHz, for authorized analysis of your **own-lab** LoRa networks.

## Why

LoRa IoT networks are increasingly used for utility meter reading, agriculture, and asset tracking — and their unprotected 433/868 MHz traffic can reveal how an attacker might replay or disrupt it. This toolkit studies **wireless protocol security in the ISM/SRD bands**: sniffing with CRC validation, replay of captured packets, frequency sweep, and FHSS-pattern detection with RSSI/SNR per packet. Everything jamming/replay here is a **proof for study and simulation**: live triggers require a `LAB_*` allowlist **and** explicit `--yes`, and emissions are confined to your own shielded bench under spectrum duty-cycle limits (ETSI ~1%, FCC Part 15).

## Features

- **Packet sniffing** — continuous monitoring with CRC validation.
- **Replay** — re-transmit captured packets on the same frequency.
- **Frequency scan** — sweep 433.0–434.0 MHz for activity.
- **FHSS detection** — hop-pattern analysis, unique-frequency count, hop-interval estimate.
- **Signal metrics** — RSSI and SNR per captured packet.
- **Offline host demo** (`host/h14_cli.py --demo`) with packet-log fixtures.

## Quickstart

### Firmware (ESP32 + SX1276/SX1278)

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/
arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 firmware/
```

### Host helper (offline demo)

```bash
python3 host/h14_cli.py --demo
```

### Tests

```bash
python3 -m unittest discover -s tests -v
```

> See [ETHICS.md](ETHICS.md) and [SCOPE.md](SCOPE.md) before any bench work.

## Project structure

```
h14-lora-attack/
├── firmware/            # ESP32 + SX1276/SX1278 Arduino sketch
├── host/                # h14_cli.py, hw_common.py (offline host demo)
├── fixtures/            # packet-log fixtures
├── tests/               # offline unittest suite
└── ETHICS.md, SCOPE.md  # authorized-use & spectrum rules
```

## Documentation

- [ETHICS.md](ETHICS.md) — authorized-use policy
- [SCOPE.md](SCOPE.md) — wireless lab scope
- [SECURITY.md](SECURITY.md) — security policy
- [CONTRIBUTING.md](CONTRIBUTING.md) — contribution guide
- [firmware/README.md](firmware/README.md) — firmware build notes

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). No enhancements that enable third-party interception or replay.

## License

MIT. See [LICENSE](LICENSE).