# lib_lora

Transportlager för rå LoRa med RadioLib (SX127x/SX126x). Fristående från payloadformat, men innehåller en enkel “frame”-header som kan användas med valfri nyttolast (t.ex. `lib_message` 48B).

## API (kort)
```cpp
namespace lora {
  struct Pins { int nss, dio0, rst; int busy; bool hasBusy; };
  struct Config {
    float freq = 867.5;   // MHz (EU868)
    float bw   = 125.0;   // kHz
    uint8_t sf = 7;       // 7..12
    uint8_t cr = 5;       // 5..8 -> 4/5 .. 4/8
    uint8_t sync = 0x12;  // standard
    uint8_t power = 14;   // dBm
    uint16_t preamble = 8;
    bool crc = true;
    Pins pins{18, 26, 14, -1, false}; // default TTGO-Lora32-ish
  };

  bool begin(const Config&);
  bool send(const uint8_t* data, size_t len);
  int  recv(uint8_t* buf, size_t cap, uint32_t timeout_ms,
            int16_t* outRssi=nullptr, float* outSnr=nullptr);

  void sleep();
  void standby();

  struct FrameHeader {
    uint8_t  magic0{'M'};
    uint8_t  magic1{'1'};
    uint8_t  ver{1};
    uint8_t  type{1};
    uint16_t seq{0};
    uint16_t src{0};
    uint8_t  flags{0};
  } __attribute__((packed));

  size_t packFrame(const FrameHeader& h, const uint8_t* payload, size_t plen,
                   uint8_t* out, size_t cap);
  bool   unpackFrame(const uint8_t* in, size_t len,
                     FrameHeader& h, const uint8_t*& payload, size_t& plen);

  uint16_t makeSrcFromMac(); // tar 16 bit från MAC om identity.h finns (annars 0)
}

