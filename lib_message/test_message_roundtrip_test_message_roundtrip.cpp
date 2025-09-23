
## Vad betyder “roundtrip” och hur kör jag testet?

**Roundtrip-test** = vi **encodar** en struct → får bytes → **decodar** tillbaka → och **verifierar** att fälten blev samma (plus validerar CRC). Det är ett snabbt sätt att bevisa att våra encoder/decoder-funktioner är symmetriska och att formatet håller.

### Så här kör du Unity-testet (enklaste vägen)

1) Välj ett **PlatformIO-projekt** (vilket som helst där du redan byggt mot din bräda).
2) Lägg in testfilen i det projektets `test/`:
<ditt-projekt>/
platformio.ini
test/
message_roundtrip/
test_message_roundtrip.cpp (klistra in filen ovan)

javascript
Kopiera kod
3) Se till att projektets `platformio.ini` pekar ut `super_libs`:
```ini
lib_extra_dirs = ${sysenv.PLATFORMIO_LIBS}
lib_ldf_mode   = chain+
Kör testet:

bash
Kopiera kod
pio test -e esp32dev
(Byt esp32dev till ditt env-namn.)

Förväntad output (typiskt):

diff
Kopiera kod
test_message_roundtrip.cpp:...: test_roundtrip [PASSED]
==================== 1 Tests 0 Failures ====================
Tips: Vill du köra testet inuti examples/message_print i libbet? Skapa en test/ mapp där också och kör pio test i den katalogen – PlatformIO kör tester per projekt (där platformio.ini ligger).



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
