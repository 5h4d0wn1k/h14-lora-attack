# H14 — LoRa Attack

LoRa packet sniffing, replay, and frequency hopping detection for ESP32.

## Overview

LoRa protocol security analysis tool:
- Sniff LoRa packets on configurable frequencies
- Replay captured packets
- Scan frequency range for activity
- Detect frequency hopping spread spectrum (FHSS) patterns
- Packet logging with RSSI, SNR, and raw data

## Hardware

| Component | Connection | Role |
|-----------|------------|------|
| ESP32 DevKit | Main board | SPI controller |
| SX1276/SX1278 | SPI (GPIO5/18/19/23) | LoRa transceiver |
| RST pin | GPIO14 | Module reset |
| IRQ pin | GPIO2 | Packet ready |

## Features

- **Packet sniffing**: Continuous monitoring with CRC validation
- **Replay attack**: Re-transmit captured packets on same frequency
- **Frequency scan**: Sweep 433.0–434.0 MHz range
- **FHSS detection**: Analyze hop pattern, count unique frequencies, estimate interval
- **Signal metrics**: RSSI and SNR per captured packet

## Serial Output

```
=== H14 — LoRa Attack ===
LoRa on 433.0 MHz, BW=125000, SF=7, CR=4/5

[PKT] Freq=433.2 RSSI=-67 SNR=9.5 Len=24 Data=AA 11 02 03 ...
[FHOP] Unique frequencies: 4
  433.0 MHz
  433.2 MHz
  433.5 MHz
  433.8 MHz
  Estimated hop interval: 250 ms
  → Likely FHSS detected
```

## Build & Flash

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/
arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 firmware/
```

## Legal Disclaimer

## IMPORTANT: Read before use.

This project is provided for **educational and authorized security testing purposes only**.

### Authorization Requirements
- You MUST have explicit written permission from the network owner before using this tool
- Unauthorized interception of network communications is illegal under federal and state laws
- This tool should ONLY be used on networks you own or have written authorization to test

### Spectrum Regulatory Notes (LoRa ISM/SRD)
- LoRa commonly uses 433 MHz (EU/UK SRD), 868 MHz (ETSI, max ~1% duty
  cycle), or 915 MHz (US ISM, FCC Part 15) — each with strict power
  and duty limits. Sniff/replay must never exceed licensed operation.
- Keep emissions inside an isolated lab: own nodes, attenuators, and
  short test windows only.

### No Third-Party Disruption
Sniffing or replaying traffic from any LoRa network, gateway, or node
you don't own is out of scope (CFAA/Wiretap exposure). Proofs here are
packet logs/fixtures and offline simulation only; live triggers
additionally require the `LAB_*` allowlist AND explicit `--yes`
confirmation.

### Legal Framework
- **Computer Fraud and Abuse Act (CFAA)**: Unauthorized access to computer systems is a federal crime
- **Wiretap Act (18 U.S.C. § 2511)**: Interception of electronic communications without consent is illegal
- **State Laws**: Many states have additional computer crime and wiretapping statutes
- **GDPR/CCPA**: Data collection may be subject to privacy regulations

### Acceptable Use
- Testing security of your own networks
- Authorized penetration testing with written scope
- Academic research in controlled lab environments
- Security education and training

### Prohibited Use
- Intercepting communications on networks you don't own
- Attacking infrastructure without authorization
- Any activity that violates applicable laws or regulations
- Commercial use without proper licensing

### No Warranty
This software is provided "AS IS" without warranty of any kind. The author is not responsible for any misuse or damage caused by this software.

### Responsible Disclosure
If you discover vulnerabilities using this tool, follow responsible disclosure practices:
1. Report to the vendor/owner privately
2. Allow reasonable time for remediation
3. Do not exploit beyond proof of concept

## Live Lab Test Plan

Run ONLY on an isolated, authorized own-lab bench against devices, networks,
and spectrum **you own**. No third-party callers, bystanders, or spectrum users
may be within range of any test transmission.

1. **Isolate** - Put the DUT in a shielded/Faraday enclosure or a room with no
   third-party devices in range. Use attenuators on any transmit path.
2. **Own devices only** - Every target (AP, remote, tag, GPS module, drone FC,
   receiver) must be your own hardware.
3. **Lowest power, shortest duration** - Start at minimum TX power / duty cycle
   and use only the seconds needed.
4. **Record** - Save before/after logs to `reports/` (git-ignored). Never
   capture or store third-party traffic.
5. **Cleanup** - Restore placeholder SSIDs (`lab-*`), MACs (`00:11:22:33:44:55`),
   example.com / RFC5737 addresses, and clear any captured data from the device.

> Jammer / spoofer / replay projects are **proofs for study and simulation**
> only. They refuse live interference scenarios: a live bench trigger requires
> the `LAB_*` allowlist environment variable AND explicit `--yes` confirmation,
> and even then only against your own hardware in a shielded bench.

## Metrics

| Metric | Target | Where |
|---|---|---|
| Firmware compile | `arduino-cli compile --fqbn esp32:esp32:esp32 firmware/h14_lora_attack` PASS | CI/local |
| Host helper | `python3 host/h14_cli.py --demo` exits 0 (offline) | host/ |
| Unit tests | `python3 -m unittest discover -s tests` passes | tests/ |
| py_compile | every `host/*.py` compiles clean | CI/local |

## License

MIT
