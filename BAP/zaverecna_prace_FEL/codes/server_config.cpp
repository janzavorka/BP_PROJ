/* !!! ---------- KONFIGURACE - nastaveni ethernetu ---------- !!! */
#define ETHMODE_STATIC //Varianty: ETHMODE_DHCP; ETHMODE_STATIC
byte mac[] = {
  0x54, 0xAB, 0x3A, 0x5B, 0x6F, 0x47
};
IPAddress serverAddress(10,0,0,8);
unsigned int localPort = 55555;
/* !!! ---------- KONEC nastaveni ethernetu ---------- !!! */

/* !!! ---------- KONFIGURACE - nastaveni pinu a LED ---------- !!! */
//Piny tlacitka start a reset
#define startPIN 2
#define resetPIN 3
//
//Piny na kterych jsou pripojenr jednotlive barvy LED, musi podporovat PWM
#define LED_red 7
#define LED_green 5
#define LED_blue 6
//
//Nastaveni maximalniho jasu vsech LED, interval <0; 100>
const byte LED_br = 40;
/* !!! ---------- KONEC nastaveni pinu a LED ---------- !!! */

/* !!! ---------- KONFIGURACE - nastaveni barev hracu ---------- !!! */
#define PLAYER1COLOR RED            //0xF800
#define PLAYER2COLOR GREEN          //0x07E0
#define PLAYER3COLOR PURPLE         //0x780F
#define PLAYER4COLOR GREENYELLOW    //0xAFE5
#define PLAYER5COLOR OLIVE          //0x7BE0
/* !!! ---------- KONEC nastaveni barev hracu ---------- !!! */
