#!/usr/bin/env bash
set -euo pipefail

# 0) Var är vi?
test -d .git || { echo "Kör i repo-roten (där .git finns)"; exit 1; }

# 1) Gå till/skap branch
if git rev-parse --verify feat/lib_serde-v0 >/dev/null 2>&1; then
  git switch feat/lib_serde-v0
else
  git switch -c feat/lib_serde-v0
fi

# 2) Rätta lib_serde: examles -> examples (vanlig mv, då ursprunget troligen är otrackat)
if [ -d lib_serde/examles ]; then
  mkdir -p lib_serde/examples
  # Flytta exempel om de finns
  if [ -d lib_serde/examles/serde_print ]; then
    mkdir -p lib_serde/examples/serde_print/src
    [ -f lib_serde/examles/serde_print/platformio.ini ] && mv lib_serde/examles/serde_print/platformio.ini lib_serde/examples/serde_print/
    [ -f lib_serde/examles/serde_print/src/main.cpp ] && mv lib_serde/examles/serde_print/src/main.cpp lib_serde/examples/serde_print/src/
  fi
  # Städa tomma korgar
  rmdir -p lib_serde/examles/serde_print/src 2>/dev/null || true
  rmdir -p lib_serde/examles/serde_print     2>/dev/null || true
  rmdir -p lib_serde/examles                 2>/dev/null || true
fi

# 3) Skapa minimal platformio.ini för serde-exemplet om den saknas
if [ ! -f lib_serde/examples/serde_print/platformio.ini ]; then
  cat > lib_serde/examples/serde_print/platformio.ini <<'INI'
[env:esp32dev]
platform = espressif32
board    = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed  = 115200

; låt PIO hitta super_libs (tre nivåer upp från example-mappen)
lib_extra_dirs = ../../..
lib_ldf_mode   = chain+

; valfritt: sätt din by-id-port här
; upload_port  = /dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0
; test_port    = ${this.upload_port}
INI
fi

# 4) Säkerställ att lib_message finns (message.h/.cpp), annars återskapa
mkdir -p lib_message
if [ ! -f lib_message/message.h ]; then
  cat > lib_message/message.h <<'CPP'
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
CPP
fi

if [ ! -f lib_message/message.cpp ]; then
  cat > lib_message/message.cpp <<'CPP'
#include "message.h"
#include <string.h>

namespace msg {

static inline void wr16LE(uint8_t* p, uint16_t v){ p[0]=v & 0xFF; p[1]=(v>>8)&0xFF; }
static inline void wr32LE(uint8_t* p, uint32_t v){ p[0]=v & 0xFF; p[1]=(v>>8)&0xFF; p[2]=(v>>16)&0xFF; p[3]=(v>>24)&0xFF; }
static inline uint16_t rd16LE(const uint8_t* p){ return (uint16_t)p[0] | ((uint16_t)p[1]<<8); }
static inline uint32_t rd32LE(const uint8_t* p){ return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }

uint8_t crc8_dallas(const uint8_t* data, size_t len){
  uint8_t crc=0x00;
  for(size_t i=0;i<len;++i){
    uint8_t inbyte=data[i];
    for(uint8_t j=0;j<8;j++){
      uint8_t mix=(crc ^ inbyte) & 0x01;
      crc >>= 1;
      if(mix) crc ^= 0x8C; // poly 0x31 (reflected)
      inbyte >>= 1;
    }
  }
  return crc;
}

void makeHeader(FrameHeader& h, uint8_t version, uint8_t type, uint16_t seq, uint16_t flags){
  h.magic0=MAGIC0; h.magic1=MAGIC1; h.version=version; h.type=type; h.seq=seq; h.flags=flags;
}

size_t encodeSensorV1(const FrameHeader& h, const SensorV1& s, uint8_t out[SENSOR_V1_TOTAL]){
  out[0]=h.magic0; out[1]=h.magic1; out[2]=h.version; out[3]=h.type;
  wr16LE(&out[4], h.seq);
  wr16LE(&out[6], h.flags);

  uint8_t* b=&out[FRAME_HDR_SIZE];
  wr32LE(&b[0],  s.ts_ms);
  wr16LE(&b[4],  (uint16_t)s.t_air_cX100);
  wr16LE(&b[6],  s.rh_x10);
  wr16LE(&b[8],  s.p_hpa_x10);
  wr16LE(&b[10], s.light_lux_div10);
  wr16LE(&b[12], (uint16_t)s.t_sauna_cX100);
  wr16LE(&b[14], (uint16_t)s.t_water_cX100);
  wr16LE(&b[16], s.batt_mV);
  b[18]=(uint8_t)s.rssi;
  b[19]=(uint8_t)s.snr;
  b[20]=s.soil_count;
  b[21]=0;
  for(int i=0;i<8;++i) wr16LE(&b[22 + i*2], s.soil[i]);

  uint8_t crc=crc8_dallas(b, SENSOR_V1_BODY_SZ - 2);
  b[38]=crc;
  b[39]=0;
  return SENSOR_V1_TOTAL;
}

bool decodeSensorV1(const uint8_t in[SENSOR_V1_TOTAL], FrameHeader& h_out, SensorV1& s_out){
  if(in[0]!=MAGIC0 || in[1]!=MAGIC1) return false;
  if(in[2]!=VERSION_SENSOR_V1 || in[3]!=TYPE_SENSOR) return false;

  h_out.magic0=in[0]; h_out.magic1=in[1]; h_out.version=in[2]; h_out.type=in[3];
  h_out.seq=rd16LE(&in[4]); h_out.flags=rd16LE(&in[6]);

  const uint8_t* b=&in[FRAME_HDR_SIZE];
  uint8_t crc_calc=crc8_dallas(b, SENSOR_V1_BODY_SZ - 2);
  if((h_out.flags & 0x0001) && (b[38]!=crc_calc)) return false;

  s_out.ts_ms           = rd32LE(&b[0]);
  s_out.t_air_cX100     = (int16_t)rd16LE(&b[4]);
  s_out.rh_x10          = rd16LE(&b[6]);
  s_out.p_hpa_x10       = rd16LE(&b[8]);
  s_out.light_lux_div10 = rd16LE(&b[10]);
  s_out.t_sauna_cX100   = (int16_t)rd16LE(&b[12]);
  s_out.t_water_cX100   = (int16_t)rd16LE(&b[14]);
  s_out.batt_mV         = rd16LE(&b[16]);
  s_out.rssi            = (int8_t)b[18];
  s_out.snr             = (int8_t)b[19];
  s_out.soil_count      = b[20];
  s_out._rsv0           = 0;
  for(int i=0;i<8;++i) s_out.soil[i]=rd16LE(&b[22 + i*2]);
  s_out.crc8=b[38];
  s_out._pad=0;
  return true;
}
} // namespace msg
CPP
fi

# 5) Stage + commit + push
git add -A
git status
git commit -m "fix: rename examles->examples; ensure lib_message/message.{h,cpp}; serde example builds"
git push -u origin feat/lib_serde-v0

echo "==> Klart. Kör nu: cd lib_serde/examples/serde_print && pio run -e esp32dev -t upload && pio device monitor"
