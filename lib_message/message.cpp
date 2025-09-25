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
