# lib_identity

Litet bibliotek som ger varje ESP-nod en tydlig identitet:
- ChipID (från eFUSE MAC)
- Wi-Fi MAC i flera format (kolon, hex utan kolon, bytes)
- Lokal IP (STA/AP om tillgängligt)
- **LoRaWAN-vänlig DevEUI** härledd från MAC via FF:FE (EUI-64)
- Tydlig boot-banner

## Snabbstart

```cpp
#include <Arduino.h>
#include <identity.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);

  identity::begin();       // valfritt, idempotent
  identity::printBootInfo();
}

void loop() {}


## Installation

Lägg biblioteket i din repo, t.ex.:


I ditt PlatformIO-projekt, peka ut libs-mappen:

```ini
; platformio.ini
[env:esp32dev]
platform      = espressif32
board         = esp32dev
framework     = arduino
monitor_speed = 115200

lib_extra_dirs = ../libs
lib_ldf_mode   = chain+
build_flags    = -D CORE_DEBUG_LEVEL=1



Snabbstart (identity_print)

src/main.cpp

#include <Arduino.h>
#include <identity.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);

  identity::begin();       // (valfri, idempotent)
  identity::printBootInfo();
}

void loop() {
  delay(5000);
  // identity::printBootInfo();  // avkommentera om du vill skriva regelbundet
}

