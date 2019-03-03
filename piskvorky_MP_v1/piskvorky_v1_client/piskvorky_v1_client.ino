 /*>>>>>>> Piškvorky s arduino Ethernet <<<<<<<
 * - max 5 hráčů
 * - 1x arduino jako server, max 5x arduino jako client
 *
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

/* ----------Nastavení ethernetu----------*/ //(ZMĚNIT)
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
bool clientConnected = false;
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
//Údaje pro kalibraci1
#define TS_MINX 177
#define TS_MINY 886
#define TS_MAXX 347
#define TS_MAXY 550

int TSx, TSy = 0;
TSPoint touchPoint;
bool touchScreenAct = true; //Aktivuje/deaktivuje dotykovou plochu - zabránění vícedotykům najednou

//Kalibrace jednotlivých displajů (ZMĚNIT)

//Client 1
#define TOUCH_XMIN 170
#define TOUCH_XMAX 950
#define TOUCH_YMIN 146
#define TOUCH_YMAX 950

//Client 2
/*#define TOUCH_XMIN  170
#define TOUCH_XMAX  935
#define TOUCH_YMIN  146
#define TOUCH_YMAX  920*/

//Client 3 (nutné změnit i v kódu u čtení z displaye - zapojení displeje má jinou orientaci)
/*#define TOUCH_XMIN 945
#define TOUCH_XMAX 155
#define TOUCH_YMIN 156
#define TOUCH_YMAX 936*/


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

/* ----------Herní data----------*/
byte myNum = 0; //Číslo v herním poli "board", je přidělováno serverem při navázání spojení
byte gamePhase = 0; //fáze hry (podle toho se vykreslí obrazovka)(0:úvodní, 1: připojování k serveru, 2: připojeno, čekání na začátek hry, 3: pro=bíhající hra, překreslování displeje, 4: můj tah)
const byte maxPlayers = 5;

/* ----------Piškvorky----------*/
const byte packetLength = 136; //Musí být dělitelné osmi
bool boardAck [packetLength/8]; 
byte board [136]; //0: nikdo, 1: hráč 1; 2: hráč 2
/* >>>>> Rozložení herního packetu <<<<<
 *  0-89:   Obsazení herních polí (standadně 0, server doplňuje čísla)
 *  90:     Hlášení prostřednictvím kódu
 *            0:    nic nedělej
 *            1:    vše OK, hraje se, překresli obrazovku
 *            2:    pouze překreslit
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
#define gb_PC1        95
#define gb_PC2        97
#define gb_PC3        99
#define gb_PC4        101
#define gb_PC5        103

const byte colorAddr [] = {gb_PC1, gb_PC2, gb_PC3, gb_PC4, gb_PC5}; //Pole adres barev hráčů v poli board
const byte IPaddr [] = {105, 109, 113, 117, 121}; //Počáteční adresy (indexy) jednotlivých IP adres hráčů v boardu

bool screenRefresh = false; //Zda se má obrazovka překreslit

/* ----------Tlačítka----------*/
//Pole tlačítek
#define max_buttRect 3
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
bool recieveBoard (void); //Pokud byla serverem odeslána herní deska, přijme ji (vratí true), pokud není co přijmout, vrátí false
void drawPoints(void); //Vykreslí puntíky podle board
//void checkWin(byte); //Zkontroluje zda nějaký hráč nevyhrál (argument je políčko, na které bylo vloženo kolečko)
void chechStatus(byte); //KOntroluje oznamovací kód (umístěn v board[90])
bool connectToServer(void); //Začne se spojovat se serverem
void buttonPressed(int, int); //Argumentem souřadnice bodu, systém vyhodnotí stisk
void processBoard(void); //Zpracuje novou přijatou herní desku
uint16_t getPlayerColor(byte); //Zjistí barvu hráče, argument je číslo pozice v poli, kde údaj začíná
byte getMyPlayerNumber (void); //Podle IP adres v boardu zjistí moje číslo hráče, pokud nenajde shodu, vrátí 0
/* communication */
bool recieveBoard (void); //Pokud byla serverem odeslána herní deska, přijme ji (vratí true), pokud není co přijmout, vrátí false
void checkRecievedBoard(void); //Zkontroluje zda byla přijata celá herní deska (board)
void sendData(byte, byte); //Odešle daná data serveru (3x po sobě pro kontrolu), první argument je zpráva a druhé je kód, podle kterého server zpravu vyhodnotí
/*
 * >>>>>>>>>> SETUP <<<<<<<<<<
 */
void setup() {
  pinMode(A0, OUTPUT);       //.kbv mcufriend have RD on A0
  digitalWrite(A0, HIGH);
  //Sériová linka
  Serial.begin(9600);
  LCD.InitLCD();
  LCD.setFont(SmallFont);
  LCD.clrScr(); //Vyčištění obrazovky (vyplnění černou)

  //Ethernet
  LCD.setTextColor(YELLOW, BLACK);
    LCD.setTextSize(2);
    LCD.setCursor(20, 45);
    LCD.println("Zkontrolujte pripojeni     kabelu");
  while(!Ethernet.begin(mac)){ //IP adresa z DHCP serveru
    Serial.println("Zkontrolujte pripojeni kabelu");
    delay(1000);
  }
  delay(50);
  LCD.clrScr();
  Serial.print("Ziskana IP adresa: ");
  Serial.println(Ethernet.localIP());

  screenRefresh = true;
  delay(100);

  //Reset pole pro potvrzení příjmu desky
  for(byte i = 0; i < packetLength/8; i++){
    boardAck[i] = false;
  }

}

/*
 * >>>>>>>>>> LOOP <<<<<<<<<<
 */
void loop() {
    switch(gamePhase){
      case 0:
        if(screenRefresh){
          //Serial.println("Faze 0");
          drawPage(0);
        }
        break;

      case 1:
        if(screenRefresh){
          //Serial.println("Faze 1");
          drawPage(1);
        }
        if ((millis() - lastReconnect > tryReconnect)){ //Pokus o znovuspojení
          lastReconnect = millis();
          if(connectToServer()){
            gamePhase = 2;
            clientConnected = true;
            screenRefresh = true;
            }
        }
        break;

     case 2: //Nová hra
      if(screenRefresh){
        drawPage(2);
      }
      if(recieveBoard() && getMyPlayerNumber() > 0){
        myNum = getMyPlayerNumber();
        LCD.clrScr();
        processBoard();
      }
      break;

    case 3: //Běžící hra
      button_index = 0;
      if(recieveBoard()){
        processBoard();
      }
      break;

    case 4: //Můj tah
      button_index = 0;
      Serial.println("Faze 4");
      if(recieveBoard()){
        processBoard();
      }
      break;
   }



  //Dotykáč
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
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (touchPoint.z > MINPRESSURE && touchPoint.z < MAXPRESSURE && touchScreenAct) {
     refreshTouchScreen = millis(); //Nastaví poslední čas stisku
     //Standardní
     TSx = map(touchPoint.y, TOUCH_XMAX, TOUCH_XMIN, 0, 320); //Prohození proměnný...aby sedělo s rozlišením
     TSy = map(touchPoint.x, TOUCH_YMIN, TOUCH_YMAX, 0 ,240);
     //Pro CLIENT 3
     /*TSx = map(touchPoint.x, TOUCH_XMIN, TOUCH_XMAX, 0, 320); //Prohození proměnný...aby sedělo s rozlišením
     TSy = map(touchPoint.y, TOUCH_YMIN, TOUCH_YMAX, 0 ,240);*/
     //Kontrola stisku
     Serial.print("X = "); Serial.print(touchPoint.x);
     Serial.print("\tY = "); Serial.print(touchPoint.y);
     Serial.print("\tXpix = "); Serial.print(TSx);
     Serial.print("\tYpix = "); Serial.print(TSy);
     Serial.print("\tPressure = "); Serial.println(touchPoint.z);

     buttonPressed(TSx, TSy);
     touchScreenAct = false;
     if (gamePhase == 4){ //POkud je client na tahu
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
        //Odesíláni
        client.write(meshX*row + column);
        gamePhase = 3;
        }
     }

   }
}


  delay(10);

}


/*
 * >>>>>>>>>> FUNKCE <<<<<<<<<<
 */
 //------------------------------------------------------------------------------------------------------

//>>>>> Vykreslení danou obrazovku podle ID <<<<<
 /*   Princip:
  *    - 0: čekání na server
  *    - 1: hraji
  *    - 2: hraje druhý hráč
  */

  void drawPage (byte id){
    switch(id){

      case 0:
        button_index = 0;
        LCD.setColor(BLACK);
        LCD.fillRect(20, 40, 310, 90);
        drawMainFrame(BLUE);
        LCD.setTextColor(YELLOW, BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(80, 10);
        LCD.println("Piskvorky");
        LCD.setTextSize(2);
        LCD.setCursor(20, 45);
        LCD.println("Moje IP: ");
        LCD.setCursor(150, 45);
        LCD.println(Ethernet.localIP());
        LCD.setCursor(20, 70);
        LCD.println("Server IP: ");
        LCD.setCursor(150, 70);
        LCD.println(serverAddress);
        buttons[button_index] = buttonRect(50, 270, 150, 210, 1, 1);
        LCD.setTextColor(BLACK, LIGHTGREY);
        LCD.setTextSize(3);
        LCD.setCursor(90,170);
        LCD.println("PRIPOJIT");
        screenRefresh = false;
        break;

      case 1:
        button_index = 0;
        drawMainFrame(BLUE);
        LCD.setColor(BLACK);
        LCD.fillRect(20, 40, 310, 90);
        LCD.setTextColor(YELLOW, BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(80, 10);
        LCD.println("Piskvorky");
        LCD.setCursor(20, 50);
        LCD.setTextSize(2);
        LCD.println("Pripojuji se k serveru");
        buttons[button_index] = buttonRect(50, 270, 150, 210, 11, 1);
        LCD.setTextColor(RED, LIGHTGREY);
        LCD.setTextSize(3);
        LCD.setCursor(90,170);
        LCD.println("PRERUSIT");
        screenRefresh = false;
        break;

      case 2:
        LCD.clrScr();
        button_index = 0;
        LCD.setColor(BLACK);
        LCD.fillRect(20, 40, 310, 90);
        drawMainFrame(BLUE);
        LCD.setTextColor(getPlayerColor(colorAddr[myNum-1]), BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(80, 10);
        LCD.println("Piskvorky");
        LCD.setTextSize(2);
        LCD.setCursor(90, 50);
        LCD.println("Pripojeno");
        LCD.setCursor(20, 70);
        LCD.println("Cekam na zahajeni hry");
        buttons[button_index] = buttonRect(50, 270, 150, 210, 21, 1);
        LCD.setTextColor(BLACK, LIGHTGREY);
        LCD.setTextSize(3);
        LCD.setCursor(100,170);
        LCD.println("ODPOJIT");
        screenRefresh = false;
        break;
    }
  }


//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

//>>>>> Vyhodocení stisku tlačítka <<<<<
 /*   Princip:
  *    - Vezme souřadnice (argument) a vyhodnotí stisky tlačítka
  *    - uvažuje jednotlivé fáze hry
  *
  */
void buttonPressed(int x, int y){
  byte cislo = 0;
  for(int i = 0; i < button_index; i++){
        if(buttons[i].isTouched(x, y)){
          //Serial.println("Stisknuto");
          cislo = buttons[i].getID();
          break;
        }
  }
  //Serial.print("cislo tlacitka: "); Serial.println(cislo);
  switch(cislo){
    case 1:
      gamePhase = 1;
      screenRefresh = true;
      break;

    case 11:
      gamePhase = 0;
      screenRefresh = true;
      break;

    case 21:
      gamePhase = 0;
      screenRefresh = true;
      client.flush();
      client.stop();
      clientConnected = false;
      break;
  }

}

//------------------------------------------------------------------------------------------------------



