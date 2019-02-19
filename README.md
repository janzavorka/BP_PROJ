# BP_PROJ

### Dotykový display - knihovny
- Knihovny použité pro ovládání (nemusí být aktuální verze)

### Piskvorky_red
- HW: 2x Arduino Ethernet, 2x 2,4" dotykový display
- jedno arduino jako server, druhé jako client (pevná IP adresa)

### Menu 
- Návrh původního menu pro piškvorky 

### piskvorky_MP_v1 
- Piškvorky pro až 5 hráčů
- 1x arduino Due jako server, max 5x arduino Ethernet s dotykovým displejem jako client
- připojovánou přes router s DHCP (serveru nutno přidělit pevnou IP)

### Zprava_BPROJ
-  Výstupní zpráva z celého projektu

### RGB_LED_cal
- Slouží ke kalibraci barev pro RGB LED u serveru (Arduino Due)
- RGB LED se připojí na příslušné PWM piny, změna barev a celkové intenzity probíhá pomocí sériového monitoru
- Příkaz _"help"_ vypíše nápovědu přímo na sériový monitor
- Příkaz: _"led rrr,ggg,bbb,inten"_, kde:
    - _rrr,ggg,bbb_ jsou voleny z <0;&nbsp;255> a reprezentují jednotlivé barvy
    - _inten_ je volena z <0;&nbsp;100> a reprezentuje celkovou intenzitu LED
