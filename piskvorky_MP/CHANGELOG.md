# BP_PROJ: Piskvorky MP

### 05.05.2019
- Server:
  - Přidáno nastavení síťového režimu (_ETHMODE\_DHCP_ a _ETHMODE\_STATIC_, stejný princip jako u clienta)
  - Upraveny podmínky u direktiv _#elif_ (přidáno _defined()_)
- Client:
    - Upraveny podmínky u direktiv _#elif_ (přidáno _defined()_)


### 21.04.2019
- Program pro clienta:
  - Upraveny přidání schémata v nastavení pro jednotlivé clienty pomocí direktiv (#define CLIENT_x_)
  - Přidán režim doutykové plochy displeje _TOUCH\_LANDSCAPE_ případně _TOUCH\_PORTRAIT_
  - Přidán síťové režimy:
    - _ETHMODE\_DHCP_: client získává IP adresu z DHCP serveru
    - _ETHMODE\_STATIC_: client používá statickou IP adresu nastavenou v kódu
  - Přidány komentáře ke konfiguracím

### 14.04.2019
- Program pro server:
  - Přidány možnost pro debuging
- Úprava manuálu pro piškvorky (_manual.md_): do tabulky přidán příkaz pro přepnutí na dalšího hráče a přidán popis funkce tlačítek

### 13.04.2019
- Program pro server:
  - Přidání příkazu pro odpojení hráčem
  - Přidána varianta, že pokud má server v kódu nastavenou adresu 0.0.0.0, nechá si IP adresu přiřadit DHCP serverem
  - Přídán příkaz pro přeskočení hráčem
  - Pokud běží hra a je stisknuto zelené tlačítko, hra se přesune na dalšího hráče
- Přidání hlavičky do zdrojových kódů, celkové úprava zdrojáků a doplnění komentářů
- Úprava readme.md: přidání odkazů (šipky) de jednotlivých složek
- Úprava manuálu pro piškvorky (_manual.md_): do tabulky přidán příkaz pro odpojení hráče
- Přesun projektu do složky _piskvorky_MP_ a přejmenování příslošných složek pro zdrojáky, úprava názvů v _readme.md_

### 09.03.2019
- Program pro server:
  - Doladěno funguvání indikační LED
  - Provedeno několik testů

- Program pro server:
  - Upravena funkce pro odesílání dat serveru
  - Přidána funkce pro kontrolu spojení se serverem (v případě, že došlo ke ztrátě spojení, je spojení zrušeno i ze strany clienta a vypíše se chybová hláška)
  - Přidáno vyhodnoce kódu devět (vyžádané odpojení ze strany serveru)
  - Kalibrace displejů
  - Drobné kosmetické úpravy kódu a doplnění komentářů
- Zhotoveny kalibrace Clientů 1 a 2 dle zjištění popsané v [notes](https://github.com/janzavorka/BP_PROJ/blob/master/Notes.md).


### 09.03.2019
- Dokončení programu pro clienta (kvůli místu v paměti odstraněna sériová linka)
- Malé úpravy v kódu pro server, odstraněna chyba v přijímací funkci

### 06.03.2019
- Program pro clienta: vylepšení vykreslování dat na displeji, začátek úprav vyhodnocování stisku tlačítek

### 03.03.2019
- Úprava programu pro clienta pro komunikaci se serverem

### 26.02.2019
- Automatické detekce ethernet kontroléru při zapnutí
- Pro kontroléry W5200 a W5500 kontrola připojení kabelu, chyba indikována blikající červenou LED
- Vylepšení a dodělání fukcí pro řízení hry
- Přidány funkce pro indikaci stavu hry pomocí RGB LED

### 25.02.2019
- Úprava funkcí pro řízení hry

### 23.02.2019
- Celkové přepracování programu pro server
- Pro server přidána LED dioda indikující stav
- Přidání zabezpečovacích funkcí pro komunikaci client->server a server -> client

### 12.12.2018
- Program pro clienta
 - pro všechny client desky zkalibrovány displeje, u clienta 3 nutná úprava v loopu (dislej má jinou orientaci)
 - oprava zobrazování hlášek (výhra, prohra, remíza)

### 09.12.2018
- Vytvoření desky s dvěma tlačítkama pro ovládání serveru
- Program pro server
 - přidání fce dvou tlačítek (start, stop), vyhodnocení stisku v loopu (nelze využít interrupty (rozhodilo by odesílání dat))

### 08.12.2018
- Použití Arduino Due jako server
- Program pro server
 - herní deska se odesílá po 8 bajtech
 - oprava funkce pro získání čísla dalšího hráče (docházelo k zacyklení)
 - přidána funkce pro kontrolu výhry
- Program pro clienta
 - Oprava zobrazování barevných koleček (nyní se zobrazují správně)
 - oprava zamrznutí hráče, kdy během jeho tahu dojde k resetu hry ze strany serveru

### 03.12.2018
- Program pro server
 - úprava funkce pro získání dalšího clienta

### 27.11.2018
- Program pro clienta
 - oprava připojovací funkce
 - doladění úvodních obrazovek


### 25.11.2018
- Práce na programu pro clienta:
 - Dodělány jednotlivé fáze hry
 - Funkce pro spojení se serverem
 - Mnoho dalších drobných úprav
- Program pro server
 - správné vyhodnocení odpojených clientů
 - při tahu daného clienta se pošle jen index dané pozice, ne celý herní board


### 12.11.2018
- Program pro clienta:
 - přidání třídy pro tlačítka

### 11.11.2018
- Příprava programu pro clienta
 - Úprava parametrů pro síťové připojení
 - Příprava rozložení packetu pro komunikaci
- Základ programu pro server
