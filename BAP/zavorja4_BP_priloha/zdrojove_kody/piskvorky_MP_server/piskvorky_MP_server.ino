/*>>>>>>> Piškvorky s arduinem po LAN <<<<<<<
*  !!! Součást programu pro server, samostatně nefunkční !!!
*
* - Autor: Jan Závorka
* - Email: zavorja4@fel.cvut.cz
* - Domovská stránka projektu: https://github.com/janzavorka/BP_PROJ
* - Seznam souborů: piskvorky_MP_server.ino; boardControl.ino; communication.ino; gameControl.ino; indicatioLED.ino; SerialControl.ino
*
* --- Popis:
* - Hra piškvorky pro max 5 hráčů
* - Jednotliví hráči jsou vybaveni deskami ArduinoEthernet s dotykovým displejem
* - Hru řídí server postavený na Arduino Due s Ethernet shieldem
* - Komunikace probíhá po lokální síti
*/

/* ---------- DEBUGING ----------*/
//Aktivovani jednotlivych debugovacích částí (pro aktivování odkomentovan daný #define)
//#define DEBUG
//#define DEBUG0CTS //Zapnutí výpisu pro debugování komunikace client->server
//#define DEBUG0STC //Zapnutí výpisu pro debugování komunikace server->client
//#define DEBUG0GAMEFL //Zapnutí výpisu pro debugování průběhu hry

//Pro debuging přes sériovou linku
#ifdef DEBUG
  #define DEBUG_PRINT(x)     Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#ifdef DEBUG0CTS
  #define DEBUG0CTS_PRINT(x)     Serial.print (x)
  #define DEBUG0CTS_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG0CTS_PRINT(x)
  #define DEBUG0CTS_PRINTLN(x)
#endif

#ifdef DEBUG0STC
  #define DEBUG0STC_PRINT(x)     Serial.print (x)
  #define DEBUG0STC_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG0STC_PRINT(x)
  #define DEBUG0STC_PRINTLN(x)
#endif

#ifdef DEBUG0GAMEFL
  #define DEBUG0GAMEFL_PRINT(x)     Serial.print (x)
  #define DEBUG0GAMEFL_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG0GAMEFL_PRINT(x)
  #define DEBUG0GAMEFL_PRINTLN(x)
#endif
/* ---------- END DEBUGING ----------*/

/* ---------- vložení knihoven ----------*/
#include <Ethernet.h> //Komunikace přes ethernet
#include <SimpleTimer.h> //Časovače

/* ----------Datum změny----------*/
char makeDate[] = "05.05.2019";

/* !!! ---------- KONFIGURACE - nastavení ethernetu ---------- !!! */
//Režimy sítě: ETHMODE_DHCP=server získá IP adresu z DHCP serveru; ETHMODE_STATIC=použije se nastavená IP adresa
//Při módu, kdy je IP adresa přiřazena DHCP serverem, je nutné aby se IP adresa neměnila a aby byla správně nastavena na straně clientů
#define ETHMODE_STATIC //Varianty: ETHMODE_DHCP; ETHMODE_STATIC

// Nastavení MAC adresy (měla by být unikátní)
byte mac[] = {
  0x00, 0x9A, 0xCD, 0x6C, 0xCA, 0xCA
};
//
//Nastavení IP adresy serveru, pokud je nastaveno 0.0.0.0 bude IP přiřazena DHCP serverem
//IP adresa serveru musí být vyplněna v kódu pro klienty
IPAddress serverAddress(10,0,0,8);
//
//Port na kterém bude probíhat komunikace
unsigned int localPort = 55555; //Defaultně 55555
/* !!! ---------- KONEC nastavení ethernetu ---------- !!! */

EthernetServer server = EthernetServer(localPort);
const byte  maxPlayers = 4; //Softwarové maximum 5, pro WIZnet W5100 max 4
EthernetClient clients [maxPlayers];
bool clientConnected = false;
bool serverReady = false;

//Casovac
SimpleTimer timer;
namespace std {void __throw_bad_function_call() { while(1); }; } //Pro správné fungování simpleTimer knihovny

//piny pro tlačítka
bool pinReady = false; //Slouží k deaktivaci tlačítek aby se zabránilo vícedotykům současně

/* !!! ---------- KONFIGURACE - nastavení pinů a LED ---------- !!! */
//Piny tlačítka start a reset
#define startPIN 2
#define resetPIN 3
//
//Piny na kterých jsou připojené jednotlivé barvy LED, musí podporovat PWM
#define LED_red 7
#define LED_green 5
#define LED_blue 6
//
//Nastavení maximálního jasu všech LED, interval <0; 100>
const byte LED_br = 40;
/* !!! ---------- KONEC nastavení pinů a LED ---------- !!! */

/* ----------Časové intervaly různých událostí----------*/
const int buttonOff = 1000; //Po jakou dobu bude dotyková plocha deaktivována (zabrání multidotykům)
unsigned long long refresh_buttonOff = 0;

const long clientMessageLast = 10000; //Doba zobrazení zprávy na straně clienta (v milisekundách)
const long clientErrMessageLast = 4000; //Doba zobrazení chybového hlášení na straně clienta


/* ----------Ovládání pomocí sériové linky----------*/
const byte max_buffik = 30;
String buffik = "";

//Rozměry herní sítě
const byte meshX = 11;
const byte meshY =  8;
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

/* !!! ---------- KONFIGURACE - nastavení barev hráčů ---------- !!! */
//Barvy jaké budou mít jednotliví hráči
//Lze zadat barvu ze seznamu nebo barvu v 16-bit tvaru
#define PLAYER1COLOR RED
#define PLAYER2COLOR GREEN
#define PLAYER3COLOR PURPLE
#define PLAYER4COLOR GREENYELLOW
#define PLAYER5COLOR OLIVE
/* !!! ---------- KONEC nastavení barev hráčů ---------- !!! */

/* ----------Piškvorky----------*/
byte packetLength = 136;
byte board [136];  //Herní deska
/* >>>>> Rozložení herního packetu <<<<<
 *  0-89:   Obsazení herních polí (standadně 0, server doplňuje čísla)
 *  90:     Hlášení prostřednictvím kódu
 *            0:    nic nedělej
 *            1:    vše OK, hraje se, překresli obrazovku
 *            2:    pouze překreslit, nikdo nehraje
 *            3:    připravit novou hru, čekání na hráče (úvodní obrazovka)
 *            9:    odpojit se
 *            100:  hra skončila remízou
 *            101:  vyhrál hráč 1
 *            102:  vyhrál hráč 2
 *            103:  vyhrál hráč 3
 *            104:  vyhrál hráč 4
 *            105:  vyhrál hráč 5
 *            201:  problémy s hráčem 1
 *            202: ....
 *            205: problémy s hráčem 5
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
//Označení jednotlivých pozic v boardu
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
//Počet bajtů, které budou mít jednotlivé packety při odesílání
const byte boardPart = 8; //Délka části pole board

/* ----------Ovládání indikační LED----------*/
//Barvy pro LED (tvar RGB)
byte LEDcol_red[] = {255, 0, 0};
byte LEDcol_green[] = {0, 255, 0};
byte LEDcol_blue[] = {0, 0, 255};
byte LEDcol_orange[] = {255, 80, 0};
byte LEDcol_violet[] = {255,0,255};
//byte LEDcol_red[] = {255, 0, 0};

//class pro RGB LED
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

//indikační RGB LED připojená k serveru
RGB_LED signalLED;

/* ----------PROTOTYPY----------*/
/* gameControl.ino */
void startGame(void); //Spustí hru
void stopGame(void); //Zruší běžící hru
byte getNextPlayer(byte); //Vrátí číslo dalšího hráče (argument fce je číslo předchozího hráče)
void checkGame(byte, byte); //Zkontroluje průběh hry...případně vyplní kód v boardu, argument je poslední pole a číslo hrajícího hráče
void shiftPlayer(void); //Změní číslo hráče a odešle herní desku
bool fillPlayerToken(byte, byte); //Vyplní do herní desky číslo hráče, který dané pole obsadil (argument1 = číslo pole, argument2 = číslo hráče)

/* communication.ino */
void sendBoardR(void); //Odešle data veme beze změny kódu
void sendBoard(byte); //Odešle herní desku všem hráčům (argument číslo řídicího kódu)
void sendBoard(byte, byte); //Odešle herní desku konkrétnímu hráči (argument číslo řídicího kódu a číslo clienta/hráče)
void sendBoardPart(byte, byte); //Odešle danou část herí desky (board) danému hráči
void syncBoardIPs(void); //Synchronizuje herní desku board s připojenými clienty
void checkIncommingData(void); //Kontroluje, zda nějaký client neposlal data
void recieveData(byte); //Přijme data a případně zpracuje, argument je index hráče
void resetClientData(byte); //Smaže přijatá data daného clienta
void processClientData(byte); //Provede požadované úkony pro daného hráče
void disconnectPlayer(byte); //Odpojí dané hráče (včetně resetování boardu a informování ostatních hráčů)
void sendErrMessage(byte); //Pošle chybovou zprávu
byte getHWcontroller(void); //Vrátí číslo použitého ethernet kontroléru

/* boardControl.ino */
void setBoard(void); //Připraví herní pole pro novou hru (reset a doplnění potřebných dat (barvy apod.))
void cleanBoard(void); //Resetuje herní pole (vyplní ho nulami)

/* SerialControl.ino */
void print_WC(void); //Voláno v případě chybného příkazu (ten vypíše společně s chybovou hláškou)
void processBuffik(void); //Zpracuje přijatou zprávu přes sériovou linku
void printLine(byte, byte); //Vypíše několik zadaných znaků za sebou (pro výpis oddělovacích čar na sériovém monitoru)

/*
 * >>>>>>>>>> SETUP <<<<<<<<<<
 */
void setup() {
  delay(100);
  //Sériová linka
  Serial.begin(9600);
  //Alokace paměti pro string buffik
  buffik.reserve(max_buffik);
  //Tlačítka
  pinMode(startPIN, INPUT_PULLUP);
  pinMode(resetPIN, INPUT_PULLUP);
  randomSeed(analogRead(0)); //Seed pro funkci random()
  delay(400);

  //Ethernet.init(10);
  //Nastavení LED
  signalLED = RGB_LED(LED_red, LED_green, LED_blue, 100);
  delay(200);
  signalLED.LEDoff();
  delay(400);


  Serial.println(" >>>>> Startuji piskvorkovy server <<<<< ");

  //Kontrola připojení kabelu (dostupné jen pro některé WIZNET kontroléry)
  if(getHWcontroller() > 1){ //Pokud je kontrolér W5200 nebo W5500 (pro ostatní není funkce podporována)
      if(Ethernet.linkStatus() == LinkOFF){//Kontrola pripojeni kabelu
        Serial.println("Zkontrolujte pripojeni kabelu");
      }
      while(Ethernet.linkStatus() == LinkOFF){
          signalLED.changeStaticColor(LEDcol_red);
          signalLED.LEDon();
          delay(500);
          signalLED.LEDoff();
          delay(500);
      }
  }

  //Ethernet

 #ifdef ETHMODE_DHCP
 //Rezim, kdy je IP adresa přiřazena DHCP serverem
    Serial.println("Ziskavam adresu z DHCP serveru");
    signalLED.changeStaticColor (LEDcol_red);
    signalLED.LEDon();
    delay(100);
    while(!Ethernet.begin(mac)){
      Serial.println("IP adresa nebyla prirazena, zkontrolujte pripojeni!");
      signalLED.LEDoff();
      delay(400);
      signalLED.LEDon();
      delay(400);
      signalLED.LEDoff();
      delay(400);
      signalLED.LEDon();
    }
#elif defined(ETHMODE_STATIC)
 //Pouzije se daná IP adresa
 delay(200);
 Ethernet.begin(mac, serverAddress);
 delay(200);
#else
  #error "Error - je nutne vybrat sitovy rezim (viz. KONFIGURACE - nastavení sítě)"
#endif

  delay(500);
  Serial.print("Server IP adresa: ");
  Serial.println(Ethernet.localIP());
  delay(200);
  do{
    server.begin();
    delay(500);
    Serial.println("Spoustim server");
  }while(!server);
  Serial.println("Server spusten!");
  delay(200);

  //Reset datových polí
  setBoard();
  for(byte i = 0; i < maxPlayers; i++){
    resetClientData(i);
  }

  delay(100);
  signalLED.changeStaticColor (LEDcol_blue);
  delay(50);
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
  checkIncommingData(); //Kontroluje, zda nejsou k dispozici data od klientů
  // --- Připojování nového hráče ---
    if(serverPhase == 0){ //Pokud neběží hra
      EthernetClient newClient = server.accept();
      delay(10);
      if ((newClient)) {
        Serial.println("Pripojen novy uzivatel");
        bool clientOK = true; //Zda client může být připojen (pokud neprojde nějakým testem, změní se na false)
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

        if(clientOK){ //Pokud novy klient prošl testy
          clients[clientIndex] = newClient; //Nový klient je přiřazen na danou pozici
          syncBoardIPs(); //synchronizují se IP adresy
          sendBoard(3);
          signalLED.changeBlinkColor(LEDcol_violet, 3); //Signalizace pomocí LED
          Serial.print("Novy client cislo ");
          Serial.print(clientIndex+1);
          Serial.print(" s IP adresou ");
          Serial.print(clients[clientIndex].remoteIP());
          Serial.print(" USPESNE pripojen \n\n");
        }
        else{
          Serial.print("Client s IP adresou ");
          Serial.print(newClient.remoteIP());
          Serial.print(" NEBYL pripojen \n\n");
          newClient.stop();
        }
      }
    }
    else if(serverPhase == 1){ //Fáze začátek hry
      //Kód vykonávaný v průběhu hry
    }
    else{
      DEBUG_PRINTLN("NEZNAM");
    }

  //Kontroluje odpojení jednotlivých klientů
  for (byte i = 0; i < maxPlayers; i++) {
    if ((clients[i]) && !clients[i].connected()) { //DEFAULT (clients[i]) && !clients[i].connected(), test (board[IPaddr[i]] != 0)
      delay(20);
      disconnectPlayer(i+1);
    }
  }


  //Kontrola stisku tlačítek, nepoužívá se interrupt - mohl by rozhodit odesílání
  if(pinReady){ //Pokud je tlačítko aktivní (zabraňuje vícedotykům)
    if(digitalRead(startPIN) == LOW){
      pinReady = false;
      refresh_buttonOff = millis(); //čas užití tlačítka (pro funkci, která tlačítka po určené době obnoví)
      if(serverPhase == 1){//Pokud je spuštěná hra, přepni na dalšího hráče
          shiftPlayer();
      }
      else{ //Jinak začni hru
        startGame();
      }
    }
    if(digitalRead(resetPIN) == LOW){
      pinReady = false;
      refresh_buttonOff = millis();
      stopGame();
    }
  }
  delay(30);
}
