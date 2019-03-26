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

### Arduino PoE
- Pro Arduino Ethernet lze dokoupit a připájet PoE shield, tato sestava byla i ofociálně prodávána na [Arduino storu.](https://store.arduino.cc/arduino-ethernet-rev3-with-poe)
- Příslušné otvory jsou na Arduinu Ethernet k dispozici a jsou zapojeny
- Problém pro tento projekt je, že napájení z PoE modulu je přivedeno na vstup regulátoru, co má Arduino na desce. Při použití PoE modulu s 5 V na výstupu bude napětí na výstupu regulátoru o cca 0,8 V nižší (tj. ne potřebnýcch 5 V). Při použití modulu s 12 V výstupem se bude regulátor přehřívat kvůli velkému odběru displeje.
- Z [datasheetu](https://www.semiconductorstore.com/pages/asp/DownloadDirect.asp?sid=1553431563413) PoE modulu plyne, že pokud je pin ADJ připojen na pin +VDC, bude napětí na výstupu PoE modulu 5,50 V (což je na desce Arduina realizováno). Po započítání úbytku napětí na regulátoru by bylo napětí cca 4,8 V (musel by se překalibrovat displej na tuto hodnotu).
