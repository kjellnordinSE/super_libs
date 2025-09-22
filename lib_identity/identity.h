// identity.h
#pragma once
#include <Arduino.h>

namespace identity {
  void   begin();            // valfri init
  String getChipID();        // unik ID-sträng
  String getMAC();           // Wi-Fi MAC som text (98:3D:AE:...)
  String getLocalIP();       // "-" om ej ansluten
  String getMacNoColon();              // "983DAE614570"
  void   getMacBytes(uint8_t out[6]);  // {0x98,0x3D,0xAE,0x61,0x45,0x70}
  void   printBootInfo();              // banner till Serial

  // --- DevEUI helpers (standard: OUI + FF:FE + NIC) ---
  void   getDevEuiFFFE(uint8_t out_msb[8]);     // bytes i MSB-ordning
  void   getDevEuiFFFE_LSB(uint8_t out_lsb[8]); // bytes i LSB-ordning (reverse)
  String getDevEuiFFFE_MSB();                   // 16 hex, utan kolon
  String getDevEuiFFFE_LSB();                   // 16 hex, utan kolon
}

