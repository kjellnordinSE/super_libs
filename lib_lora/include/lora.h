//lib_lora/lora.h
#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

namespace lora {

struct Pins {
  int nss  = -1;   // LORA_CS
  int dio1 = -1;   // SX126x DIO1
  int rst  = -1;   // reset
  int busy = -1;   // SX126x BUSY
  int rfsw = -1;   // RF switch (valfri, ofta samma pin för tx/rx)
  int sck  = -1;   // SPI SCK
  int miso = -1;   // SPI MISO
  int mosi = -1;   // SPI MOSI
};

struct Config {
  Pins     pins;
  uint32_t freq_hz  = 868100000UL;
  uint16_t bw_khz   = 125;
  uint8_t  sf       = 9;    // 5..12
  uint8_t  cr       = 5;    // 5=>4/5, 8=>4/8
  uint8_t  preamble = 8;    // symboler
  uint8_t  sync     = 0x34; // LoRa sync word
  int8_t   tx_dbm   = 2;    // sändeffekt
  bool     crc_on   = true;
};

bool begin(const Config& cfg);
int  send(const uint8_t* data, size_t len);
int  recv(uint8_t* buf, size_t maxLen, int16_t* outRSSI = nullptr, float* outSNR = nullptr);
void sleep();
void standby();

} // namespace lora

