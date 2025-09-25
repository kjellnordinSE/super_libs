lib_message/README.md
# lib_message

Fasta, versionerade paket för sensordata – enkla att sända över ESP-NOW, LoRa (P2P) och möjliga att hålla inom LoRaWAN-MTU. **V1** är 48 byte totalt.

## Varför fast storlek?
- **Förutsägbar MTU** och enklare felsökning.
- **Nollor** där data saknas – paketets storlek ändras aldrig.
- **Versionerat header** → framtida V2/V3 kan samexistera.

## Snabbstart

```cpp
#include <message.h>

static uint16_t g_seq = 0;

void setup() {
  Serial.begin(115200);
  msg::FrameHeader h; 
  msg::makeHeader(h, msg::VERSION_SENSOR_V1, msg::TYPE_SENSOR, g_seq++);

  msg::SensorV1 s{};
  s.ts_ms           = millis();
  s.t_air_cX100     = 2345;   // 23.45°C
  s.rh_x10          = 553;    // 55.3%
  s.p_hpa_x10       = 10132;  // 1013.2 hPa
  s.batt_mV         = 3790;
  s.soil_count      = 2;
  s.soil[0]         = 3123;

  uint8_t buf[msg::SENSOR_V1_TOTAL];
  size_t n = msg::encodeSensorV1(h, s, buf);
  Serial.printf("Encoded %u bytes\n", (unsigned)n);
}


Se examples/message_print för komplett exempel (encode + decode + utskrift).

Design

Header (8 B): magic('M','1'), version(1), type(1), seq(2 LE), flags(2).

Body V1 (40 B): skalade heltal (temp×100, RH×10, tryck×10, …), 8 st soil, samt CRC8 över body.

Totalt: 48 B (ryms i LoRaWAN DR0 EU868 ≈ 51 B).

Endianness: little-endian på “tråden”. Vi skriver/läser fält manuellt i encode/decode – inga packade C-structs över radion.

CRC: Dallas/Maxim CRC8 (poly 0x31, init 0x00). Flagga flags bit0 anger att CRC finns (V1 = alltid 1).

Naming convention (förslag)

Namespace: msg

Typer: FrameHeader, SensorV1

Konstanter (UPPER_SNAKE): MAGIC0, VERSION_SENSOR_V1, FRAME_HDR_SIZE …

Storlekar: FRAME_HDR_SIZE, SENSOR_V1_BODY_SZ, SENSOR_V1_TOTAL

Skalfaktorer i fältnamn: t_air_cX100, rh_x10, p_hpa_x10

Versionering

V1 låses på 48 B total längd.

Nya fält → V2 med ny body-layout och uppdaterad version. Transportsidan läser version och väljer decoder.

Beroenden

Endast Arduino.h och <stdint.h>.


---

# `lib_message/API_message.md`
```md
# API-kontrakt — lib_message

Målet: fasta, versionerade paket med förutsägbar storlek; robusta över flera radio/protokoll.

## Storlekar
- **Header** = 8 B  
- **Body V1** = 40 B  
- **Total V1** = 48 B

## Header (8 B)


0: magic0 ('M')
1: magic1 ('1')
2: version (=1 för denna spec)
3: type (=1 TYPE_SENSOR)
4..5: seq (uint16 LE)
6..7: flags (uint16 LE) — bit0: CRC8 i body finns (V1: alltid 1)


## Body: SensorV1 (40 B, little-endian fält)
Fälten skrivs/läses manuellt (wr16LE/rd16LE etc.). Inga packade C-structs över “tråden”.

| Offset | Typ    | Fält                | Skala / Kommentar                  |
|------:|--------|---------------------|------------------------------------|
| 0     | u32    | ts_ms               | millis() (0=okänd)                 |
| 4     | i16    | t_air_cX100         | 23.45°C → 2345 (0=saknas)          |
| 6     | u16    | rh_x10              | 55.3% → 553 (0=saknas)             |
| 8     | u16    | p_hpa_x10           | 1013.2 hPa → 10132 (0=saknas)      |
| 10    | u16    | light_lux_div10     | Lux/10 (0=saknas)                  |
| 12    | i16    | t_sauna_cX100       | 0=saknas                           |
| 14    | i16    | t_water_cX100       | 0=saknas                           |
| 16    | u16    | batt_mV             | mV (0=saknas)                      |
| 18    | i8     | rssi                | dBm (0=okänd)                      |
| 19    | i8     | snr                 | dB (0=okänd)                       |
| 20    | u8     | soil_count          | antal giltiga soil-värden (0..8)   |
| 21    | u8     | _rsv0               | reserverad (0)                     |
| 22    | u16[8] | soil                | 8 kanaler, 0=saknas                |
| 38    | u8     | crc8                | Dallas/Maxim CRC8 över body[0..37] |
| 39    | u8     | _pad                | padding (0)                        |

## API
```cpp
namespace msg {
  // Konstanter: MAGIC0, MAGIC1, VERSION_SENSOR_V1, TYPE_SENSOR,
  //             FRAME_HDR_SIZE, SENSOR_V1_BODY_SZ, SENSOR_V1_TOTAL

  struct FrameHeader {
    uint8_t  magic0, magic1, version, type;
    uint16_t seq, flags;
  };

  struct SensorV1 { /* se headerfil */ };

  void   makeHeader(FrameHeader& h, uint8_t version, uint8_t type, uint16_t seq, uint16_t flags = 0x0001);
  size_t encodeSensorV1(const FrameHeader& h, const SensorV1& s, uint8_t out[SENSOR_V1_TOTAL]);
  bool   decodeSensorV1(const uint8_t in[SENSOR_V1_TOTAL], FrameHeader& h_out, SensorV1& s_out);
  uint8_t crc8_dallas(const uint8_t* data, size_t len);
}

Felhantering

decodeSensorV1 returnerar false om magic/version/type inte matchar eller om CRC8 inte stämmer (när flags bit0 är satt).

Versionspolicy

V1 är låst (48 B). Nya fält → V2 med ny body-layout och uppdaterad version.


---

# `lib_message/examples/message_print/platformio.ini`
```ini
[env:esp32dev]
platform = espressif32
board    = esp32dev
framework = arduino
monitor_speed = 115200

; justera om din super_libs ligger annorlunda
lib_extra_dirs = ../../..
lib_ldf_mode   = chain+

lib_message/examples/message_print/src/main.cpp
#include <Arduino.h>
#include <message.h>

static uint16_t g_seq = 0;

static void hexdump(const uint8_t* p, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    if (i && !(i % 16)) Serial.println();
    Serial.printf("%02X ", p[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);

  msg::FrameHeader h;
  msg::makeHeader(h, msg::VERSION_SENSOR_V1, msg::TYPE_SENSOR, g_seq++);

  msg::SensorV1 s{};
  s.ts_ms           = millis();
  s.t_air_cX100     = 2345;   // 23.45°C
  s.rh_x10          = 553;    // 55.3%
  s.p_hpa_x10       = 10132;  // 1013.2 hPa
  s.light_lux_div10 = 1234;   // ≈ 12340 lux
  s.t_sauna_cX100   = 0;
  s.t_water_cX100   = 0;
  s.batt_mV         = 3790;
  s.rssi            = -70;
  s.snr             = 8;
  s.soil_count      = 2;
  s.soil[0]         = 3123;
  s.soil[1]         = 2890;

  uint8_t buf[msg::SENSOR_V1_TOTAL];
  size_t n = msg::encodeSensorV1(h, s, buf);
  Serial.printf("Encoded %u bytes\n", (unsigned)n);
  hexdump(buf, n);

  msg::FrameHeader h2;
  msg::SensorV1    s2{};
  bool ok = msg::decodeSensorV1(buf, h2, s2);
  Serial.printf("Decode: %s, seq=%u, t_air=%.2f°C, rh=%.1f%%, soil0=%u\n",
    ok ? "OK" : "FAIL",
    h2.seq,
    s2.t_air_cX100/100.0f,
    s2.rh_x10/10.0f,
    s2.soil[0]
  );
}

void loop() {
  delay(2000);
}

lib_message/test/message_roundtrip/test_message_roundtrip.cpp
#include <Arduino.h>
#include <unity.h>
#include <message.h>

void setUp() {}
void tearDown() {}

void test_roundtrip() {
  msg::FrameHeader h;
  msg::makeHeader(h, msg::VERSION_SENSOR_V1, msg::TYPE_SENSOR, 42);

  msg::SensorV1 s{};
  s.ts_ms = 123456;
  s.t_air_cX100 = 2501;
  s.rh_x10 = 503;
  s.p_hpa_x10 = 10012;
  s.batt_mV = 3700;
  s.soil_count = 1;
  s.soil[0] = 4321;

  uint8_t buf[msg::SENSOR_V1_TOTAL];
  size_t n = msg::encodeSensorV1(h, s, buf);
  TEST_ASSERT_EQUAL_UINT32(msg::SENSOR_V1_TOTAL, n);

  msg::FrameHeader h2{};
  msg::SensorV1 s2{};
  bool ok = msg::decodeSensorV1(buf, h2, s2);
  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_UINT16(42, h2.seq);
  TEST_ASSERT_EQUAL_INT16(2501, s2.t_air_cX100);
  TEST_ASSERT_EQUAL_UINT16(503, s2.rh_x10);
  TEST_ASSERT_EQUAL_UINT16(4321, s2.soil[0]);
}

void setup() {
  delay(1000);
  UNITY_BEGIN();
  RUN_TEST(test_roundtrip);
  UNITY_END();
}

void loop() {}

