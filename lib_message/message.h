#pragma once
#include <Arduino.h>
#include <stdint.h>

namespace msg {
static constexpr uint8_t  MAGIC0 = 'M';
static constexpr uint8_t  MAGIC1 = '1';
static constexpr uint8_t  VERSION_SENSOR_V1 = 1;
static constexpr uint8_t  TYPE_SENSOR = 1;

static constexpr size_t   FRAME_HDR_SIZE    = 8;
static constexpr size_t   SENSOR_V1_BODY_SZ = 40;
static constexpr size_t   SENSOR_V1_TOTAL   = FRAME_HDR_SIZE + SENSOR_V1_BODY_SZ;

struct FrameHeader {
  uint8_t  magic0, magic1, version, type;
  uint16_t seq, flags;
};

struct SensorV1 {
  uint32_t ts_ms;
  int16_t  t_air_cX100;
  uint16_t rh_x10;
  uint16_t p_hpa_x10;
  uint16_t light_lux_div10;
  int16_t  t_sauna_cX100;
  int16_t  t_water_cX100;
  uint16_t batt_mV;
  int8_t   rssi;
  int8_t   snr;
  uint8_t  soil_count;
  uint8_t  _rsv0;
  uint16_t soil[8];
  uint8_t  crc8;
  uint8_t  _pad;
};

void     makeHeader(FrameHeader& h, uint8_t version, uint8_t type, uint16_t seq, uint16_t flags = 0x0001);
size_t   encodeSensorV1(const FrameHeader& h, const SensorV1& s, uint8_t out[SENSOR_V1_TOTAL]);
bool     decodeSensorV1(const uint8_t in[SENSOR_V1_TOTAL], FrameHeader& h_out, SensorV1& s_out);
uint8_t  crc8_dallas(const uint8_t* data, size_t len);
} // namespace msg
