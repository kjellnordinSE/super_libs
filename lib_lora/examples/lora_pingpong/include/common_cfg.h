//common_cfg.h
#pragma once
#include <lora.h>

inline lora::Config makeConfigFromFlags() {
  lora::Config c;
  // Pins
  c.pins.nss  = LORA_CS;
  c.pins.dio1 = LORA_DIO1;
  c.pins.rst  = LORA_RST;
  c.pins.busy = LORA_BUSY;
  c.pins.rfsw = LORA_RFSW;
  c.pins.sck  = LORA_SCK;
  c.pins.miso = LORA_MISO;
  c.pins.mosi = LORA_MOSI;

  // Radio params
  c.freq_hz   = LORA_FREQ_HZ;
  c.bw_khz    = LORA_BW_KHZ;
  c.sf        = LORA_SF;
  c.cr        = LORA_CR;
  c.preamble  = LORA_PREAMBLE;
  c.sync      = LORA_SYNCWORD;
  c.tx_dbm    = LORA_TX_DBM;
  c.crc_on    = (LORA_CRC_ON != 0);
  return c;
}

