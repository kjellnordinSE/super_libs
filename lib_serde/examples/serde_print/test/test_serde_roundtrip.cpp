//lib_serde/examples/serde_print/test/test_serde_roundtrip.cpp
#include <Arduino.h>
#include <unity.h>
#include <message.h>
#include <serde.h>

static msg::SensorV1 mkSample() {
  msg::SensorV1 s{};
  s.ts_ms           = 123456;
  s.t_air_cX100     = 2345;
  s.rh_x10          = 553;
  s.p_hpa_x10       = 10132;
  s.light_lux_div10 = 1234;
  s.t_sauna_cX100   = 0;
  s.t_water_cX100   = 0;
  s.batt_mV         = 3790;
  s.rssi            = -70;
  s.snr             = 8;
  s.soil_count      = 2;
  s.soil[0]         = 3123;
  s.soil[1]         = 2890;
  return s;
}

void test_json_roundtrip() {
#if SERDE_ENABLE_JSON && SERDE_ENABLE_JSON_DECODE
  auto s = mkSample();
  char json[512];
  size_t n = serde::toJson(s, json, sizeof(json));
  TEST_ASSERT_TRUE(n > 0);

  msg::SensorV1 t{};
  TEST_ASSERT_TRUE( serde::fromJson(json, t) );

  TEST_ASSERT_EQUAL_UINT32(s.ts_ms, t.ts_ms);
  TEST_ASSERT_EQUAL_INT16(s.t_air_cX100, t.t_air_cX100);
  TEST_ASSERT_EQUAL_UINT16(s.rh_x10, t.rh_x10);
  TEST_ASSERT_EQUAL_UINT16(s.p_hpa_x10, t.p_hpa_x10);
  TEST_ASSERT_EQUAL_UINT16(s.light_lux_div10, t.light_lux_div10);
  TEST_ASSERT_EQUAL_INT16(s.t_sauna_cX100, t.t_sauna_cX100);
  TEST_ASSERT_EQUAL_INT16(s.t_water_cX100, t.t_water_cX100);
  TEST_ASSERT_EQUAL_UINT16(s.batt_mV, t.batt_mV);
  TEST_ASSERT_EQUAL_INT8(s.rssi, t.rssi);
  TEST_ASSERT_EQUAL_INT8(s.snr, t.snr);
  TEST_ASSERT_EQUAL_UINT8(s.soil_count, t.soil_count);
  TEST_ASSERT_EQUAL_UINT16(s.soil[0], t.soil[0]);
  TEST_ASSERT_EQUAL_UINT16(s.soil[1], t.soil[1]);
#endif
}

void test_cbor_roundtrip() {
#if SERDE_ENABLE_CBOR && SERDE_ENABLE_CBOR_DECODE
  auto s = mkSample();
  uint8_t buf[128];
  size_t n = serde::toCbor(s, buf, sizeof(buf));
  TEST_ASSERT_TRUE(n > 0);

  msg::SensorV1 t{};
  TEST_ASSERT_TRUE( serde::fromCbor(buf, n, t) );

  TEST_ASSERT_EQUAL_UINT32(s.ts_ms, t.ts_ms);
  TEST_ASSERT_EQUAL_INT16(s.t_air_cX100, t.t_air_cX100);
  TEST_ASSERT_EQUAL_UINT16(s.rh_x10, t.rh_x10);
  TEST_ASSERT_EQUAL_UINT16(s.p_hpa_x10, t.p_hpa_x10);
  TEST_ASSERT_EQUAL_UINT16(s.light_lux_div10, t.light_lux_div10);
  TEST_ASSERT_EQUAL_UINT16(s.batt_mV, t.batt_mV);
  TEST_ASSERT_EQUAL_INT8(s.rssi, t.rssi);
  TEST_ASSERT_EQUAL_INT8(s.snr, t.snr);
  TEST_ASSERT_EQUAL_UINT8(s.soil_count, t.soil_count);
  TEST_ASSERT_EQUAL_UINT16(s.soil[0], t.soil[0]);
  TEST_ASSERT_EQUAL_UINT16(s.soil[1], t.soil[1]);
#endif
}

void test_json_missing_fields_defaults_zero() {
#if SERDE_ENABLE_JSON_DECODE
  // saknar flera fält (ska bli 0)
  const char* j = "{\"ver\":1,\"ts\":42,\"soil\":[7,0,0,0,0,0,0,0]}";
  msg::SensorV1 t{};
  TEST_ASSERT_TRUE( serde::fromJson(j, t) );
  TEST_ASSERT_EQUAL_UINT32(42, t.ts_ms);
  TEST_ASSERT_EQUAL_INT16(0, t.t_air_cX100);
  TEST_ASSERT_EQUAL_UINT16(0, t.rh_x10);
  TEST_ASSERT_EQUAL_UINT16(0, t.p_hpa_x10);
  TEST_ASSERT_EQUAL_UINT16(0, t.light_lux_div10);
  TEST_ASSERT_EQUAL_UINT16(7, t.soil[0]);
  TEST_ASSERT_EQUAL_UINT8(1, t.soil_count); // fallback: om soil_count saknas → faktiska arraylängden
#endif
}

void setup() {
  delay(200);
  UNITY_BEGIN();
  RUN_TEST(test_json_roundtrip);
  RUN_TEST(test_cbor_roundtrip);
  RUN_TEST(test_json_missing_fields_defaults_zero);
  UNITY_END();
}

void loop() {}

