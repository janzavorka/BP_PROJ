/*>>>>>>> Piškvorky s arduino Ethernet <<<<<<<
 * - max 5 hráčů
 * - 1x arduino jako server, max 5x arduino jako client
 *
 */

#include <Ethernet.h>
#include <SimpleTimer.h>
/* ----------Datum změny----------*/
char makeDate[] = "12.03.2019";

/* ----------Nastavení ethernetu----------*/ //(ZMĚNIT)
//Client 1
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEE, 0xFE, 0x00
};

unsigned int localPort = 3333;      // local port to listen on
EthernetServer server = EthernetServer(localPort);
const byte maxPlayers = 5;
EthernetClient clients [maxPlayers];
bool clientConnected = false;
bool serverReady = false;

//Casovac
SimpleTimer timer;
namespace std {void __throw_bad_function_call() { while(1); }; } //Pro správné fungování simpleTimer knihovny

//piny pro tlačítka
bool pinReady = false; //Slouží k deaktivaci tlačítek aby se zabránilo vícedotykům současně
const byte startPIN = 40;
const byte resetPIN = 38;

/* ----------Časové intervaly různých událostí----------*/
#define buttonOff 1000 //Po jakou dobu bude dotyková plocha deaktivována (zabrání multidotykům)
unsigned long long refresh_buttonOff = 0;

const long clientMessageLast = 8000; //Doba zobrazení zprávy na straně clienta (v milisekundách)

//Herní server
IPAddress serverAddress(10,0,0,8);

/* ----------Ovládání pomocí sériové linky----------*/
const byte max_buffik = 30;
String buffik = "";


//Rozměry herní sítě
#define meshX 11
#define meshY 8
/* ----------HRA----------*/
byte serverPhase = 0; //označuje fázi hry= 0: čekání na připojení klientů, 1: hra
byte lastPlayer = 0; //Označuje posledního hráče, který hrál
byte crossNum = 5; //Počet koleček, které je nutné spojit pro výhru
byte clientsData [maxPlayers][2][3]; //první index: číslo hráče; druhý index: číslo zprávy (aby bylo provedeno, obě se musí rovnat); třetí index: datový packet1, datový packet2, přijato/nepřijato (0/1) - vyplní server při příjmu dat

/* ----------Barvy----------*/
#define BLACK           0x0000      /*   0,   0,   0 */
#define NAVY            0x000F      /*   0,   0, 128 */
#define DARKGREEN       0x03E0      /*   0, 128,   0 */
#define DARKCYAN        0x03EF      /*   0, 128, 128 */
#define MAROON          0x7800      /* 128,   0,   0 */
#define PURPLE          0x780F      /* 128,   0, 128 */
#define OLIVE           0x7BE0      /* 128, 128,   0 */
#define LIGHTGREY       0xC618      /* 192, 192, 192 */
#define DARKGREY        0x7BEF      /* 128, 128, 128 */
#define BLUE            0x001F      /*   0,   0, 255 */
#define GREEN           0x07E0      /*   0, 255,   0 */
#define CYAN            0x07FF      /*   0, 255,   0 */
#define RED             0xF800      /* 255,   0,   0 */
#define MAGENTA         0xF81F      /* 255,   0, 255 */
#define YELLOW          0xFFE0      /* 255, 255,   0 */
#define WHITE           0xFFFF      /* 255, 255, 255 */
#define ORANGE          0xFD20      /* 255, 165,   0 */
#define GREENYELLOW     0xAFE5      /* 173, 255,  47 */

/* ----------Piškvorky----------*/
byte packetLength = 136;
byte board [136]; //0: nikdo, 1: hráč 1; 2: hráč 2
/* >>>>> Rozložení herního packetu <<<<<
 *  0-89:   Obsazení herních polí (standadně 0, server doplňuje čísla)
 *  90:     Hlášení prostřednictvím kódu
 *            0:    nic nedělej
 *            1:    vše OK, hraje se, překresli obrazovku
 *            3:    připravit novou hru, čekání na hráče (úvodní obrazovka)
 *            9:    odpojuji (výzva pro hráče aby se také odpojil)
 *            100:  hra skončila remízou
 *            101:  vyhrál hráč 1
 *            102:  vyhrál hráč 2
 *            103:  vyhrál hráč 3
 *            104:  vyhrál hráč 4
 *            105:  vyhrál hráč 5
 *            201:  problémy s hráčem 1 (odpojil se)
 *            202:  problémy s hráčem 2 (odpojil se)
*             203:  problémy s hráčem 3 (odpojil se)
              204:  problémy s hráčem 4 (odpojil se)
 *            205:  problémy s hráčem 5 (odpojil se)
 *  91:     Číslo hrajícího hráče
 *  93:     Počet odehraných kol, zvyšuje se na straně serveru
 *  95-96:  Barva hráče 1
 *  97-98:  Barva hráče 2
 *  99-100: Barva hráče 3
 *  100-101:Barva hráče 4
 *  102-103:Barva hráče 5
 *  104-107:IP hráče 1
 *  108-111:IP hráče 2
 *  112-115:IP hráče 3
 *  116-119:IP hráče 4
 *  120-123:IP hráče 5
 */
//Označení pozic
#define gb_code       90
#define gb_actPlayer  91
#define gb_filled     92
#define gb_round      93
#define gb_PC1        95
#define gb_PC2        97
#define gb_PC3        99
#define gb_PC4        101
#define gb_PC5        103

const byte colorAddr [] = {gb_PC1, gb_PC2, gb_PC3, gb_PC4, gb_PC5}; //Pole adres barev hráčů v poli board
const byte IPaddr [] = {105, 109, 113, 117, 121}; //Počáteční adresy (indexy) jednotlivých IP adres hráčů v boardu


/* ----------Ovládání LED----------*/
//Piny
#define LED_red 6
#define LED_green 7
#define LED_blue 8

//Nastavení maximálního jasu všech LED
#define LED_br 100

//Barvy pro LED
byte LEDcol_red[] = {255, 0, 0};
byte LEDcol_green[] = {0, 255, 0};
byte LEDcol_blue[] = {0, 0, 255};
byte LEDcol_orange[] = {255, 50, 0};
byte LEDcol_violet[] = {255,0,255};
//byte LEDcol_red[] = {255, 0, 0};

//RGB informační LED připojená k serveru
class RGB_LED{
  private:
    byte p_RED = 0;
    byte p_GREEN = 0;
    byte p_BLUE = 0;
    byte static_col[3] = {255,255,255}; //Pro statické svícení
    byte blink_col[3] = {255,255,255}; //Pro blikání
    bool blinkState = false;
    byte intensity = 0;
    byte blinkCount = 0;
    int blinkTimerID = 0;

  public:
    RGB_LED();
    RGB_LED(byte Red, byte Green, byte Blue, byte intens);
    void changeStaticColor(byte color[]);
    void changeIntensity(byte inten);
    void changeBlinkColor(byte color[], byte times);
    void blink(void);
    void LEDoff(void);
    void LEDon(void);
    void checkLEDstat(void);
};

RGB_LED signalLED;

/* ----------PROTOTYPY----------*/
void cleanBoard(void); //Vyplní herní desku nulami
void syncBoardIPs(void); //Synchronizuje IP adresy v boardu a s IP adresy v seznamu clientů (clients[])
void sendBoard(byte); //Odešle herní desku všem hráčům (argument číslo řídicího kódu)
void sendBoard(byte, byte); //Odešle herní desku konkrétnímu hráči (argument číslo řídicího kódu a číslo clienta/hráče)
void setBoard(void); //Připraví herní desku
byte getNextPlayer(byte); //Vrátí číslo dalšího hráče (argument fce je číslo předchozího hráče)
void checkGame(byte, byte); //Zkontroluje průběh hry...případně vyplní kód v boardu, argument je poslední pole a číslo hrajícího hráče
void syncBoardIPs(void); //Synchronizuje herní desku board s připojenými clienty
/* gameControl */
void stopGame(void); //Zruší běžící hru
void startGame(void); //Spustí hru
void shiftPlayer(void); //Změní číslo hráče a odešle herní desku
/* communication */
void recieveData(byte); //Přijme data a případně zpracuje, argument je index hráče
void checkIncommingData(void); //Kontroluje, zda nějaký client neposlal data
void resetClientData(byte); //Smaže přijatá data daného clienta
void processClientData(byte); //Provede požadované úkony pro daného hráče
byte getHWcontroller(void); //Vrátí číslo použitého ethernet kontroléru
/* SerialControl */
void processBuffik(void); //Zpracuje přijatou zprávu přes sériovou linku
void printLine(byte, byte); //Vypíše několik zadaných znaků za sebou (pro výpis oddělovacích čar na sériovém monitoru)
/* Ovládání LED */
//void setLED (byte, byte); //Nastaví Barvu LED podle požadavků (barva, jas)
//void setLEDgamePhase(void); //Nastaví barvu LEDky podle herní fáze
/*
 * >>>>>>>>>> SETUP <<<<<<<<<<
 */
void setup() {
  //Sériová linka
  Serial.begin(9600);
  //Alokace paměti pro string
  buffik.reserve(max_buffik);
  //Tlačítka
  pinMode(startPIN, INPUT);
  pinMode(resetPIN, INPUT);
  randomSeed(analogRead(0));
  delay(10);
  //Nastavení LED

  signalLED = RGB_LED(LED_red, LED_green, LED_blue, 100);
  //Turn off LED


  Serial.println("Startuji piskvorkovy server");
  //Ethernet
  Ethernet.begin(mac, serverAddress);
  //Kontrola připojení kabelu
  if(getHWcontroller() > 1){ //Pokud je kontrolér W5200 nebo W5500 (pro ostatní není funkce podporována)
      if(Ethernet.linkStatus() == LinkOFF){
        Serial.println("Zkontrolujte pripojeni kabelu");
      }
      while(Ethernet.linkStatus() == LinkOFF){
          signalLED.changeStaticColor (LEDcol_red);
          signalLED.LEDon();
          delay(1000);
          signalLED.LEDoff();
          delay(1000);
      }
  }

  Serial.print("Moje IP adresa: ");
  Serial.println(Ethernet.localIP());
  delay(200);
  Serial.println("Spoustim server");
  server.begin();
  delay(200);

  //Reset datových polí
  setBoard();
  for(byte i = 0; i < maxPlayers; i++){
    resetClientData(i);
  }

  //setLED(LEDcol_blue, 100);
  signalLED.changeStaticColor (LEDcol_blue);
}

/*
 * >>>>>>>>>> LOOP <<<<<<<<<<
 */
void loop() {
  if ((millis() - refresh_buttonOff > buttonOff) && !pinReady){ //Obnova funkce tlačítek (zabrání přečtení více stisků současně)
    pinReady = true;
    refresh_buttonOff = millis();
  }
  timer.run(); //Aby timery správně běžely (knihovna simpleTimer)
  checkIncommingData();
  //Část se zpracováním hry
    if(serverPhase == 0){
      EthernetClient newClient = server.accept();
      delay(10);
      if ((newClient)) {
        Serial.println("Pripojen novy uzivatel");
        bool clientOK = true; //Test clienta
        byte clientIndex = 100; //Index, ktery bude clientovi přiřazen

        //Hledání volné pozice pro nového clienta
        for(int i = 0; i < maxPlayers; i++){
          if(!clients[i]){
            clientIndex = i;
            break;
          }
        }
        if(clientIndex==100){ //Pokud index zůstal 100, není pro clienta místo
          Serial.println("Neni volna pozice pro noveho hrace");
          clientOK = false;
        }

        //Test na duplicitu
        for(byte i = 0; i < maxPlayers; i++){ //Ověření duplicit
            if(clients[i].remoteIP() == newClient.remoteIP()){
               clientOK = false;
               Serial.println("Stejny hrac je jiz pripojen");
               sendBoard(board[gb_code], i+1); //Deska se odešle hráči znovu
               break;
            }
        }

        //Kontrola, zda client poslal pro připojení správný kód (v cyklu je počet pokusů)
        byte checkCode = 0;
        for(int i = 0; i < 200; i++){
          if(newClient.available()){
            checkCode = newClient.read();
            break;
          }
          delay(10);
        }
        //Odstranění zbylých dat od clienta
        while(newClient.available()){
          byte bin = newClient.read();
        }
        //Kontrola, zda přijatý kód pro připojení je správný
        if(checkCode != 100){
          clientOK = false;
        }

        if(clientOK){
          clients[clientIndex] = newClient;
          syncBoardIPs();
          sendBoard(3);
          signalLED.changeBlinkColor(LEDcol_violet, 3);
          Serial.print("Novy client cislo ");
          Serial.print(clientIndex+1);
          Serial.print(" s IP adresou ");
          Serial.print(clients[clientIndex].remoteIP());
          Serial.print(" USPESNE pripojen \n");
        }
        else{
          Serial.print("Client s IP adresou ");
          Serial.print(newClient.remoteIP());
          Serial.print(" NEBYL pripojen \n");
          newClient.stop();
        }
      }
    }
    else if(serverPhase == 1){ //Fáze začátek hry
      //Pro herní čas
    }
    else{
      Serial.println("NEZNAM");
    }

  // stop any clients which disconnect
  for (byte i = 0; i < maxPlayers; i++) {
    if ((clients[i]) && !clients[i].connected()) {
      Serial.print(i);
      Serial.println(" STOP");
      clients[i].stop();
      board[IPaddr[i]] = 0;
      board[IPaddr[i]+1] = 0;
      board[IPaddr[i]+2] = 0;
      board[IPaddr[i]+3] = 0;
      signalLED.changeBlinkColor(LEDcol_orange, 3);
    }
  }


  //Kontrola stisku tlačítek
  if(pinReady){
    if(digitalRead(startPIN) == HIGH){
      pinReady = false;
      refresh_buttonOff = millis(); //čas užití tlačítka (pro funkci, která tlačítka po určené době obnoví)
      startGame();
    }
    if(digitalRead(resetPIN) == HIGH){
      pinReady = false;
      refresh_buttonOff = millis();
      stopGame();
    }
  }
  delay(10);
}


/*
 * >>>>>>>>>> FUNKCE <<<<<<<<<<
 */
 //>>>>> Nastaví LED <<<<<
 /*   Princip:
  *    -
  */
/*void setLED (byte color[], byte br){
  byte L_RED = (byte)(color[0]*(br/100.0)*(LED_br/100.0));
  byte L_GREEN = (byte)(color[1]*(br/100.0)*(LED_br/100.0));
  byte L_BLUE = (byte)(color[2]*(br/100.0)*(LED_br/100.0));
  analogWrite(LED_red, 255-L_RED);
  analogWrite(LED_green, 255-L_GREEN);
  analogWrite(LED_blue, 255-L_BLUE);
}*/
//------------------------------------------------------------------------------------------------------
//>>>>> Nastaví LED podle herní fáze <<<<<
/*   Princip:
 *    -
 */
/*void setLEDgamePhase(){
  switch (serverPhase) {
    case 0:
      setLED (LEDcol_blue, 100);
      break;
    case 1:
      setLED (LEDcol_green, 100);
      break;
    default:
      setLED (LEDcol_blue, 0);
  }
}*/
 //------------------------------------------------------------------------------------------------------
