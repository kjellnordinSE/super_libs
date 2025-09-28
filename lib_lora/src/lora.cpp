#include "lora.h"

namespace lora {

static SPIClass* g_spi = nullptr;
static Module*   g_mod = nullptr;

#if defined(LORA_CHIP_SX1262)
  static SX1262* g_radio = nullptr;
#else
  static SX1276* g_radio = nullptr;  // fallback om du vill stötta SX127x
#endif

static Config g_cfg;

static void logStep(const char* name, int16_t st) {
  Serial.printf("[LoRa] %-20s -> %d\n", name, st);
  Serial.flush();
}

bool begin(const Config& cfg) {
  g_cfg = cfg;

  // ===== DEBUG: dumpa cfg =====
  Serial.println("[LoRa] ---- cfg dump ----");
  Serial.printf("[LoRa] pins: NSS=%d DIO1=%d RST=%d BUSY=%d RFSW=%d SCK=%d MISO=%d MOSI=%d\n",
                g_cfg.pins.nss, g_cfg.pins.dio1, g_cfg.pins.rst, g_cfg.pins.busy,
                g_cfg.pins.rfsw, g_cfg.pins.sck, g_cfg.pins.miso, g_cfg.pins.mosi);
  const float freq_mhz_dbg = g_cfg.freq_hz / 1e6f;
  const float bw_khz_dbg   = (float)g_cfg.bw_khz;
  Serial.printf("[LoRa] params: freq_hz=%lu (%.3f MHz)  bw_khz=%u (%.3f kHz)  sf=%u  cr=%u  preamble=%u  sync=0x%02X  tx_dbm=%d  crc=%d\n",
                (unsigned long)g_cfg.freq_hz, freq_mhz_dbg,
                (unsigned)g_cfg.bw_khz, bw_khz_dbg,
                g_cfg.sf, g_cfg.cr, g_cfg.preamble, g_cfg.sync,
                g_cfg.tx_dbm, g_cfg.crc_on ? 1 : 0);
  Serial.flush();

  // ===== SPI =====
  if (!g_spi) g_spi = &SPI;
  if (g_cfg.pins.sck >= 0 && g_cfg.pins.miso >= 0 && g_cfg.pins.mosi >= 0) {
    g_spi->begin(g_cfg.pins.sck, g_cfg.pins.miso, g_cfg.pins.mosi);
  } else {
    g_spi->begin();
  }

#if defined(LORA_CHIP_SX1262)
  Serial.println("[LoRa] using SX1262 path");
  g_mod   = new Module(g_cfg.pins.nss, g_cfg.pins.dio1, g_cfg.pins.rst, g_cfg.pins.busy, *g_spi);
  g_radio = new SX1262(g_mod);

  // TCXO via DIO3 (om satt i build_flags)
  #if defined(LORA_TCXO_MV) && (LORA_TCXO_MV > 0)
    Serial.printf("[LoRa] setTCXO(%.3f V)\n", LORA_TCXO_MV / 1000.0f);
    logStep("setTCXO", g_radio->setTCXO(LORA_TCXO_MV / 1000.0f));
    delay(10);
  #endif

  // Regulator-läge
  #if defined(LORA_USE_LDO) && (LORA_USE_LDO != 0)
    Serial.println("[LoRa] setRegulatorLDO()");
    g_radio->setRegulatorLDO();
  #else
    Serial.println("[LoRa] setRegulatorDCDC()");
    g_radio->setRegulatorDCDC();
  #endif

  // RF-switch: en-pin (HIGH=TX, LOW=RX) -> rx=NC, tx=RF_SW
  if (g_cfg.pins.rfsw >= 0) {
    Serial.printf("[LoRa] setRfSwitchPins(rx=NC, tx=%d)\n", g_cfg.pins.rfsw);
    g_radio->setRfSwitchPins(RADIOLIB_NC, g_cfg.pins.rfsw);
  }

  // (valfritt) vissa kort använder DIO2 som RF-switch
  #if defined(LORA_DIO2_RFSW) && (LORA_DIO2_RFSW != 0)
    Serial.println("[LoRa] setDio2AsRfSwitch(true)");
    g_radio->setDio2AsRfSwitch(true);
  #endif

#else
  Serial.println("[LoRa] using SX127x path");
  g_mod   = new Module(g_cfg.pins.nss, /*dio0*/ g_cfg.pins.dio1, g_cfg.pins.rst, /*dio1*/ g_cfg.pins.busy, *g_spi);
  g_radio = new SX1276(g_mod);
#endif

  // ===== Stegvis init för maximal tydlighet =====
  // 0) starta med defaultvärden (434 MHz, 125 kHz, SF9, CR7, …)
  int16_t st = g_radio->begin();         logStep("begin(defaults)", st);
  if (st != RADIOLIB_ERR_NONE) return false;

  // 1) frekvens (MHz)
  const float freq_mhz = g_cfg.freq_hz / 1e6f;
  st = g_radio->setFrequency(freq_mhz);  logStep("setFrequency", st);
  if (st != RADIOLIB_ERR_NONE) return false;

  // 2) bandbredd (kHz) – om -8 här vet vi att BW-inmatningen är boven
  const float bw_khz = (float)g_cfg.bw_khz;   // förväntas vara t.ex. 125.0
  st = g_radio->setBandwidth(bw_khz);    logStep("setBandwidth", st);
  if (st != RADIOLIB_ERR_NONE) return false;

  // 3) SF
  st = g_radio->setSpreadingFactor(g_cfg.sf); logStep("setSpreadingFactor", st);
  if (st != RADIOLIB_ERR_NONE) return false;

  // 4) CR (5..8)
  st = g_radio->setCodingRate(g_cfg.cr); logStep("setCodingRate", st);
  if (st != RADIOLIB_ERR_NONE) return false;

  // 5) Sync word
  st = g_radio->setSyncWord(g_cfg.sync); logStep("setSyncWord", st);
  if (st != RADIOLIB_ERR_NONE) return false;

  // 6) Effekt
  st = g_radio->setOutputPower(g_cfg.tx_dbm); logStep("setOutputPower", st);
  if (st != RADIOLIB_ERR_NONE) return false;

  // 7) Preamble
  st = g_radio->setPreambleLength(g_cfg.preamble); logStep("setPreambleLen", st);
  if (st != RADIOLIB_ERR_NONE) return false;

  // 8) CRC
  g_radio->setCRC(g_cfg.crc_on);

  Serial.println("[LoRa] begin OK (stepwise)");
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
    
