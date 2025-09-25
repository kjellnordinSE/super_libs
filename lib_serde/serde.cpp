//lib_serde/serde.cpp
#include "serde.h"

namespace serde {

#if SERDE_ENABLE_JSON
static size_t append(char* out, size_t cap, size_t& pos, const char* fmt, ...) {
  if (!out || cap==0) return 0;
  va_list ap; va_start(ap, fmt);
  int n = vsnprintf(out + pos, (pos < cap) ? cap - pos : 0, fmt, ap);
  va_end(ap);
  if (n < 0) return 0;
  pos += (size_t)n;
  return (size_t)n;
}

size_t toJson(const msg::SensorV1& s, char* out, size_t cap) {
  size_t pos = 0;
  append(out, cap, pos, "{");
  append(out, cap, pos, "\"ver\":%u", (unsigned)msg::VERSION_SENSOR_V1);
  append(out, cap, pos, ",\"ts\":%u", (unsigned)s.ts_ms);
  append(out, cap, pos, ",\"t_air\":%d", (int)s.t_air_cX100);
  append(out, cap, pos, ",\"rh\":%u", (unsigned)s.rh_x10);
  append(out, cap, pos, ",\"p\":%u", (unsigned)s.p_hpa_x10);
  append(out, cap, pos, ",\"lux10\":%u", (unsigned)s.light_lux_div10);
  append(out, cap, pos, ",\"t_sauna\":%d", (int)s.t_sauna_cX100);
  append(out, cap, pos, ",\"t_water\":%d", (int)s.t_water_cX100);
  append(out, cap, pos, ",\"batt\":%u", (unsigned)s.batt_mV);
  append(out, cap, pos, ",\"rssi\":%d", (int)s.rssi);
  append(out, cap, pos, ",\"snr\":%d", (int)s.snr);
  append(out, cap, pos, ",\"soil_count\":%u", (unsigned)s.soil_count);
  append(out, cap, pos, ",\"soil\":[");
  for (int i=0;i<8;i++){
    append(out, cap, pos, "%u", (unsigned)s.soil[i]);
    if (i<7) append(out, cap, pos, ",");
  }
  append(out, cap, pos, "]}");
  // Säkerställ null-terminering om plats finns
  if (pos < cap) out[pos] = '\0';
  else if (cap) out[cap-1] = '\0';
  return pos;
}

bool toJson(const msg::SensorV1& s, Print& out) {
  char buf[512]; // räcker gott för våra fält
  size_t n = toJson(s, buf, sizeof(buf));
  return out.write((const uint8_t*)buf, n) == n;
}
#endif // SERDE_ENABLE_JSON

#if SERDE_ENABLE_CBOR
// --- Minimal CBOR encoder för våra fall ---
// Hjälpfunktioner för att skriva typ/argument enligt RFC 7049/8949
static bool put(uint8_t* out, size_t cap, size_t& pos, uint8_t b) {
  if (pos >= cap) return false;
  out[pos++] = b; return true;
}
static bool putN(uint8_t* out, size_t cap, size_t& pos, const void* src, size_t n) {
  if (pos + n > cap) return false;
  memcpy(out+pos, src, n); pos += n; return true;
}
static bool emitTypeArg(uint8_t major, uint64_t val, uint8_t* out, size_t cap, size_t& pos) {
  if (val < 24) return put(out, cap, pos, (uint8_t)((major<<5) | val));
  else if (val <= 0xFF) {
    if (!put(out, cap, pos, (uint8_t)((major<<5) | 24))) return false;
    return put(out, cap, pos, (uint8_t)val);
  } else if (val <= 0xFFFF) {
    if (!put(out, cap, pos, (uint8_t)((major<<5) | 25))) return false;
    uint16_t v = (uint16_t)val; uint8_t tmp[2] = {(uint8_t)(v>>8),(uint8_t)v};
    return putN(out, cap, pos, tmp, 2);
  } else {
    if (!put(out, cap, pos, (uint8_t)((major<<5) | 26))) return false;
    uint32_t v = (uint32_t)val; uint8_t tmp[4] = {(uint8_t)(v>>24),(uint8_t)(v>>16),(uint8_t)(v>>8),(uint8_t)v};
    return putN(out, cap, pos, tmp, 4);
  }
}
static bool emitUInt(uint64_t v, uint8_t* out, size_t cap, size_t& pos) {
  return emitTypeArg(0, v, out, cap, pos);
}
static bool emitInt(int64_t v, uint8_t* out, size_t cap, size_t& pos) {
  if (v >= 0) return emitTypeArg(0, (uint64_t)v, out, cap, pos);
  // negativer: major=1, representeras som -1 - n
  uint64_t n = (uint64_t)(-1 - v);
  return emitTypeArg(1, n, out, cap, pos);
}
static bool emitArray(uint64_t n, uint8_t* out, size_t cap, size_t& pos) {
  return emitTypeArg(4, n, out, cap, pos);
}
static bool emitMap(uint64_t n, uint8_t* out, size_t cap, size_t& pos) {
  return emitTypeArg(5, n, out, cap, pos);
}

size_t toCbor(const msg::SensorV1& s, uint8_t* out, size_t cap) {
  size_t pos = 0;
  // Map med 13 par
  if (!emitMap(13, out, cap, pos)) return 0;

  // Nyckel 13: ver
  if (!emitUInt(13, out, cap, pos) || !emitUInt(msg::VERSION_SENSOR_V1, out, cap, pos)) return 0;
  // 1: ts
  if (!emitUInt(1, out, cap, pos) || !emitUInt(s.ts_ms, out, cap, pos)) return 0;
  // 2: t_air (i16)
  if (!emitUInt(2, out, cap, pos) || !emitInt(s.t_air_cX100, out, cap, pos)) return 0;
  // 3: rh (u16)
  if (!emitUInt(3, out, cap, pos) || !emitUInt(s.rh_x10, out, cap, pos)) return 0;
  // 4: p (u16)
  if (!emitUInt(4, out, cap, pos) || !emitUInt(s.p_hpa_x10, out, cap, pos)) return 0;
  // 5: lux10 (u16)
  if (!emitUInt(5, out, cap, pos) || !emitUInt(s.light_lux_div10, out, cap, pos)) return 0;
  // 6: t_sauna (i16)
  if (!emitUInt(6, out, cap, pos) || !emitInt(s.t_sauna_cX100, out, cap, pos)) return 0;
  // 7: t_water (i16)
  if (!emitUInt(7, out, cap, pos) || !emitInt(s.t_water_cX100, out, cap, pos)) return 0;
  // 8: batt (u16)
  if (!emitUInt(8, out, cap, pos) || !emitUInt(s.batt_mV, out, cap, pos)) return 0;
  // 9: rssi (i8)
  if (!emitUInt(9, out, cap, pos) || !emitInt(s.rssi, out, cap, pos)) return 0;
  // 10: snr (i8)
  if (!emitUInt(10, out, cap, pos) || !emitInt(s.snr, out, cap, pos)) return 0;
  // 11: soil_count (u8)
  if (!emitUInt(11, out, cap, pos) || !emitUInt(s.soil_count, out, cap, pos)) return 0;
  // 12: soil array (8 x u16)
  if (!emitUInt(12, out, cap, pos)) return 0;
  if (!emitArray(8, out, cap, pos)) return 0;
  for (int i=0;i<8;i++) if (!emitUInt(s.soil[i], out, cap, pos)) return 0;

  return pos;
}
#endif // SERDE_ENABLE_CBOR

} // namespace serde

