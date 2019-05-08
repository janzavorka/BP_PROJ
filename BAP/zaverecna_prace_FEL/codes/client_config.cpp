/* ---------- KONFIGURACE - nastaveni schemat----------*/
#define CLIENT1 //Vyber profilu pro klienta
/* ---------- KONEC - nastaveni schemat----------*/

/* ---------- KONFIGURACE - nastaveni mac adres----------*/
#ifdef CLIENT1
//Klient 1
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x11, 0x08, 0x18
};
#elif defined(CLIENT2)
//Klient 2
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x11, 0x09, 0x78
};
#elif defined(CLIENT3)
//Klient 3
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x11, 0x08, 0xA0
};
#elif defined(CLIENT4)
//Klient 4
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEE, 0xFE, 0xBD
};
#elif defined(CLIENT5)
//Klient 5
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEE, 0xFE, 0xAD
};
#else
//Defaultni hodnoty
#warning "Je pouzita defaultni MAC adresa"
byte mac [] = {
  0xDE, 0xAD, 0xBE, 0xEE, 0xFE, 0xAB
}
#endif
/* ---------- KONEC - nastaveni mac adres----------*/

/* ---------- KONFIGURACE - nastaveni site ----------*/
#define ETHMODE_STATIC //Varianty: ETHMODE_DHCP; ETHMODE_STATIC

#ifdef ETHMODE_STATIC
  #ifdef CLIENT1
  IPAddress clientAddress(10,0,0,138);

  #elif defined(CLIENT2)
  IPAddress clientAddress(10,0,0,139);

  #elif defined(CLIENT3)
  IPAddress clientAddress(10,0,0,140);

  #elif defined(CLIENT4)
  IPAddress clientAddress(10,0,0,141);

  #elif defined(CLIENT5)
  IPAddress clientAddress(10,0,0,142);
  #else
  //Defaultni nastaveni
  IPAddress clientAddress(10,0,0,100);
  #warning "Pozor - je pouzito defaultni nastaveni IP, pro kazdeho clienta nutno zmenit"
  #endif
#endif
IPAddress serverAddress(10,0,0,8);  //Nastaveni IP adresy serveru
unsigned int localPort = 3333;      //Port
/* ---------- KONEC - nastaveni site ----------*/

/* ---------- KONFIGURACE - kalibrace displeje ----------*/
#ifdef CLIENT1
//Klient 1
#define TOUCH_XMIN 221
#define TOUCH_XMAX 950
#define TOUCH_YMIN 200
#define TOUCH_YMAX 950
#define TOUCH_LANDSCAPE
#elif defined(CLIENT2)
//Klient 2
#define TOUCH_XMIN  233
#define TOUCH_XMAX  937
#define TOUCH_YMIN  210
#define TOUCH_YMAX  910
#define TOUCH_LANDSCAPE
#elif defined(CLIENT3)
//Klient 3
#define TOUCH_XMIN 230
#define TOUCH_XMAX 960
#define TOUCH_YMIN 220
#define TOUCH_YMAX 920
#define TOUCH_LANDSCAPE
#elif defined(CLIENT4)
//Klient 4
#define TOUCH_XMIN 0
#define TOUCH_XMAX 100
#define TOUCH_YMIN 0
#define TOUCH_YMAX 100
#elif defined(CLIENT5)
//Klient 5
#define TOUCH_XMIN 0
#define TOUCH_XMAX 100
#define TOUCH_YMIN 0
#define TOUCH_YMAX 100
#else
//Defaultni nastaveni, je nutne upravit
#define TOUCH_XMIN 0
#define TOUCH_XMAX 100
#define TOUCH_YMIN 0
#define TOUCH_YMAX 100
#endif
/* ---------- KONEC - kalibrace displeje ----------*/
