
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

