//lib_lora/lora.cpp
#include "lora.h"

namespace lora {

static SPIClass* g_spi = nullptr;
static Module*   g_mod = nullptr;

#if defined(LORA_CHIP_SX1262)
  static SX1262* g_radio = nullptr;
#else
  // fallback om du någon gång bygger mot SX1276
  static SX1276* g_radio = nullptr;
#endif

static Config g_cfg;

static uint32_t bwToHz(uint16_t khz) { return (uint32_t)khz * 1000UL; }

bool begin(const Config& cfg) {
  g_cfg = cfg;

  // Starta SPI på angivna pinnar (XIAO S3 behöver detta)
  if (!g_spi) g_spi = &SPI;
  if (cfg.pins.sck >= 0 && cfg.pins.miso >= 0 && cfg.pins.mosi >= 0) {
    g_spi->begin(cfg.pins.sck, cfg.pins.miso, cfg.pins.mosi);
  } else {
    g_spi->begin(); // fallback
  }

#if defined(LORA_CHIP_SX1262)
  // SX1262: Module(cs, dio1, rst, busy, spi)
  g_mod   = new Module(cfg.pins.nss, cfg.pins.dio1, cfg.pins.rst, cfg.pins.busy, *g_spi);
  g_radio = new SX1262(g_mod);
#else
  // SX127x: behåll om du vill stödja båda familjerna
  g_mod   = new Module(cfg.pins.nss, /*dio0*/ cfg.pins.dio1, cfg.pins.rst, /*dio1*/ cfg.pins.busy, *g_spi);
  g_radio = new SX1276(g_mod);
#endif

  // RF-switch om din HAT kräver styrpin (skadar ej om hårdlänkad)
  if (cfg.pins.rfsw >= 0) {
#if defined(LORA_CHIP_SX1262)
    g_radio->setRfSwitchPins(cfg.pins.rfsw, cfg.pins.rfsw);
#endif
  }

  // Sätt radio: RadioLib begin(f, bwHz, sf, cr, sync, power, preamble)
  const uint32_t bw_hz = bwToHz(cfg.bw_khz);
  int16_t state = g_radio->begin(
    cfg.freq_hz,
    bw_hz,
    cfg.sf,
    cfg.cr,
    cfg.sync,
    cfg.tx_dbm,
    cfg.preamble
  );
  if (state != RADIOLIB_ERR_NONE) {
    return false;
  }

  // CRC
  g_radio->setCRC(cfg.crc_on);

  return true;
}

int send(const uint8_t* data, size_t len) {
  if (!g_radio) return RADIOLIB_ERR_UNKNOWN;
  return g_radio->transmit((uint8_t*)data, len);
}

int recv(uint8_t* buf, size_t maxLen, int16_t* outRSSI, float* outSNR) {
  if (!g_radio) return RADIOLIB_ERR_UNKNOWN;

  int16_t state = g_radio->receive(buf, maxLen);
  if (state == RADIOLIB_ERR_NONE) {
    if (outRSSI) *outRSSI = g_radio->getRSSI();
    if (outSNR)  *outSNR  = g_radio->getSNR();
  }
  return state;
}

void sleep()   { if (g_radio) g_radio->sleep(); }
void standby() { if (g_radio) g_radio->standby(); }

} // namespace lora

