//lib_serde/serde.h
#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <message.h>

#ifndef SERDE_ENABLE_JSON
#define SERDE_ENABLE_JSON 1
#endif
#ifndef SERDE_ENABLE_CBOR
#define SERDE_ENABLE_CBOR 1
#endif

// nya: styr decode separat om du vill spara flash
#ifndef SERDE_ENABLE_JSON_DECODE
#define SERDE_ENABLE_JSON_DECODE 1
#endif
#ifndef SERDE_ENABLE_CBOR_DECODE
#define SERDE_ENABLE_CBOR_DECODE 1
#endif

namespace serde {

// ENCODE (du har redan dessa)
#if SERDE_ENABLE_JSON
size_t toJson(const msg::SensorV1& s, char* out, size_t cap);
bool   toJson(const msg::SensorV1& s, Print& out);
#endif
#if SERDE_ENABLE_CBOR
size_t toCbor(const msg::SensorV1& s, uint8_t* out, size_t cap);
#endif

// DECODE (nya)
#if SERDE_ENABLE_JSON_DECODE
// förväntar JSON i vårt "egenproducerade" format; saknade fält → lämnas 0
bool   fromJson(const char* json, msg::SensorV1& out);
#endif

#if SERDE_ENABLE_CBOR_DECODE
// förväntar CBOR map med int-nycklar (1..13) som vi använder; ok med extra nycklar
bool   fromCbor(const uint8_t* buf, size_t len, msg::SensorV1& out);
#endif

} // namespace serde

