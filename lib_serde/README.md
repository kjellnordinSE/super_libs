# lib_serde

Konverterar `msg::SensorV1` till **JSON** och **CBOR** för loggning, MQTT och integration. Wire-formatet (48B) i `lib_message` lämnas orört.

## API (kort)
```cpp
namespace serde {
// JSON
size_t toJson(const msg::SensorV1& s, char* out, size_t cap);
bool   toJson(const msg::SensorV1& s, Print& out);

// CBOR (map med int-nycklar: 1..13)
size_t toCbor(const msg::SensorV1& s, uint8_t* out, size_t cap);
}

Build-flaggor

I platformio.ini kan du styra vad som byggs:

build_flags =
  -DSERDE_ENABLE_JSON
  -DSERDE_ENABLE_CBOR

Exempel

Se examples/serde_print/.


---

# `lib_serde/serde.h`
```cpp
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

namespace serde {

// JSON (skriver alla fält, numeriska; soil som array[8])
#if SERDE_ENABLE_JSON
size_t toJson(const msg::SensorV1& s, char* out, size_t cap);
bool   toJson(const msg::SensorV1& s, Print& out);
#endif

// CBOR (map med int-nycklar; 1:ts,2:t_air,3:rh,4:p,5:lux10,6:t_sauna,7:t_water,
//       8:batt,9:rssi,10:snr,11:soil_count,12:soil[8],13:ver)
#if SERDE_ENABLE_CBOR
size_t toCbor(const msg::SensorV1& s, uint8_t* out, size_t cap);
#endif

} // namespace serde
