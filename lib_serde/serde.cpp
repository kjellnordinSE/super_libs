// lib_serde/serde.cpp
#include "serde.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace serde {

// ===================== JSON ENCODE =====================
#if SERDE_ENABLE_JSON
static size_t append(char* out, size_t cap, size_t& pos, const char* fmt, ...) {
  if (!out || cap == 0) return 0;
  va_list ap; va_start(ap, fmt);
  int n = vsnprintf(out + (pos < cap ? pos : cap), (pos < cap) ? cap - pos : 0, fmt, ap);
  va_end(ap);
  if (n < 0) return 0;
  pos += (size_t)n;
  if (pos < cap) out[pos] = '\0';
  else out[cap - 1] = '\0';
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
  for (int i = 0; i < 8; ++i) {
    append(out, cap, pos, "%u", (unsigned)s.soil[i]);
    if (i < 7) append(out, cap, pos, ",");
  }
  append(out, cap, pos, "]}");
  return pos;
}

bool toJson(const msg::SensorV1& s, Print& out) {
  char buf[512];
  size_t n = toJson(s, buf, sizeof(buf));
  return out.write((const uint8_t*)buf, n) == n;
}
#endif // SERDE_ENABLE_JSON

// ===================== CBOR ENCODE =====================
#if SERDE_ENABLE_CBOR
static bool put(uint8_t* out, size_t cap, size_t& pos, uint8_t b) {
  if (pos >= cap) return false; out[pos++] = b; return true;
}
static bool putN(uint8_t* out, size_t cap, size_t& pos, const void* src, size_t n) {
  if (pos + n > cap) return false; memcpy(out + pos, src, n); pos += n; return true;
}
static bool emitTypeArg(uint8_t major, uint64_t val, uint8_t* out, size_t cap, size_t& pos) {
  if (val < 24) return put(out, cap, pos, (uint8_t)((major << 5) | val));
  if (val <= 0xFF)  { if (!put(out, cap, pos, (uint8_t)((major<<5)|24))) return false; return put(out, cap, pos, (uint8_t)val); }
  if (val <= 0xFFFF){ if (!put(out, cap, pos, (uint8_t)((major<<5)|25))) return false; uint16_t v=(uint16_t)val; uint8_t t[2]={(uint8_t)(v>>8),(uint8_t)v}; return putN(out,cap,pos,t,2); }
  { if (!put(out, cap, pos, (uint8_t)((major<<5)|26))) return false; uint32_t v=(uint32_t)val; uint8_t t[4]={(uint8_t)(v>>24),(uint8_t)(v>>16),(uint8_t)(v>>8),(uint8_t)v}; return putN(out,cap,pos,t,4); }
}
static bool emitUInt(uint64_t v, uint8_t* out, size_t cap, size_t& pos) { return emitTypeArg(0, v, out, cap, pos); }
static bool emitInt (int64_t  v, uint8_t* out, size_t cap, size_t& pos) {
  if (v >= 0) return emitUInt((uint64_t)v, out, cap, pos);
  uint64_t n = (uint64_t)(-1 - v); return emitTypeArg(1, n, out, cap, pos);
}
static bool emitArray(uint64_t n, uint8_t* out, size_t cap, size_t& pos) { return emitTypeArg(4, n, out, cap, pos); }
static bool emitMap  (uint64_t n, uint8_t* out, size_t cap, size_t& pos) { return emitTypeArg(5, n, out, cap, pos); }

size_t toCbor(const msg::SensorV1& s, uint8_t* out, size_t cap) {
  size_t pos = 0;
  if (!emitMap(13, out, cap, pos)) return 0;

  if (!emitUInt(13, out, cap, pos) || !emitUInt(msg::VERSION_SENSOR_V1, out, cap, pos)) return 0;
  if (!emitUInt(1,  out, cap, pos) || !emitUInt(s.ts_ms,           out, cap, pos)) return 0;
  if (!emitUInt(2,  out, cap, pos) || !emitInt (s.t_air_cX100,     out, cap, pos)) return 0;
  if (!emitUInt(3,  out, cap, pos) || !emitUInt(s.rh_x10,          out, cap, pos)) return 0;
  if (!emitUInt(4,  out, cap, pos) || !emitUInt(s.p_hpa_x10,       out, cap, pos)) return 0;
  if (!emitUInt(5,  out, cap, pos) || !emitUInt(s.light_lux_div10, out, cap, pos)) return 0;
  if (!emitUInt(6,  out, cap, pos) || !emitInt (s.t_sauna_cX100,   out, cap, pos)) return 0;
  if (!emitUInt(7,  out, cap, pos) || !emitInt (s.t_water_cX100,   out, cap, pos)) return 0;
  if (!emitUInt(8,  out, cap, pos) || !emitUInt(s.batt_mV,         out, cap, pos)) return 0;
  if (!emitUInt(9,  out, cap, pos) || !emitInt (s.rssi,            out, cap, pos)) return 0;
  if (!emitUInt(10, out, cap, pos) || !emitInt (s.snr,             out, cap, pos)) return 0;
  if (!emitUInt(11, out, cap, pos) || !emitUInt(s.soil_count,      out, cap, pos)) return 0;

  if (!emitUInt(12, out, cap, pos)) return 0;
  if (!emitArray(8, out, cap, pos)) return 0;
  for (int i = 0; i < 8; ++i) if (!emitUInt(s.soil[i], out, cap, pos)) return 0;

  return pos;
}
#endif // SERDE_ENABLE_CBOR

// ===================== JSON DECODE =====================
#if SERDE_ENABLE_JSON_DECODE
static const char* skip_ws(const char* p){ while (*p==' '||*p=='\n'||*p=='\r'||*p=='\t') ++p; return p; }

static bool parse_key_num(const char* json, const char* key, long& out) {
  char pat[32]; snprintf(pat, sizeof(pat), "\"%s\"", key);
  const char* k = strstr(json, pat); if (!k) return false;
  const char* p = strchr(k, ':');     if (!p) return false;
  p = skip_ws(p + 1);
  char* end = nullptr; long v = strtol(p, &end, 10);
  if (end == p) return false;
  out = v; return true;
}

static bool parse_key_uarray8(const char* json, const char* key, uint16_t out[8], uint8_t& count) {
  char pat[32]; snprintf(pat, sizeof(pat), "\"%s\"", key);
  const char* k = strstr(json, pat); if (!k) { count = 0; return false; }
  const char* p = strchr(k, ':');     if (!p) { count = 0; return false; }
  p = strchr(p, '[');                 if (!p) { count = 0; return false; }
  ++p; count = 0;
  while (*p && count < 8) {
    p = skip_ws(p);
    if (*p == ']') { ++p; break; }
    char* end = nullptr; unsigned long v = strtoul(p, &end, 10);
    if (end == p) break;
    out[count++] = (uint16_t)v;
    p = end;
    while (*p && *p != ']' && *p != ',') ++p;
    if (*p == ',') ++p;
  }
  for (int i = count; i < 8; ++i) out[i] = 0;
  return true;
}

bool fromJson(const char* json, msg::SensorV1& o) {
  if (!json) return false;
  memset(&o, 0, sizeof(o));

  long v = 0;
  if (parse_key_num(json, "ts", v))        o.ts_ms = (uint32_t)v;
  if (parse_key_num(json, "t_air", v))     o.t_air_cX100 = (int16_t)v;
  if (parse_key_num(json, "rh", v))        o.rh_x10 = (uint16_t)v;
  if (parse_key_num(json, "p", v))         o.p_hpa_x10 = (uint16_t)v;
  if (parse_key_num(json, "lux10", v))     o.light_lux_div10 = (uint16_t)v;
  if (parse_key_num(json, "t_sauna", v))   o.t_sauna_cX100 = (int16_t)v;
  if (parse_key_num(json, "t_water", v))   o.t_water_cX100 = (int16_t)v;
  if (parse_key_num(json, "batt", v))      o.batt_mV = (uint16_t)v;
  if (parse_key_num(json, "rssi", v))      o.rssi = (int8_t)v;
  if (parse_key_num(json, "snr", v))       o.snr  = (int8_t)v;

  // soil-array
  uint8_t parsed_len = 0;
  parse_key_uarray8(json, "soil", o.soil, parsed_len);

  // fallback: sista icke-noll-index + 1 om soil_count saknas
  uint8_t nz = 0;
  for (int i = 0; i < 8; ++i) if (o.soil[i] != 0) nz = (uint8_t)(i + 1);

  if (parse_key_num(json, "soil_count", v)) o.soil_count = (uint8_t)v;
  else                                       o.soil_count = nz;

  o._rsv0 = 0; o._pad = 0; o.crc8 = 0;
  return true;
}
#endif // SERDE_ENABLE_JSON_DECODE

// ===================== CBOR DECODE =====================
#if SERDE_ENABLE_CBOR_DECODE
struct CborR { const uint8_t* p; size_t n, i; };
static bool getb(CborR& r, uint8_t& b){ if (r.i >= r.n) return false; b = r.p[r.i++]; return true; }

static bool read_arg(uint8_t ai, uint64_t& v, CborR& r) {
  if (ai < 24) { v = ai; return true; }
  if (ai == 24) { uint8_t b; if(!getb(r,b)) return false; v = b; return true; }
  if (ai == 25) { uint8_t b1,b2; if(!getb(r,b1)||!getb(r,b2)) return false; v = ((uint16_t)b1<<8)|b2; return true; }
  if (ai == 26) { uint8_t b[4]; for(int k=0;k<4;k++) if(!getb(r,b[k])) return false;
                  v = ((uint32_t)b[0]<<24)|((uint32_t)b[1]<<16)|((uint32_t)b[2]<<8)|b[3]; return true; }
  return false;
}
static bool read_uint(CborR& r, uint64_t& v) {
  uint8_t ib; if (!getb(r,ib)) return false;
  uint8_t major = ib>>5, ai = ib & 0x1F;
  if (major == 0) return read_arg(ai, v, r);
  if (major == 1) { uint64_t n; if(!read_arg(ai,n,r)) return false; v = (uint64_t)(-1) - n; return true; }
  return false;
}
static bool read_sint(CborR& r, int64_t& sv) {
  uint8_t ib; if (!getb(r,ib)) return false;
  uint8_t major = ib>>5, ai = ib & 0x1F;
  if (major == 0) { uint64_t v; if(!read_arg(ai,v,r)) return false; sv = (int64_t)v; return true; }
  if (major == 1) { uint64_t n; if(!read_arg(ai,n,r)) return false; sv = -(int64_t)(n+1); return true; }
  return false;
}
static bool expect_array(CborR& r, uint64_t& len) { uint8_t ib; if(!getb(r,ib)) return false; if((ib>>5)!=4) return false; return read_arg(ib&0x1F, len, r); }
static bool expect_map  (CborR& r, uint64_t& len) { uint8_t ib; if(!getb(r,ib)) return false; if((ib>>5)!=5) return false; return read_arg(ib&0x1F, len, r); }

bool fromCbor(const uint8_t* buf, size_t len, msg::SensorV1& o) {
  if (!buf || !len) return false;
  memset(&o, 0, sizeof(o));

  bool seen_soil = false;
  bool seen_soil_count = false;
  uint8_t soil_nz = 0;

  CborR r{buf, len, 0};
  uint64_t mlen = 0;
  if (!expect_map(r, mlen)) return false;

  for (uint64_t m = 0; m < mlen; ++m) {
    uint64_t key = 0;
    if (!read_uint(r, key)) return false;

    switch (key) {
      case 13: { uint64_t v; if(!read_uint(r,v)) return false; /* ver */ break; }
      case 1:  { uint64_t v; if(!read_uint(r,v)) return false; o.ts_ms = (uint32_t)v; break; }
      case 2:  { int64_t  v; if(!read_sint(r,v)) return false; o.t_air_cX100 = (int16_t)v; break; }
      case 3:  { uint64_t v; if(!read_uint(r,v)) return false; o.rh_x10 = (uint16_t)v; break; }
      case 4:  { uint64_t v; if(!read_uint(r,v)) return false; o.p_hpa_x10 = (uint16_t)v; break; }
      case 5:  { uint64_t v; if(!read_uint(r,v)) return false; o.light_lux_div10 = (uint16_t)v; break; }
      case 6:  { int64_t  v; if(!read_sint(r,v)) return false; o.t_sauna_cX100 = (int16_t)v; break; }
      case 7:  { int64_t  v; if(!read_sint(r,v)) return false; o.t_water_cX100 = (int16_t)v; break; }
      case 8:  { uint64_t v; if(!read_uint(r,v)) return false; o.batt_mV = (uint16_t)v; break; }
      case 9:  { int64_t  v; if(!read_sint(r,v)) return false; o.rssi = (int8_t)v; break; }
      case 10: { int64_t  v; if(!read_sint(r,v)) return false; o.snr  = (int8_t)v; break; }
      case 11: { // soil_count
        uint64_t v; if(!read_uint(r,v)) return false;
        o.soil_count = (uint8_t)v;
        seen_soil_count = true;
        break;
      }
      case 12: { // soil array
        uint64_t alen = 0;
        if (!expect_array(r, alen)) return false;
        for (uint8_t i = 0; i < 8; ++i) {
          if (i < alen) {
            uint64_t v; if(!read_uint(r,v)) return false;
            o.soil[i] = (uint16_t)v;
            if (o.soil[i] != 0) soil_nz = (uint8_t)(i + 1);
          } else {
            o.soil[i] = 0;
          }
        }
        // konsumera ev. extra element > 8
        for (uint64_t i = 8; i < alen; ++i) { uint64_t dummy; if(!read_uint(r,dummy)) return false; }
        seen_soil = true;
        break;
      }
      default: {
        // Hoppa över okända värden (enkel konsumtion av uint/int/array/map med heltal)
        uint8_t ib; if(!getb(r,ib)) return false;
        uint8_t major = ib>>5, ai = ib & 0x1F;
        if (major == 0 || major == 1) { uint64_t dummy; if(!read_arg(ai, dummy, r)) return false; }
        else if (major == 4) { uint64_t al; if(!read_arg(ai, al, r)) return false;
          for (uint64_t i=0;i<al;i++){ uint64_t dummy; if(!read_uint(r,dummy)) return false; } }
        else if (major == 5) { uint64_t ml; if(!read_arg(ai, ml, r)) return false;
          for (uint64_t i=0;i<ml;i++){ uint64_t k; if(!read_uint(r,k)) return false; uint64_t v; if(!read_uint(r,v)) return false; } }
        else { return false; }
      }
    }
  }

  // fallback: om soil fanns men soil_count saknades → sätt sista icke-noll + 1
  if (seen_soil && !seen_soil_count) {
    o.soil_count = soil_nz;
  }

  o._rsv0 = 0; o._pad = 0; o.crc8 = 0;
  return true;
}
#endif // SERDE_ENABLE_CBOR_DECODE

} // namespace serde

