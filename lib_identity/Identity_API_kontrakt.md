API (översikt)

namespace identity {
  void   begin();
  String getChipID();
  String getMAC();
  String getLocalIP();
  String getMacNoColon();
  void   getMacBytes(uint8_t out[6]);

  void   getDevEuiFFFE(uint8_t out_msb[8]);
  void   getDevEuiFFFE_LSB(uint8_t out_lsb[8]);
  String getDevEuiFFFE_MSB();
  String getDevEuiFFFE_LSB();

  void   printBootInfo();
}


Notera om DevEUI:

Standard EUI-64 skapas som OUI(3) + FF FE + NIC(3).

Portaler (TTN/Helium) visar oftast MSB. Vissa Arduino-libs vill ha LSB – därför skriver vi ut båda.

Beroenden

Arduino.h

WiFi.h (för MAC och IP)


---

## API_identity.md

```md
# API-kontrakt — lib_identity

## Namespace
`namespace identity`

## Init
```cpp
void begin();  // idempotent

Getters

String getChipID();   // 12 hex (VERSALER), t.ex. "704561AE3D98"
String getMAC();      // "98:3D:AE:61:45:70"
String getLocalIP();  // IP eller "-"

String  getMacNoColon();             // "983DAE614570"
void    getMacBytes(uint8_t out[6]); // {0x98,0x3D,0xAE,0x61,0x45,0x70}

void   getDevEuiFFFE(uint8_t out_msb[8]);     // OUI + FF:FE + NIC
void   getDevEuiFFFE_LSB(uint8_t out_lsb[8]); // omvänd byteordning
String getDevEuiFFFE_MSB();                   // 16 hex, utan kolon
String getDevEuiFFFE_LSB();                   // 16 hex, utan kolon

void printBootInfo();

Skriver blocket:

========== IDENTITY ==========
ChipID : <chipIdHex>
MAC    : <mac>
MACHEX : <macHexNoColon>
MACARR : {0x..,0x..,0x..,0x..,0x..,0x..}
----- LoRa DevEUI (from MAC) -----
DEVEUI FFFE MSB : <hex16>
DEVEUI FFFE LSB : <hex16>
==============================

