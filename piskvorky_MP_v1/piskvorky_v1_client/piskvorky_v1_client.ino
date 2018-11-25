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
IPAddress serverAddress(10,0,0,1);

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
#define TOUCH_XMIN 
#define TOUCH_XMAX
#define TOUCH_YMIN
#define TOUCH_YMAX

//Client 2
/*#define TOUCH_XMIN 
#define TOUCH_XMAX
#define TOUCH_YMIN
#define TOUCH_YMAX*/

//Client 2
/*#define TOUCH_XMIN 
#define TOUCH_XMAX
#define TOUCH_YMIN
#define TOUCH_YMAX*/


/* ----------Časové intervaly různých událostí----------*/
#define touchScreenOff 500 //Po jakou dobu bude dotyková plocha deaktivována (zabrání multidotykům)
unsigned long long refreshTouchScreen = 0;

#define tryReconnect 2000 //Po jaké době se client pokusí znovu spojit se serverem
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
byte gamePhase = 0; //fáze hry (podle toho se vykreslí obrazovka)(0:úvodní, 1: připojování k serveru, 2: čekání na tah, 3: tah)
const byte maxPlayers = 5;

/* ----------Piškvorky----------*/
byte packetLength = 95; 
byte board [103]; //0: nikdo, 1: hráč 1; 2: hráč 2
/* >>>>> Rozložení herního packetu <<<<<
 *  0-89:   Obsazení herních polí (standadně 0, server doplňuje čísla)
 *  90:     Hlášení prostřednictvím kódu 
 *            0:    vše OK, hraje se, překresli obrazovku
 *            2:    hraj (vyplň pole)
 *            3:    připravit novou hru, čekání na hráče (úvodní obrazovka)
 *            9:    odpojuji
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
 *  92:     Číslo vyplněného pole (vyplňuje client)
 *  93:     Počet odehraných kol, zvyšuje se na straně serveru
 *  95-96:  Barva hráče 1
 *  97-98:  Barva hráče 2
 *  99-100: Barva hráče 3
 *  100-101:Barva hráče 4
 *  102-103:Barva hráče 5
 */
//Označení pozic
#define gb_code       90
#define gb_actPlayer  91
#define gb_filled     92
#define gb_round      93
#define gb_PC1        95  
#define gb_PC2        97
#define gb_PC3        99
#define gb_PC4        100
#define gb_PC5        102

bool screenRefresh = false; //Zda se má obrazovka překreslit

/* ----------Tlačítka----------*/
//Pole tlačítek
#define max_buttRect 2
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
  if(touchX >= x1 && touchX <= x2 && touchY >= y1 && touchY <= y2){
    return true;
  }
  else{
    return false;
  }
}

//Pole tlačítek
buttonRect buttons[max_buttRect];

//DOTAŽENO SEM

void drawMainFrame(uint16_t); //Vykreslí základní rámeček (v dané barvě)
void drawMesh (uint16_t); //Vykreslí základní hrací mřížku (argument je barva)
void drawPage (byte); //Vykreslí obrazovku podle čísla
void drawPoints(void); //Vykreslí puntíky podle board
//void checkWin(byte); //Zkontroluje zda nějaký hráč nevyhrál (argument je políčko, na které bylo vloženo kolečko)
void chechStatus(byte); //KOntroluje oznamovací kód (umístěn v board[90])
bool connectToServer(void); //Začne se spojovat se serverem
void buttonPressed(int, int); //Argumentem souřadnice bodu, systém vyhodnotí stisk
void processBoard(void); //Zpracuje novou přijatou herní desku
uint16_t getPlayerColor(byte); //Zjistí barvu hráče, argument je číslo pozice v poli, kde údaj začíná
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

  //Úvodní obrazovka
  LCD.clrScr(); //Vyčištění obrazovky (vyplnění černou)
  //Sériová linka
  //Ethernet
  Ethernet.begin(mac); //IP adresa z DHCP serveru
  delay(50);
  Serial.print("Ziskana IP adresa: ");
  Serial.println(Ethernet.localIP());
  
  //Nulování herní desky
  Serial.println("Nulovani pole pro herni desku");
  for (int i = 0; i < packetLength; i++){
    board[i] = 0;
  }
  screenRefresh = true;
  delay(100);

  
}

/*
 * >>>>>>>>>> LOOP <<<<<<<<<<
 */
void loop() {
    switch(gamePhase){
      case 0:
        if(screenRefresh){
          drawPage(0);
        }
        break;

      case 1:
        if(screenRefresh){
          drawPage(1);
        }
        if ((millis() - lastReconnect > tryReconnect)){ //Pokus o znovuspojení
          refreshTouchScreen = millis();
          if(connectToServer()){
            gamePhase = 2;
            clientConnected = true;
            screenRefresh = true;
            }
        }
        break;

     case 2:
      byte index = 0;
      while (index < packetLength){
        if(client.available() > 0){
          board[index] = client.read();
          index++;
        }
        delay(5);
      }
      processBoard();
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
     TSx = map(touchPoint.x, 139, 680, 0, 320); //Prohození proměnný...aby sedělo s rozlišením
     TSy = map(touchPoint.y, 905, 360, 0 ,240);
     Serial.print("X = "); Serial.print(touchPoint.x);
     Serial.print("\tY = "); Serial.print(touchPoint.y);
     Serial.print("\tPressure = "); Serial.println(touchPoint.z);
     buttonPressed(TSx, TSy);
     touchScreenAct = false;
     if (gamePhase == 1){ //Místo v hracím poli
      byte row = 0;
      byte column = 0;
        for(int i = 0; i < meshX; i++){
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
      if(board[meshX*row + column]==0){ //Pokud je pole volné (není tam druhý hráč) 
        board[meshX*row + column] = 2; //Zabrat pole
        drawPoints(); //Překreslí puntíky 
        board[89]++; //Posune herní kolo
        gamePhase = 2; //Nastaví fázi 2 (hraje druhý hráč)
        //drawPage(2);
        screenRefresh = true;
        //checkWin(meshX*row + column); //Zkontroluje, zda nedošlo k výhře (kontrola se provádí pouze pro vepsaný puntík (sloupec, řádek, křížem)

        //Odesíláni
        client.write(board, packetLength);
        //checkStatus(board[90]); //Zkontroluje kód (podle hodnoty výhra,prohra, pokračování)
      }
        
     }
     
   }
}


if(clientConnected && gamePhase == 2){ //fáze 2: čekání na příjem
    byte index = 0;
    while (index < packetLength){
      if(client.available() > 0){
        board[index] = client.read();
        index++;
      }
     delay(5);
    }
    screenRefresh = true;
    gamePhase = 1;
    //checkStatus(board[90]); //Kontrola kódu
   }
  
  delay(10);
  
}


/*
 * >>>>>>>>>> FUNKCE <<<<<<<<<<
 */
 //------------------------------------------------------------------------------------------------------
//>>>>> Vykreslení základního rámčku <<<<<
 /*   Princip:   
  *    - Vykreslí modrý rámeček kolem celého displeje
  */
void drawMainFrame(uint16_t color){
  LCD.setColor(color); //Nastavení barvy
  LCD.drawRect(0, 0, 320, 240); //Vykreslení čtverce (souřadnice levý horní a pravý dolní roh)
  LCD.drawRect(1, 1, 319, 239);
  
}

//------------------------------------------------------------------------------------------------------
//>>>>> Vykreslení základního hrací mřížku <<<<<
 /*   Princip:   
  *    - vykreslí modrou mříž jak hrací plochu 
  *    - velikost 11x8 polí
  */
void drawMesh(uint16_t color){
  LCD.setColor(color); //Nastavení barvy
  for(int i = 0; i < meshX; i++){ //Svislé
    LCD.drawLine((int)i*resX/meshX, 0, (int)i*resX/meshX, resY); 
  }
  for(int i = 0; i < meshY; i++){ //Vodorovné
    LCD.drawLine(0,(int)i*resY/meshY, resX, (int)i*resY/meshY); 
  }
}

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
        drawMainFrame(BLUE);
        LCD.setTextColor(YELLOW, BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(50, 10);
        LCD.println("Piskvorky");
        LCD.setCursor(50, 70);
        LCD.setTextSize(2);
        LCD.println("Server IP: ");
        LCD.setCursor(150, 70);
        LCD.println(serverAddress);
        buttons[button_index] = buttonRect(50, 100, 200, 150, 1, 1);
        screenRefresh = false;
        break;
  
      case 1:
        button_index = 0;
        drawMainFrame(BLUE);
        LCD.setTextColor(YELLOW, BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(50, 10);
        LCD.println("Piskvorky");
        LCD.setCursor(50, 70);
        LCD.setTextSize(2);
        LCD.println("Pripojuji se k serveru");
        buttons[button_index] = buttonRect(50, 100, 200, 150, 1, 1);
        screenRefresh = false;
        break;
    }
  }


//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
//>>>>> Vykreslí do mřížky kolečka na základě hodnoty v board <<<<<
 /*   Princip:   
  *    - 
  *    
  */
void drawPoints(){
  byte row = 0;
  byte column = 0;
  byte colorAddr[] = {gb_PC1, gb_PC2, gb_PC3, gb_PC4, gb_PC5} 
  uint16_t colors[maxPlayer];

  for(int i = 0; i < maxPlayers; i++){ //Zjištění jednotlivých barev
    colors[i] = getPlayerColor(colorAddr[i]);
  }

  for (int i = 0; i < meshX*meshY; i++){ //Překreslí všechny puntíky
      if(board[i] != 0){
        row = i/meshX;
        column = i - row*meshX;
        LCD.setColor(colors[board[i] +1); //Nastaví barvu hráče podle čísla v poli
        LCD.fillCircle(column * resX/meshX + (resX/meshX)/2, row * resY/meshY + (resY/meshY)/2, 10);
      }
   }
}


//------------------------------------------------------------------------------------------------------
//>>>>> Zracovává přijatou herí desku <<<<<
 /*   Princip:   
  *    - POstupně zpracuje přijatou herní duesku
  *    
  */

void processBoard(){
  switch(board[gb_cpde]){
    case 0: //Jen překreslit
      drawMainFrame(LIGHTGREY);
      drawMesh(LIGHTGREY);
      drawPoints();
      break;

    case 2:
      break;
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> vypisuje hlášení podle kodu <<<<<
 /*   Princip:   
  *    - 
  *    
  */
void checkStatus(byte code){
  switch(code){
    case 100: //Vše OK
      break;

    case 200: //Remíza
        LCD.setColor(BLACK);
        LCD.fillRect(0,0, 320, 50);
        LCD.setTextColor(YELLOW, BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(20, 20);
        LCD.println("Remiza");
        delay(10000);
        //prepareNewGame();
      break;

    case 201: //Vyhrál server
        LCD.setColor(BLACK);
        LCD.fillRect(0,0, 320, 50);
        LCD.setTextColor(RED, BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(20, 20);
        LCD.println("Porazeny");
        delay(10000);
        //prepareNewGame();
      break;

   case 202: //Vyhrál client
        LCD.setColor(BLACK);
        LCD.fillRect(0,0, 320, 50);
        LCD.setTextColor(GREEN, BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(20, 20);
        LCD.println("Vitez");
        delay(10000);
        //prepareNewGame();
      break;

   default:
      break;
  }
}


//------------------------------------------------------------------------------------------------------
//>>>>> Vyhodocení stisku tlačítka <<<<<
 /*   Princip:   
  *    - Vezme souřadnice (argument) a vyhodnotí stisky tlačítka
  *    - uvažuje jednotlivé fáze hry 
  *    
  */
void buttonPressed(int x, int y){
  byte id = 0;
  for(int i = 0; i < button_index; i++){
        if(buttons[i].isTouched(x, y)){
          id = buttons[i].getID();
          break;
        }
  }
  switch(gamePhase){
    case 0:
      switch(id){
        case 1:
          gamePhase = 1;
          screenRefresh = true;
          break;
      }
      break;

    case 1:
      switch(id){
        case 1:
          gamePhase = 0;
          screenRefresh = true;
          break;
      }
  }
  
}

//------------------------------------------------------------------------------------------------------
//>>>>> Připojení k serveru <<<<<
 /*   Princip:   
  *    - nekonečná smyčka snaží cí se připojit k serveru
  *    - pokud je připojení úspěšné, změní se fáze hry
  *    - v případě úspěšného spoojení vrátí true
  *    - hodnoty vrácené serverem: 0 = spojení odmítnuto (třeba z důvodu běžící hry); 1-5 = přiřazené číslo hráče
  */
bool connectToServer(){
  byte server_code = 0;
  if(!clientConnected){
    Serial.println("Pokus o spojeni");
    if (client.connect(serverAddress, 3333)){
      Serial.println("Pripojuji");
      client.write(33); //Aby připojení server správně zaznamenal (kód 33: chci se připojit)
      while(1){ //Čekání na příjem potvrzovacího kódu
        if(client.available()){
          server_code = client.read();
          if(server_code > 0){
            myNum = server_code;
            clientConnected = true;
            return true;
          }
          else{
            Serial.println("Spojeni odmitnuto");
            clientConnected = false;
            client.stop();
            return false;
          }
        }
      }
    }  
  }
}

//------------------------------------------------------------------------------------------------------
//>>>>> Zjištění barvy hráče <<<<<
 /*   Princip:   
  *    - argument je index v poli board, podle něho je zjištěna daná barva 
  *    - návratová hodnota je ona barva
  */

uint16_t getPlayerColor(byte start){
  return uint16_t(board[start] << 8 | board[start+1]);
}
//------------------------------------------------------------------------------------------------------
