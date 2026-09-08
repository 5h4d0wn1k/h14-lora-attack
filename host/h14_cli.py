#!/usr/bin/env python3
"""H14 - LoRa Attack host helper: offline packet-log + FHSS detection.
Live replay is never triggered from this script.
Educational/authorized own-lab use only (see README "IMPORTANT").
"""
import argparse
import os
import sys

MOD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, MOD)
from hw_common import DEMO_TAG, read_target

SKIP_LINES = 0
DEFAULT_SKIP = ("SKIP",)


def analyze(text):
    pkts = []
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith("#") or line.startswith("SKIP"):
            continue
        # format: 433.1 -72 3.5 AA BB CC
        parts = line.split()
        if len(parts) < 4:
            continue
        try:
            freq, rssi, snr = float(parts[0]), int(parts[1]), float(parts[2])
            data = bytes.fromhex("".join(parts[3:]))
        except ValueError:
            continue
        pkts.append({"freq": freq, "rssi": rssi, "snr": snr, "data": data})
    return pkts


def fhss_detect(pkts):
    freqs = {round(p["freq"], 1) for p in pkts}
    return {"unique": len(freqs),
            "likely_fhss": len(freqs) > 2,
            "freqs": sorted(freqs)}


def run_demo():
    print("=== H14 LoRa packet log analysis (offline) ===")
    pkts = analyze(read_target(
        "fixtures/packet_log.txt",
        "433.0 -72 3.5 AA 11 22\n433.1 -75 2.9 BB CC\n433.2 -70 4.0 DD\n"
        "433.1 -73 3.1 EE\n433.3 -69 4.2 FF\n"))
    print("  packets: %d" % len(pkts))
    f = fhss_detect(pkts)
    print("  unique freq: %d  likely FHSS: %s" % (f["unique"], f["likely_fhss"]))
    print(DEMO_TAG)
    return 0


def main(argv=None):
    p = argparse.ArgumentParser(
        description="H14 LoRa attack - offline packet/FHSS analysis")
    p.add_argument("--demo", action="store_true", help="offline demo (exit 0)")
    p.add_argument("--file", help="packet log path")
    args = p.parse_args(argv)
    text = read_target("fixtures/packet_log.txt")
    if args.file:
        text = open(args.file).read()
    if args.demo or not args.file:
        return run_demo()
    pkts = analyze(text)
    for pkt in pkts:
        print("%.1f MHz rssi=%s snr=%s len=%d" % (
            pkt["freq"], pkt["rssi"], pkt["snr"], len(pkt["data"])))
    print(fhss_detect(pkts))
    return 0


if __name__ == "__main__":
    sys.exit(main())
