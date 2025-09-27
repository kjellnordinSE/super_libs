//lib_lora/examples/lora_pingpong/src/main.cpp
#include <Arduino.h>
#include <lora.h>
#include "common_cfg.h"

static uint8_t buf[64];

void setup() {
  Serial.begin(115200);
  delay(200);

  auto cfg = makeConfigFromFlags();

  if (!lora::begin(cfg)) {
    Serial.println("[LoRa] begin FAILED");
    while (true) { delay(1000); }
  }
  Serial.println("[LoRa] begin OK");
}

void loop() {
  const char* msg = "PING";
  int tx = lora::send(reinterpret_cast<const uint8_t*>(msg), strlen(msg));
  Serial.printf("[LoRa] TX %d\n", tx);

  uint32_t t0 = millis();
  while (millis() - t0 < 500) {
    int16_t rssi; float snr;
    int rx = lora::recv(buf, sizeof(buf), &rssi, &snr);
    if (rx > 0) {
      Serial.printf("[LoRa] RX %d bytes, RSSI=%d dBm, SNR=%.1f dB: ", rx, rssi, snr);
      for (int i = 0; i < rx; i++) Serial.write(buf[i]);
      Serial.println();
      break;
    }
    delay(10);
  }

  delay(1000);
}

