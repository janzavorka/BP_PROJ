# Návod na použití piškvorek

### Význam indikační LED u serveru
Význam: <img src="manual_data/LED_state/blue.png" width="10" height="10" /> svítí, <img src="manual_data/LED_state/blue_blink.png"  height="20" /> bliká


| Stav                                                                        | Význam        | 
| :------------------------------------------------------------------------:  |:-------------:| 
| <img src="manual_data/LED_state/black.png" width="10" height="10" />        | server je vypnutý/nemá napájení | 
| <img src="manual_data/LED_state/blue.png" width="10" height="10" />         | server je připraven             |  
| <img src="manual_data/LED_state/violet_blink.png"  height="20" />   3x      | nový client připojen            |
| <img src="manual_data/LED_state/orange_blink.png"  height="20" />   3x      | client se odpojil/byl odpojen   |
| <img src="manual_data/LED_state/green.png" width="10" height="10" />        | aktuálně běží hra               |
| <img src="manual_data/LED_state/green_blink.png" height="20" />             | hra ukončena (výhra/remíza)     |
| <img src="manual_data/LED_state/red_blink.png" height="20" />       3x      | chyba (nedostatek hráčů pro hru)|
| <img src="manual_data/LED_state/red_blink.png" height="20" />     stále     | chyba sítě (připojení kabelu)   |

---

### Příkazy pro server
|  Příkaz |                         Význam                         |
|:-------:|:------------------------------------------------------:|
|   help  | Vypíše nápovědu (dostupné příkazy)                     |
|   info  | Vypiší informace o serveru (HW, verze SW, apod.)       |
| clients | Zobrazí čísla a IP adresy připojených clientů          |
|  start  | Spustí hru (ekvivalent zeleného tlačítka)              |
|  reset  | Přeruší a resetuje hru (ekvivalent červeného tlačítka) |
