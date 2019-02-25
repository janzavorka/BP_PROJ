# BP_PROJ: Piskvorky v1

### 25.20.2019
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
