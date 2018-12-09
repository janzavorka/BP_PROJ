# BP_PROJ: Piskvorky v1

### 09.12.2018
- Vytvoøení desky s dvìma tlaèítkama pro ovládání serveru
- Program pro server
	- pøidání fce dvou tlaèítek (start, stop), vyhodnocení stisku v loopu (nelze vyuít interrupty (rozhodilo by odesílání dat))

### 08.12.2018
- Pouití Arduino Due jako server 
- Program pro server
	- herní deska se odesílá po 8 bajtech
	- oprava funkce pro získání èísla dalšího hráèe (docházelo k zacyklení)
	- pøidána funkce pro kontrolu vıhry
- Program pro clienta
	- Oprava zobrazování barevnıch koleèek (nyní se zobrazují správnì)
	- oprava zamrznutí hráèe, kdy bìhem jeho tahu dojde k resetu hry ze strany serveru

### 03.12.2018
- Program pro server
	- úprava funkce pro získání dalšího clienta

### 27.11.2018
- Program pro clienta
	- oprava pøipojovací funkce
	- doladìní úvodních obrazovek


### 25.11.2018
- Práce na programu pro clienta:
	- Dodìlány jednotlivé fáze hry
	- Funkce pro spojení se serverem
	- Mnoho dalších drobnıch úprav 
- Program pro server
	- správné vyhodnocení odpojenıch clientù
	- pøi tahu daného clienta se pošle jen index dané pozice, ne celı herní board


### 12.11.2018
- Program pro clienta:
  - pøidání tøídy pro tlaèítka 

### 11.11.2018
- Pøíprava programu pro clienta
  - Úprava parametrù pro síové pøipojení
  - Pøíprava rozloení packetu pro komunikaci
- Základ programu pro server
	

