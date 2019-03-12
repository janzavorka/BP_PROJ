# Notes for this project

### Plugin pro Atom
- Funkční plugin pro Atom: [arduino-upload](https://atom.io/packages/arduino-upload)
- Pro funkčnost nutné Arduino IDE, v nastavení pluginu specifikovat cestu k _arduino_debug.exe_
- Vytváří .bin, .hex soubory, ignorovány pomocí [.gitignore](/.gitignore)
- Pro správný výpis informací/chyb během kompilace/uploadu by měl být jazyk v Arduino IDE nastaven na _English_


### Kalibrace displejů
- Při kalibraci displeje se standardně připojí Arduino k počítači pomocí převodníku USB->USART aby bylo možné číst údaje při kalibraci ze sériové linky
- Připojený displej ovšem odebírá kolem 300 mA což s při připojení delším USB kabelem vytvoří úbytek napětí a kalibrace není tak přesná (výkyv klidně o 300 mV)
- Eliminováno připojením externího zdroje 5 V
