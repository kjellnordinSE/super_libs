//lib_lora/test/frame_roundtrip/test_main.cpp
#include <Arduino.h>
#include <unity.h>
#include <lora.h>

void test_pack_unpack() {
  lora::FrameHeader h{};
  h.ver=1; h.type=2; h.seq=123; h.src=0xBEEF; h.flags=0x5A;
  uint8_t payload[5] = {1,2,3,4,5};
  uint8_t frame[64];

  size_t n = lora::packFrame(h, payload, sizeof(payload), frame, sizeof(frame));
  TEST_ASSERT_TRUE(n > 0);

  const uint8_t* pl=nullptr; size_t plen=0; lora::FrameHeader g{};
  TEST_ASSERT_TRUE( lora::unpackFrame(frame, n, g, pl, plen) );

  TEST_ASSERT_EQUAL_UINT8('M', g.magic0);
  TEST_ASSERT_EQUAL_UINT8('1', g.magic1);
  TEST_ASSERT_EQUAL_UINT8(1, g.ver);
  TEST_ASSERT_EQUAL_UINT8(2, g.type);
  TEST_ASSERT_EQUAL_UINT16(123, g.seq);
  TEST_ASSERT_EQUAL_UINT16(0xBEEF, g.src);
  TEST_ASSERT_EQUAL_UINT8(0x5A, g.flags);
  TEST_ASSERT_EQUAL_UINT(5, plen);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, pl, plen);
}

void setup(){ delay(100); UNITY_BEGIN(); RUN_TEST(test_pack_unpack); UNITY_END(); }
void loop(){}

