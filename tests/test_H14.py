import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "host"))
import h14_cli as m


class TestLoRa(unittest.TestCase):
    def test_parse_log(self):
        text = "433.0 -72 3.5 AA BB CC\n433.3 -69 4.2 DD\n"
        pkts = m.analyze(text)
        self.assertEqual(len(pkts), 2)
        self.assertEqual(pkts[0]["freq"], 433.0)
        self.assertEqual(pkts[0]["data"], bytes.fromhex("AABBCC"))

    def test_fhss_detect(self):
        text = ("433.0 -72 3.5 AA\n433.1 -75 2.9 BB\n433.2 -70 4.0 CC\n"
                "433.3 -69 4.2 DD\n")
        f = m.fhss_detect(m.analyze(text))
        self.assertEqual(f["unique"], 4)
        self.assertTrue(f["likely_fhss"])

    def test_single_freq_no_fhss(self):
        f = m.fhss_detect(m.analyze("433.0 -70 3.0 AA\n433.0 -71 3.1 BB\n"))
        self.assertFalse(f["likely_fhss"])


if __name__ == "__main__":
    unittest.main()
