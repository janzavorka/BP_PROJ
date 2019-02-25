# BP_PROJ: Piskvorky v1

### 25.02.2019
- Úprava funkcí pro řízení hry

### 23.02.2019
- Celkov� p�epracov�n� programu pro server
- Pro server p�id�na LED dioda indikuj�c� stav
- P�id�n� zabezpe�ovac�ch funkc� pro komunikaci client->server a server -> client

### 12.12.2018
- Program pro clienta
	- pro v�echny client desky zkalibrov�ny displeje, u clienta 3 nutn� �prava v loopu (dislej m� jinou orientaci)
	- oprava zobrazov�n� hl��ek (v�hra, prohra, rem�za)

### 09.12.2018
- Vytvo�en� desky s dv�ma tla��tkama pro ovl�d�n� serveru
- Program pro server
	- p�id�n� fce dvou tla��tek (start, stop), vyhodnocen� stisku v loopu (nelze vyu��t interrupty (rozhodilo by odes�l�n� dat))

### 08.12.2018
- Pou�it� Arduino Due jako server
- Program pro server
	- hern� deska se odes�l� po 8 bajtech
	- oprava funkce pro z�sk�n� ��sla dal��ho hr��e (doch�zelo k zacyklen�)
	- p�id�na funkce pro kontrolu v�hry
- Program pro clienta
	- Oprava zobrazov�n� barevn�ch kole�ek (nyn� se zobrazuj� spr�vn�)
	- oprava zamrznut� hr��e, kdy b�hem jeho tahu dojde k resetu hry ze strany serveru

### 03.12.2018
- Program pro server
	- �prava funkce pro z�sk�n� dal��ho clienta

### 27.11.2018
- Program pro clienta
	- oprava p�ipojovac� funkce
	- dolad�n� �vodn�ch obrazovek


### 25.11.2018
- Pr�ce na programu pro clienta:
	- Dod�l�ny jednotliv� f�ze hry
	- Funkce pro spojen� se serverem
	- Mnoho dal��ch drobn�ch �prav
- Program pro server
	- spr�vn� vyhodnocen� odpojen�ch client�
	- p�i tahu dan�ho clienta se po�le jen index dan� pozice, ne cel� hern� board


### 12.11.2018
- Program pro clienta:
  - p�id�n� t��dy pro tla��tka

### 11.11.2018
- P��prava programu pro clienta
  - �prava parametr� pro s��ov� p�ipojen�
  - P��prava rozlo�en� packetu pro komunikaci
- Z�klad programu pro server
