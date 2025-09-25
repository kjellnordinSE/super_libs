//lib_serde/examples/serde_print/src/main.cpp
#include <Arduino.h>
#include <message.h>
#include <serde.h>

void setup() {
  Serial.begin(115200);
  while(!Serial){}
  delay(200);

  msg::SensorV1 s{};
  s.ts_ms           = millis();
  s.t_air_cX100     = 2345;
  s.rh_x10          = 553;
  s.p_hpa_x10       = 10132;
  s.light_lux_div10 = 1234;
  s.batt_mV         = 3790;
  s.rssi            = -70;
  s.snr             = 8;
  s.soil_count      = 2;
  s.soil[0]         = 3123;
  s.soil[1]         = 2890;

  // JSON
  char json[512];
  size_t jn = serde::toJson(s, json, sizeof(json));
  Serial.printf("JSON (%u B): %s\n", (unsigned)jn, json);

  // CBOR
  uint8_t cbor[128];
  size_t cn = serde::toCbor(s, cbor, sizeof(cbor));
  Serial.printf("CBOR (%u B): ", (unsigned)cn);
  for (size_t i=0;i<cn;i++){ Serial.printf("%02X ", cbor[i]); }
  Serial.println();
}

void loop(){}

