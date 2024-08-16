## 
# https://github.com/nodemcu/nodemcu-devkit-v1.0
# télécahrger "https://github.com/martin-ger/esp_wifi_repeater/tree/master/firmware"

## Setup Instructions
```bash
pip install esptool
e:
cd E:\Programmation\GitHub\BIOREACTOR\development\wifi_communication\BRIDGE_WIFI_ESP8266 (not working)\esp_wifi_repeater(firmware)
# changer port COM et le nom d'utilisateur :
C:\Users\Simon\anaconda3\envs\bioreactor\Scripts\esptool.exe --port COM12 write_flash -fm dio -fs 8MB 0x00000 0x00000.bin 0x02000 0x02000.bin 0x82000 0x82000.bin

ou 
C:\Users\Simon\anaconda3\envs\bioreactor\Scripts\esptool.exe --port COM12 --baud 115200 write_flash -fm dio -fs 8MB 0x00000 0x00000.bin 0x02000 0x02000.bin 0x82000 0x82000.bin

```

## Configuration Instructions
-Utiliser un logiciel de terminal série (Arduino IDE, PuTTY) avec 74880 baud et le port serie UART (COM)


set ssid1 oOo           // Réseau principal
set password1 wYnXLVYW42yknz7VG4Ab25E8
set ssid2 gGg        // Réseau répéteur (vous pouvez garder le même SSID pour éviter la confusion)
set password2 fehoiHFOEPHI5469f655DFOP
set bridge 1                   // Activer le mode bridge
save
reboot







C:\Tests>C:\Users\Simon\anaconda3\envs\bioreactor\Scripts\esptool.exe --port COM12 --baud 115200 write_flash -fm dio -fs 1MB 0x00000 C:\Tests\AIThinker_ESP8266_8Mbit_v1.5.4.1.bin