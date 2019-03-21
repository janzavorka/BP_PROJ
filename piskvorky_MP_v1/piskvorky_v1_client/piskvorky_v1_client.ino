 /*>>>>>>> Piškvorky s arduino Ethernet <<<<<<<
 * - max 5 hráčů
 * - 1x arduino jako server, max 5x arduino jako client
 * - Více informací na GitHubu: https://github.com/janzavorka/BP_PROJ
 */

#include <Ethernet.h>
//Displej
#include <UTFTGLUE.h>
//Dotyková plocha
#include "TouchScreen.h"

//Font
#if !defined(SmallFont)
extern uint8_t SmallFont[];   //.kbv GLUE defines as GFXFont ref
#endif
/* ---------- KONFIGURACE ----------*/
//Nastavení MAC adres pro jednotlivé clienty
//Každý client by měl mít unikatní IP adresu
//Client 1
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEE, 0xFE, 0xED
};

//Client 2
/*byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEE, 0xFE, 0xDD
};*/

//Client 3
/*byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEE, 0xFE, 0xCD
};*/

//Client 4
/*byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEE, 0xFE, 0xBD
};*/

//Client 5
/*byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEE, 0xFE, 0xAD
};*/


unsigned int localPort = 3333;      // local port to listen on
EthernetClient client;
bool connectingToServer = false;
bool serverReady = false;

//Herní server
IPAddress serverAddress(10,0,0,8);

/* ----------Nastavení dotykové plochy----------*/
#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin
//Maximální a minimální tlak
#define MINPRESSURE 10
#define MAXPRESSURE 1000

int TSx, TSy = 0;
TSPoint touchPoint;
bool touchScreenAct = true; //Aktivuje/deaktivuje dotykovou plochu - zabránění vícedotykům najednou

/* ---------- KONFIGURACE ----------*/
//Kalibrace jednotlivých displajů
//Kalibraci lle provést například pomocí příkladu v knihovně UTFGLU
//MCUFRIEND_kbv -> TouchScreen_calibr_kbv
//Uložené kalibrační hodnoty pro použité displeje
//Client 1
#define TOUCH_XMIN 221
#define TOUCH_XMAX 950
#define TOUCH_YMIN 200
#define TOUCH_YMAX 950

//Client 2
/*#define TOUCH_XMIN  233
#define TOUCH_XMAX  937
#define TOUCH_YMIN  210
#define TOUCH_YMAX  910*/

//Client 3 (nutné změnit i v kódu u čtení z displaye - zapojení displeje má jinou orientaci)
/*#define TOUCH_XMIN 950
#define TOUCH_XMAX 205
#define TOUCH_YMIN 190
#define TOUCH_YMAX 945*/


/* ----------Časové intervaly různých událostí----------*/
#define touchScreenOff 500 //Po jakou dobu bude dotyková plocha deaktivována (zabrání multidotykům)
unsigned long long refreshTouchScreen = 0;

#define tryReconnect 5000 //Po jaké době se client pokusí znovu spojit se serverem
unsigned long long lastReconnect = 0;

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

/* ----------Inicializační funkce----------*/
UTFTGLUE LCD(0x0154,A2,A1,A3,A4,A0);
//Dotyková plocha
TouchScreen Touch(XP, YP, XM, YM, 300);

/* ----------Nastavení displeje----------*/
#define resX 320 //Rozlišení
#define resY 240
//Rozměry sítě
#define meshX 11
#define meshY 8
//Index poslední vykreslené obrazovky (pro správné překreslení)
byte lastPage = 255;

/* ----------Herní data----------*/
byte myNum = 0; //Číslo v herním poli "board", je přidělováno serverem při navázání spojení
byte gamePhase = 0; //fáze hry (podle toho se vykreslí obrazovka)(0:úvodní, 1: připojování k serveru, 2: připojeno, čekání na začátek hry, 3: pro=bíhající hra, překreslování displeje, 4: můj tah)
const byte maxPlayers = 5;
bool serverConnection = false; //Zda bylo spojení úspěšně navázáno

/* ----------Piškvorky----------*/
const byte packetLength = 136; //Musí být dělitelné osmi
bool boardAck [packetLength/8];
byte board [136]; //0: nikdo, 1: hráč 1; 2: hráč 2
/* >>>>> Rozložení herního packetu <<<<<
 *  0-89:   Obsazení herních polí (standadně 0, server doplňuje čísla)
 *  90:     Hlášení prostřednictvím kódu
 *            0:    nic nedělej
 *            1:    vše OK, hraje se, překresli obrazovku
 *            2:    pouze překreslit, nikdo nehraje
 *            3:    připravit novou hru, čekání na hráče (úvodní obrazovka)
 *            9:    odpojuji
 *            11:   hraje hráč 1
 *            12:   hraje hráč 2
 *            13:   hraje hráč 3
 *            14:   hraje hráč 4
 *            15:   hraje hráč 5
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
 *  92:     Číslo vyplněného pole (vyplňuje client) //NEPLATí
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
/*#define gb_PC1        95
#define gb_PC2        97
#define gb_PC3        99
#define gb_PC4        101
#define gb_PC5        103*/

const byte colorAddr [] = {95, 97, 99, 101, 103}; //Pole adres barev hráčů v poli board
const byte IPaddr [] = {105, 109, 113, 117, 121}; //Počáteční adresy (indexy) jednotlivých IP adres hráčů v boardu

bool screenRefresh = false; //Zda se má obrazovka překreslit

/* ----------Tlačítka----------*/
//Pole tlačítek
#define max_buttRect 1
byte button_index = 0;


class buttonRect{
  private:
    int x1 = 0;
    int x2 = 0;
    int y1 = 0;
    int y2 = 0;
    byte id = 0;
  public:
    buttonRect();
    buttonRect(int corX1, int corX2, int corY1, int corY2, byte idecko, byte mode);
    bool isTouched (int touchX, int touchY);
    byte getID (void);
};

buttonRect::buttonRect(){}
buttonRect::buttonRect(int corX1, int corX2, int corY1, int corY2, byte idecko, byte mode){
  x1 = corX1;
  x2 = corX2;
  y1 = corY1;
  y2 = corY2;
  id = idecko;
  switch(mode){
    case 0:
      break;

    case 1:
      LCD.setColor(LIGHTGREY);
      LCD.fillRect(x1,y1,x2,y2);
      LCD.setColor(WHITE);
      LCD.drawRect(x1,y1,x2,y2);
      break;
  }
  button_index++;
}

byte buttonRect::getID(){
  return id;
}

bool buttonRect::isTouched(int touchX, int touchY){
  //Serial.print("Kontrola stisku x: "); Serial.print(y1);; Serial.print(" y: "); Serial.print(y2); Serial.print(" s id: "); Serial.println(id);
  if(touchX >= x1 && touchX <= x2 && touchY >= y1 && touchY <= y2){
    return true;
  }
  else{
    return false;
  }
}

buttonRect buttons[max_buttRect];



void drawMainFrame(uint16_t); //Vykreslí základní rámeček (v dané barvě)
void drawMesh (uint16_t); //Vykreslí základní hrací mřížku (argument je barva)
void drawPage (byte); //Vykreslí obrazovku podle čísla
void drawPogetMyPlayerNumber (void); //Podle IP adres v boardu zjistí moje číslo hráče, pokud nenajde shodu, vrátí -1
void drawPoints(void); //Vykreslí puntíky podle board
/* displayControl */
void drawHead (uint16_t); //Vykreslí hlavičku - nápis "piskvorky" danou barvou (argument)
//void checkWin(byte); //Zkontroluje zda nějaký hráč nevyhrál (argument je políčko, na které bylo vloženo kolečko)
void chechStatus(byte); //KOntroluje oznamovací kód (umístěn v board[90])
bool connectToServer(void); //Začne se spojovat se serverem
void buttonPressed(int, int); //Argumentem souřadnice bodu, systém vyhodnotí stisk
void processBoard(void); //Zpracuje novou přijatou herní desku
uint16_t getPlayerColor(byte); //Zjistí barvu hráče, argument je číslo pozice v poli, kde údaj začíná
byte getMyPlayerNumber (void); //Podle IP adres v boardu zjistí moje číslo hráče, pokud nenajde shodu, vrátí 0
/* communication */
void recieveBoard (void); //Pokud byla serverem odeslána herní deska, přijme ji (vratí true), pokud není co přijmout, vrátí false
void sendData(byte, byte); //Odešle daná data serveru (3x po sobě pro kontrolu), první argument je zpráva a druhé je kód, podle kterého server zpravu vyhodnotí
void resetBoardAck(void); //Vyplní pole pro potvrzování příjmu pole board hodnotami false (nepřijato)
/*
 * >>>>>>>>>> SETUP <<<<<<<<<<
 */
void setup() {
  pinMode(A0, OUTPUT);       //.kbv mcufriend have RD on A0
  digitalWrite(A0, HIGH);
  //Sériová linka
  //Serial.begin(9600);
  LCD.InitLCD();
  LCD.setFont(SmallFont);
  LCD.clrScr(); //Vyčištění obrazovky (vyplnění černou)

  //Ethernet
  LCD.setTextColor(YELLOW, BLACK);
    LCD.setTextSize(2);
    //LCD.setCursor(20, 45);
    LCD.print("Zkontrolujte pripojeni \n kabelu", 20, 45);
  while(!Ethernet.begin(mac)){ //IP adresa z DHCP serveru
   // Serial.println("Zkontrolujte pripojeni kabelu");
    delay(1000);
  }
  delay(50);
  LCD.clrScr();
  //Serial.print("Ziskana IP adresa: ");
  //Serial.println(Ethernet.localIP());

  screenRefresh = true;
  delay(100);

  //Reset pole pro potvrzení příjmu desky
  resetBoardAck();

  //Nulování časových konstant
  lastReconnect = 0;

  //Pokud vše proběhlo v pořádku vykreslí se úvodní obrazovka
  drawPage(0);
}

/*
 * >>>>>>>>>> LOOP <<<<<<<<<<
 */
void loop() {
  //Pokud uplynulo dostatek času od posledního pokusus o spojení A připojení je vyžaováno A client ještě není připojen
  if ((millis() - lastReconnect > tryReconnect) && connectingToServer && !client.connected()){
    lastReconnect = millis();
    if(connectToServer()){
      connectingToServer = false;
      serverConnection = true;
      }
  }

  //Kontrola spojení se serverem
  if(!client.connected() && serverConnection){
    disconnectFromServer();
  }

  //Zkontroluj zda nejsou přijatá data
  recieveBoard();

  //Obnovení funkce dotykové plochy
  if ((millis() - refreshTouchScreen > touchScreenOff) && ! touchScreenAct){ //Obnova funkce dotykové plochy
    touchScreenAct = true;
    refreshTouchScreen = millis();
  }

  if(touchScreenAct){
   touchPoint = Touch.getPoint();
   //nutno obnovit piny (sdíleno s displayem)
   pinMode(YP, OUTPUT);
   pinMode(XM, OUTPUT);
   digitalWrite(YP, HIGH);
   digitalWrite(XM, HIGH);
  // Zkontroluje tlak na dotykovou plochu (0 znamená žádný tlak/dotyk), stisk se vyhodnotí až od určité hodnoty
  if (touchPoint.z > MINPRESSURE && touchPoint.z < MAXPRESSURE && touchScreenAct) {
     refreshTouchScreen = millis(); //Nastaví poslední čas stisku
     touchScreenAct = false;
     //Standardní
     TSx = map(touchPoint.y, TOUCH_XMAX, TOUCH_XMIN, 0, 320); //Prohození proměnný...aby sedělo s rozlišením
     TSy = map(touchPoint.x, TOUCH_YMIN, TOUCH_YMAX, 0 ,240);
     //Pro CLIENT 3
     /*TSx = map(touchPoint.x, TOUCH_XMIN, TOUCH_XMAX, 0, 320); //Prohození proměnný...aby sedělo s rozlišením
     TSy = map(touchPoint.y, TOUCH_YMIN, TOUCH_YMAX, 0 ,240);*/
     //Kontrola stisku (seriova linka pro debug)
     /*Serial.print("X = "); Serial.print(touchPoint.x);
     Serial.print("\tY = "); Serial.print(touchPoint.y);
     Serial.print("\tXpix = "); Serial.print(TSx);
     Serial.print("\tYpix = "); Serial.print(TSy);
     Serial.print("\tPressure = "); Serial.println(touchPoint.z);*/
     buttonPressed(TSx, TSy);
    }
  }


  delay(20);

}


/*
 * >>>>>>>>>> FUNKCE <<<<<<<<<<
 */
 //------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

//>>>>> Vyhodocení stisku tlačítka <<<<<
 /*   Princip:
  *    - Vezme souřadnice (argument) a vyhodnotí stisky tlačítka
  *    - Každé tlačítko má unikátní ID, podle toho dojde k vyhodnocení
  *    - V případě, že je rozehraná hra (je vykreslena herní mříž, uvažuje dotyky jako pokládání žetonů)
  *    - Bere v úvahu zda je daný hráč na řadě
  */
void buttonPressed(int x, int y){
  byte cislo = 255; //Defaultní hodnota (nelze 0 jinak bude vždy spouštět tlačítko 0)
  for(int i = 0; i < button_index; i++){
        if(buttons[i].isTouched(x, y)){
          //Serial.println("Stisknuto");
          cislo = buttons[i].getID();
          break;
        }
  }
  //Serial.print("cislo tlacitka: "); Serial.println(cislo);

  switch(cislo){
    case 00:
      drawPage(1);
      connectingToServer = true;
      break;

    case 10:
      connectingToServer = false;
      if(client.connected()){
        client.stop();
        serverConnection = false;
      }
      drawPage(0);
      break;

    case 20:
      client.stop();
      connectingToServer = false;
      serverConnection = false;
      drawPage(0);
      break;
    default:
      break;
  }

  if(board[gb_actPlayer] == getMyPlayerNumber() && board[gb_code] == 1){ //Zkontroluje zda daný hráč je na řadě a kód je 1 (hra běží)
    //Pokusit se vyhdnotit vložení herního žetonu
    byte row = 0;
    byte column = 0;
    for(int i = 0; i < meshX; i++){ //Zjištění místa v hracím poli
      if(i*resX/meshX<= TSx && (i+1)*resX/meshX >= TSx){
        column = i;
        break;
       }
     }

      for(int i = 0; i < meshY; i++){
        if(i*resY/meshY<= TSy && (i+1)*resY/meshY >= TSy){
          row = i;
          break;
        }
      }
      if(board[meshX*row + column]==0){ //Pokud je pole volné (není tam jiný hráč)
      //Vyplní ihned daný žeton na obraovku (jinak by docházelo k vykreslení až po zpracování serverem, zpoždění několik stovek ms, uživatelsky nepříjemné)
      LCD.setColor(getPlayerColor(getMyPlayerNumber())); //Nastaví barvu hráče podle čísla v poli
      LCD.fillCircle(column * resX/meshX + (resX/meshX)/2, row * resY/meshY + (resY/meshY)/2, 10);
      //Odesíláni vybraného pole
      sendData(meshX*row + column, 10);
      }
  }

}

//------------------------------------------------------------------------------------------------------
