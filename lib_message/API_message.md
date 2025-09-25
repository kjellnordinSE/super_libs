
---

# `API_message.md`
```md
# API-kontrakt — lib_message

Målet: fasta, versionerade paket med förutsägbar storlek; robusta över flera radio/protokoll.

## Storlekar
- **Header** = 8 B  
- **Body V1** = 40 B  
- **Total V1** = 48 B

## Header (8 B)


0: magic0 ('M')
1: magic1 ('1')
2: version (=1 för denna spec)
3: type (=1 TYPE_SENSOR)
4..5: seq (uint16 LE)
6..7: flags (uint16 LE) — bit0: CRC8 i body finns (V1: alltid 1)


## Body: SensorV1 (40 B, little-endian fält)
Fälten skrivs/läses manuellt (wr16LE/rd16LE etc.). Inga packade C-structs över “tråden”.

| Offset | Typ    | Fält                | Skala / Kommentar                  |
|------:|--------|---------------------|------------------------------------|
| 0     | u32    | ts_ms               | millis() (0=okänd)                 |
| 4     | i16    | t_air_cX100         | 23.45°C → 2345 (0=saknas)          |
| 6     | u16    | rh_x10              | 55.3% → 553 (0=saknas)             |
| 8     | u16    | p_hpa_x10           | 1013.2 hPa → 10132 (0=saknas)      |
| 10    | u16    | light_lux_div10     | Lux/10 (0=saknas)                  |
| 12    | i16    | t_sauna_cX100       | 0=saknas                           |
| 14    | i16    | t_water_cX100       | 0=saknas                           |
| 16    | u16    | batt_mV             | mV (0=saknas)                      |
| 18    | i8     | rssi                | dBm (0=okänd)                      |
| 19    | i8     | snr                 | dB (0=okänd)                       |
| 20    | u8     | soil_count          | antal giltiga soil-värden (0..8)   |
| 21    | u8     | _rsv0               | reserverad (0)                     |
| 22    | u16[8] | soil                | 8 kanaler, 0=saknas                |
| 38    | u8     | crc8                | Dallas/Maxim CRC8 över body[0..37] |
| 39    | u8     | _pad                | padding (0)                        |

## API
```cpp
namespace msg {
  // Konstanter: MAGIC0, MAGIC1, VERSION_SENSOR_V1, TYPE_SENSOR,
  //             FRAME_HDR_SIZE, SENSOR_V1_BODY_SZ, SENSOR_V1_TOTAL

  struct FrameHeader {
    uint8_t  magic0, magic1, version, type;
    uint16_t seq, flags;
  };

  struct SensorV1 { /* se headerfil */ };

  void   makeHeader(FrameHeader& h, uint8_t version, uint8_t type, uint16_t seq, uint16_t flags = 0x0001);
  size_t encodeSensorV1(const FrameHeader& h, const SensorV1& s, uint8_t out[SENSOR_V1_TOTAL]);
  bool   decodeSensorV1(const uint8_t in[SENSOR_V1_TOTAL], FrameHeader& h_out, SensorV1& s_out);
  uint8_t crc8_dallas(const uint8_t* data, size_t len);
}


Felhantering

decodeSensorV1 returnerar false om magic/version/type inte matchar eller om CRC8 inte stämmer (när flags bit0 är satt).

Versionspolicy

V1 är låst (48 B). Nya fält → V2 med ny body-layout och uppdaterad version.
