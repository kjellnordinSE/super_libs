// identity.cpp
#include "identity.h"
#include <WiFi.h>

namespace {

String chipIdFromEfuse() {
  // Bygg en kompakt chip-ID-sträng från eFUSE MAC
  uint64_t id = ESP.getEfuseMac();
  char buf[17];  // 16 hex + NUL
  sprintf(buf, "%04X%08X", (uint16_t)(id >> 32), (uint32_t)id);
  return String(buf);
}

String hexStr(const uint8_t* bytes, size_t len, bool reverse) {
  char buf[2 * 8 + 1]; // stöd för upp till 8 bytes => 16 tecken + NUL
  size_t idx = 0;
  if (!reverse) {
    for (size_t i = 0; i < len; ++i) idx += sprintf(buf + idx, "%02X", bytes[i]);
  } else {
    for (size_t i = 0; i < len; ++i) idx += sprintf(buf + idx, "%02X", bytes[len - 1 - i]);
  }
  buf[idx] = '\0';
  return String(buf);
}

} // namespace

void identity::begin() {
  // Idempotent init – lämna tom nu (ev. hostname senare)
}

String identity::getChipID() {
  return chipIdFromEfuse();
}

String identity::getMAC() {
  return WiFi.macAddress(); // "98:3D:AE:61:45:70"
}

String identity::getLocalIP() {
  if (WiFi.status() == WL_CONNECTED) return WiFi.localIP().toString();
  if (WiFi.getMode() & WIFI_AP)      return WiFi.softAPIP().toString();
  return String("-");
}

String identity::getMacNoColon() {
  String s = WiFi.macAddress(); // "98:3D:AE:61:45:70"
  String out; out.reserve(12);
  for (size_t i = 0; i < s.length(); ++i) if (s[i] != ':') out += s[i];
  return out; // "983DAE614570"
}

void identity::getMacBytes(uint8_t out[6]) {
  // Använd WiFi.macAddress() för korrekt visningsordning → parsa till bytes
  String s = WiFi.macAddress();
  int v[6] = {0};
  sscanf(s.c_str(), "%x:%x:%x:%x:%x:%x", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]);
  for (int i = 0; i < 6; ++i) out[i] = (uint8_t)v[i];
}

// ---------- DevEUI (FF:FE) – standard EUI-64 ----------
void identity::getDevEuiFFFE(uint8_t out_msb[8]) {
  uint8_t m[6]; getMacBytes(m);
  // MSB-ordning: OUI(3) + FF FE + NIC(3)
  out_msb[0] = m[0];
  out_msb[1] = m[1];
  out_msb[2] = m[2];
  out_msb[3] = 0xFF;
  out_msb[4] = 0xFE;
  out_msb[5] = m[3];
  out_msb[6] = m[4];
  out_msb[7] = m[5];
}

void identity::getDevEuiFFFE_LSB(uint8_t out_lsb[8]) {
  uint8_t msb[8]; getDevEuiFFFE(msb);
  for (int i = 0; i < 8; ++i) out_lsb[i] = msb[7 - i];
}

String identity::getDevEuiFFFE_MSB() {
  uint8_t e[8]; getDevEuiFFFE(e);
  return hexStr(e, 8, /*reverse=*/false);
}

String identity::getDevEuiFFFE_LSB() {
  uint8_t e[8]; getDevEuiFFFE_LSB(e);
  return hexStr(e, 8, /*reverse=*/false); // arrayen är redan LSB
}

void identity::printBootInfo() {
  String chip = getChipID();
  String macC = getMAC();
  String macN = getMacNoColon();
  uint8_t b[6]; getMacBytes(b);

  // DevEUI (standard FFFE)
  String dev_msb = getDevEuiFFFE_MSB();
  String dev_lsb = getDevEuiFFFE_LSB();

  Serial.println(F("========== IDENTITY =========="));
  Serial.printf("ChipID : %s\n", chip.c_str());
  Serial.printf("MAC    : %s\n", macC.c_str());
  Serial.printf("MACHEX : %s\n", macN.c_str());
  Serial.printf("MACARR : {0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}\n",
                b[0], b[1], b[2], b[3], b[4], b[5]);

  Serial.println(F("----- LoRa DevEUI (from MAC) -----"));
  Serial.printf("DEVEUI FFFE MSB : %s\n", dev_msb.c_str());
  Serial.printf("DEVEUI FFFE LSB : %s\n", dev_lsb.c_str());
  Serial.println(F("=============================="));
}

