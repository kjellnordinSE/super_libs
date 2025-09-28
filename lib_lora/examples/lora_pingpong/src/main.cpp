// lib_lora/examples/lora_pingpong/src/main.cpp
#include <Arduino.h>
#include <lora.h>
#include "common_cfg.h"

static uint8_t buf[64];
static uint32_t seq = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  auto cfg = makeConfigFromFlags();
  if (!lora::begin(cfg)) {
    Serial.println("[LoRa] begin FAILED");
    while (true) delay(1000);
  }
  Serial.println("[LoRa] begin OK");
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  char msg[32];
  int n = snprintf(msg, sizeof(msg), "HELLO %lu", (unsigned long)seq++);
  int st = lora::send(reinterpret_cast<const uint8_t*>(msg), n);
  Serial.printf("[LoRa] TX %d : %s\n", st, msg);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
  delay(950);
}

