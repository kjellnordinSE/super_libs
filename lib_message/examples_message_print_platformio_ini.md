
---

# `examples/message_print/platformio.ini`
```ini
[env:esp32dev]
platform = espressif32
board    = esp32dev
framework = arduino
monitor_speed = 115200

; justera om din super_libs ligger annorlunda
lib_extra_dirs = ../../..
lib_ldf_mode   = chain+


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




