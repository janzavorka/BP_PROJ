# Návod na použití piškvorek

### Význam indikační LED u serveru
Význam: <img src="manual_data/LED_state/blue.png" height="20" /> svítí, <img src="manual_data/LED_state/blue_blink.png"  height="20" /> bliká


| Stav                                                                        | Význam        |
| :------------------------------------------------------------------------:  |:-------------:|
| <img src="manual_data/LED_state/black.png" height="20" />                   | server je vypnutý/nemá napájení |
| <img src="manual_data/LED_state/blue.png" height="20px" />                  | server je připraven             |  
| <img src="manual_data/LED_state/violet_blink.png"  height="20" />   3x      | nový client připojen            |
| <img src="manual_data/LED_state/orange_blink.png"  height="20" />   3x      | client se odpojil/byl odpojen   |
| <img src="manual_data/LED_state/green.png" height="20" />                   | aktuálně běží hra               |
| <img src="manual_data/LED_state/green_blink.png" height="20" />             | hra ukončena (výhra/remíza)     |
| <img src="manual_data/LED_state/red_blink.png" height="20" />       3x      | chyba (nedostatek hráčů pro hru)|
| <img src="manual_data/LED_state/red_blink.png" height="20" />     stále     | chyba sítě (připojení kabelu)   |

---

### Funkce tlačítek na serveru
#### ![Zelené tlacitko](https://placehold.it/15/c5f015/000000?text=+) `Zelené tlačítko`:
  - Pokud neběží hra (indikační LED svítí modře <img src="manual_data/LED_state/blue.png" height="20" />) - stiskem tlačítka dojde ke spuštění hry (s ověřením zda je k dispozici dostatek hráčů)
  - Pokud hra již běží (indikační LED svítí zeleně <img src="manual_data/LED_state/green.png" height="20" />) - stiskem tlačítka dojde k posunutí tahu na dalšího hráče

#### ![Cervene tlacitko](https://placehold.it/15/f03c15/000000?text=+) `Červené tlačítko`:
  - Pokud je stisknuto, dojde k přerušení hry aktuálně běžící hry (dosavadní stav hry je resetován)
---

### Příkazy pro server
|  Příkaz |                         Význam                         |
|:-------:|:------------------------------------------------------:|
|   help  | Vypíše nápovědu (dostupné příkazy)                     |
|   info  | Vypiší informace o serveru (HW, verze SW, apod.)       |
| players | Zobrazí čísla a IP adresy připojených hracu            |
| kick 'x'| Odpojí hráče číslo _x_                                 |
| nextP   | Přepne na dalšího hráče                                |
|  start  | Spustí hru (ekvivalent zeleného tlačítka)              |
|  reset  | Přeruší a resetuje hru (ekvivalent červeného tlačítka) |
