# super_libs

Samling av mina ESP-bibliotek som används via `lib_extra_dirs`.

## Snabbkoppling på maskinen
```bash
git clone git@github.com:kjellnordinSE/super_libs.git ~/Documents/Git/super_libs
echo 'export PLATFORMIO_LIBS="$HOME/Documents/Git/super_libs"' >> ~/.bashrc
source ~/.bashrc
I dina platformio.ini:

ini
Kopiera kod
lib_extra_dirs = ${sysenv.PLATFORMIO_LIBS}
lib_ldf_mode   = chain+
Bibliotek
lib_identity — chip-ID, MAC, DevEUI (FF:FE), boot-banner.

lib_message — fixed-size paket (SensorV1 = 48 B), encoder/decoder, CRC8.

yaml
Kopiera kod

