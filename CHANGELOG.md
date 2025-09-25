# Changelog — super_libs

## lib_message v0.1.0 — First cut
- Fixed-size SensorV1 (48 B total)
- Header (8 B) med magic/version/type/seq/flags
- Little-endian trådformat, manuell encode/decode
- CRC8 (Dallas/Maxim) över body
- Exempel: message_print
- Unity-test: message_roundtrip
