//message.h
#pragma once
#include <Arduino.h>
#include <stdint.h>

namespace msg {

// === Konstanter / metadata ===
static constexpr uint8_t  MAGIC0 = 'M';
static constexpr uint8_t  MAGIC1 = '1';
static constexpr uint8_t  VERSION_SENSOR_V1 = 1;
static constexpr uint8_t  TYPE_SENSOR = 1;

// Total storlek (byte) = header (8) + body (40) = 48
static constexpr size_t   FRAME_HDR_SIZE    = 8;
static constexpr size_t   SENSOR_V1_BODY_SZ = 40;
static constexpr size_t   SENSOR_V1_TOTAL   = FRAME_HDR_SIZE + SENSOR_V1_BODY_SZ;

// === Header (inte packad på “tråd”; vi skriver fälten manuellt i encode) ===
struct FrameHeader {
  uint8_t  magic0;   // 'M'
  uint8_t  magic1;   // '1'
  uint8_t  version;  // 1
  uint8_t  type;     // TYPE_SENSOR
  uint16_t seq;      // little-endian på tråden
  uint16_t flags;    // bit 0: CRC8 över body är närvarande (V1: alltid 1)
};

// === SensorPayload V1 (värden i skalade heltal) ===
// Detta är “arbetsstruct” i RAM; vi skriver/byter ordning vid encode/decode.
struct SensorV1 {
  uint32_t ts_ms;          // tidsstämpel (ex millis) (0 = okänd)
  int16_t  t_air_cX100;    // 25.43°C → 2543, 0 = saknas
  uint16_t rh_x10;         // 55.3% → 553, 0 = saknas
  uint16_t p_hpa_x10;      // 1013.2 hPa → 10132, 0 = saknas
  uint16_t light_lux_div10;// 1234.5 lux → 12345/10 ≈ 1235 → lagra 1235
  int16_t  t_sauna_cX100;  // kan vara 0 om ej bastu
  int16_t  t_water_cX100;  // kan vara 0 om ej sjö
  uint16_t batt_mV;        // 0 om saknas
  int8_t   rssi;           // radiosnitt, 0 = okänt
  int8_t   snr;            // 0 = okänt
  uint8_t  soil_count;     // hur många soil-värden som är meningsfulla (0..8)
  uint8_t  _rsv0;          // reserverad (håll = 0)

  uint16_t soil[8];        // råa/normaliserade (0..65535). 0 = saknas.

  uint8_t  crc8;           // CRC8 över body (alla bytes utom denna)
  uint8_t  _pad;           // pad för att nå exakt 40 bytes body
};

// === API ===
// Fyll header (magic/version/type/seq/flags).
void     makeHeader(FrameHeader& h, uint8_t version, uint8_t type, uint16_t seq, uint16_t flags = 0x0001);

// Packa till buffer i “trådformat” (LE) – returnerar total längd (48) eller 0 vid fel.
size_t   encodeSensorV1(const FrameHeader& h, const SensorV1& s, uint8_t out[SENSOR_V1_TOTAL]);

// Avkoda från buffer – returnerar true om ok (magic/version/type/CRC stämmer).
bool     decodeSensorV1(const uint8_t in[SENSOR_V1_TOTAL], FrameHeader& h_out, SensorV1& s_out);

// Hjälp: beräkna CRC8 (Dallas/Maxim, poly 0x31, init 0x00).
uint8_t  crc8_dallas(const uint8_t* data, size_t len);

} // namespace msg

