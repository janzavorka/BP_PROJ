#include <Ethernet.h>
//#include <stdlib.h>
//Displej
//#include <Adafruit_GFX.h>
#include <UTFTGLUE.h>              
//Dotyková plocha
#include "TouchScreen.h"

//Font
#if !defined(SmallFont)
extern uint8_t SmallFont[];   //.kbv GLUE defines as GFXFont ref
#endif

/* ----------Nastavení ethernetu----------*/
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
unsigned int localPort = 3333;      // local port to listen on
EthernetServer server(localPort);
bool clientConnected = false;
bool serverReady = false;

//Client 
IPAddress serverAddress(192,168,0,180);
EthernetClient client;

/* ----------Nastavení dotykové plochy----------*/
#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin
//Maximální a minimální tlak
#define MINPRESSURE 10
#define MAXPRESSURE 1000
//Údaje pro kalibraci
#define TS_MINX 177
#define TS_MINY 886
#define TS_MAXX 347
#define TS_MAXY 550

/* ----------Barvy----------*/
#define BLACK           0x0000      /*   0,   0,   0 */
//#define NAVY            0x000F      /*   0,   0, 128 */
//#define DARKGREEN       0x03E0      /*   0, 128,   0 */
//#define DARKCYAN        0x03EF      /*   0, 128, 128 */
//#define MAROON          0x7800      /* 128,   0,   0 */
//#define PURPLE          0x780F      /* 128,   0, 128 */
//#define OLIVE           0x7BE0      /* 128, 128,   0 */
#define LIGHTGREY       0xC618      /* 192, 192, 192 */
#define DARKGREY        0x7BEF      /* 128, 128, 128 */
#define BLUE            0x001F      /*   0,   0, 255 */
#define GREEN           0x07E0      /*   0, 255,   0 */
//#define CYAN            0x07FF      /*   0, 255,   0 */
#define RED             0xF800      /* 255,   0,   0 */
#define MAGENTA         0xF81F      /* 255,   0, 255 */
#define YELLOW          0xFFE0      /* 255, 255,   0 */
#define WHITE           0xFFFF      /* 255, 255, 255 */
//#define ORANGE          0xFD20      /* 255, 165,   0 */
//#define GREENYELLOW     0xAFE5      /* 173, 255,  47 */

/* ----------Inicializační funkce----------*/
UTFTGLUE LCD(0x0154,A2,A1,A3,A4,A0);
//Dotyková plocha
TouchScreen Touch(XP, YP, XM, YM, 300);

/* ----------Nastavení displeje----------*/
#define resX 320 //Rozlišení
#define resY 240
#define meshX 11
#define meshY 8

//Pro dotykovou plochu
#define touchScreenOff 1000 //Po jakou dobu budedotyková plocha deaktivována (zabrání multidotykům)
unsigned long long refreshTouchScreen;
int TSx, TSy = 0;
TSPoint touchPoint;
bool touchScreenAct = true; //Aktivuje/deaktivuje dotykovou plochu - zabránění vícedotykům najednou


/* ----------Piškvorky----------*/
byte packetLength = 95;
byte board [95]; //0: nikdo, 1: hráč 1; 2: hráč 2
byte gamePhase = 0; //fáze hry (podle toho se vykreslí obrazovka)
bool gameMyRound = 0; //hraju já nebo portivník
bool screenRefresh = false; //Zda se má obrazovka překreslit
byte crossNum = 5; //Počet koleček vedle sebe pro výhru
byte gameRound = 0;
const byte myNum = 1; //Označuje jaké číslo boardu náleží které desce (server = 1; client = 2)

uint16_t clientColor = RED;
uint16_t serverColor = GREEN;


void drawMainFrame(void); //Vykreslí základní rámeček
void drawMesh (uint16_t); //Vykreslí základní hrací mřížku (argument je barva)
void drawPoints(void); //Vykreslí puntíky podle board
void checkWin(byte); //Zkontroluje zda nějaký hráč nevyhrál (argument je políčko, na které bylo vloženo kolečko)
void chechStatus(byte); //KOntroluje oznamovací kód (umístěn v board[90])
void prepareNewGame(void); //Vytvoří novou hru (zůstává staré spojení - nenavazuje se nové)
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
  drawMainFrame();
  //Sériová linka
  //Ethernet
  Ethernet.begin(mac,serverAddress); //**********
  server.begin();
  delay(50);
  
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
  if (screenRefresh){//Překreslování obrazovky
    switch(gamePhase){
      case 0:
        drawPage(0);
        break;

      case 1:
        drawPage(1);
        break;

     case 2:
      drawPage(2);
      break;
    }
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
     TSx = map(touchPoint.y, 945, 162, 0, 320); //Prohození proměnný...aby sedělo s rozlišením
     TSy = map(touchPoint.x, 131, 910, 0, 240);
     Serial.print("X = "); Serial.print(TSx);
     Serial.print("\tY = "); Serial.print(TSy);
     Serial.print("\tPressure = "); Serial.println(touchPoint.z);
     touchScreenAct = false;
     if (gamePhase == 1){//Místo v hracím poli
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
        board[meshX*row + column] = 1; //Zabrat pole
        drawPoints(); //Překreslí puntíky 
        board[89]++; //Posune herní kolo
        gamePhase = 2; //Nastaví fázi 2 (hraje druhý hráč)
        drawPage(2);
        screenRefresh = true;
        checkWin(meshX*row + column); //Zkontroluje, zda nedošlo k výhře (kontrola se provádí pouze pro vepsaný puntík (sloupec, řádek, křížem)

        //Odesíláni
        while(client.available() > 0){ //Vyprázdnění
        client.read();
      }
        server.write(board, packetLength);
        checkStatus(board[90]); //Zkontroluje kód (podle hodnoty výhra,prohra, pokračování)
      }
        
     }
     
     
   }
}
  
  

   //SERVER
   if(!clientConnected){ //Čekání na clienta
    //Serial.println("pokousim se spojit");
    client = server.available();
    if(client){
      Serial.println("Client pripojen");
      client.flush();
      while(client.available() > 0){ //Vyprázdnění
        client.read();
      }
      clientConnected = true;
      screenRefresh = true;
      gamePhase = 1;
      drawMainFrame();
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
    checkStatus(board[90]); //Kontrola kódu
    screenRefresh = true;
    gamePhase = 1;
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
void drawMainFrame(){
  LCD.setColor(0, 0, 255); //Nastavení barvy
  LCD.drawRect(0, 0, 320, 240); //Vykreslení čtverce (souřadnice levý horní a pravý dolní roh)
  LCD.drawRect(1, 1, 319, 239);
  LCD.setColor(0, 0, 0);
  LCD.fillRect(2, 2, 318, 238);
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
  *    - 0: čekání na clienta
  *    - 1: hraji
  *    - 2: hraje druhý hráč
  */

  void drawPage (byte id){
  switch(id){
  
    case 0:
      LCD.setTextColor(YELLOW, BLACK);
      LCD.setTextSize(3);
      LCD.setCursor(50, 10);
      LCD.println("Piskvorky");
      LCD.setCursor(50, 70);
      LCD.setTextSize(2);
      LCD.println("Cekam na clienta");
      screenRefresh = false;
      break;

    case 1:
      drawMesh(BLUE);
      drawPoints();
      screenRefresh = false;
      break;

    case 2:
      drawMesh(LIGHTGREY);
      screenRefresh = false;
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

  for (int i = 0; i < 88; i++){
        if(board [i] == 1){
          row = i/11;
          column = i - row*11;
          LCD.setColor(serverColor);
          LCD.fillCircle(column * resX/meshX + (resX/meshX)/2, row * resY/meshY + (resY/meshY)/2, 10);
          
        }
        else if (board [i] == 2){
          row = i/11;
          column = i - row*11;
          LCD.setColor(clientColor);
          LCD.fillCircle(column * resX/meshX + (resX/meshX)/2, row * resY/meshY + (resY/meshY)/2, 10);
          
        }
      }
}


//------------------------------------------------------------------------------------------------------

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
        prepareNewGame();
      break;

    case 201: //Vyhrál server
        LCD.setColor(BLACK);
        LCD.fillRect(0,0, 320, 50);
        LCD.setTextColor(GREEN, BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(20, 20);
        LCD.println("Vitez");
        delay(10000);
        prepareNewGame();
      break;

   case 202: //Vyhrál client
        LCD.setColor(BLACK);
        LCD.fillRect(0,0, 320, 50);
        LCD.setTextColor(RED, BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(20, 20);
        LCD.println("Porazeny");
        delay(10000);
        prepareNewGame();
      break;

   default:
      break;
  }
}


//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
//>>>>> Připraví novou hru <<<<<
 /*   Princip:   
  *    - 
  *    
  */
void prepareNewGame(){
   //Nulování herní desky
  Serial.println("Nulovani pole pro herni desku");
  for (int i = 0; i < packetLength; i++){
    board[i] = 0;
  }
  board[90] = 100; //vše OK
  drawMainFrame();
  gamePhase = 1; //Začínám - fáze 1
  drawPage(1);
  screenRefresh = true;
}


//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
//>>>>> Kontrola výhry/stavu <<<<<
 /*   Princip:   
  *     - pro zadaný puntík v poli
  *    
  */
void checkWin(byte pole){
  byte row = 0;
  byte column = 0;
  byte count = 0; //počet puntíků za sebou
   if (board[89] >= 88) {//Pole je plná => remíza
      board[90] = 200;
   }

   row = pole/11;
   column = pole%11;

   for(int i = 0; i < meshX; i++){ //v řádku
      if(board[11*row + i ] == myNum){
        count++;
      }
      else{
        count = 0;
      }
      if (count >= crossNum){
        board[90] = 201;
        break;
      }
    
   }
  count = 0;
   for(int i = 0; i < meshY; i++){ //v sloupec
      if(board[column + i*11 ] == myNum){
        count++;
      }
      else{
        count = 0;
      }
      if (count >= crossNum){
        board[90] = 201;
        break;
      }
    
   }
   count = 0;
    //Do kříže
   byte index = 0;
   index = pole % 12;
   while (index < meshX*meshY){
      if(board[index] == myNum){
        count++;
      }
      else{
        count = 0;
      }
      index += 12;
      if (count >= crossNum){
        board[90] = 201;
        break;
      }
   }

   count = 0;
   index = pole % 10;
   while (index < meshX*meshY){
      if(board[index] == myNum){
        count++;
      }
      else{
        count = 0;
      }
      index += 10;
      if (count >= crossNum){
        board[90] = 201;
        break;
      }
   }
}


//------------------------------------------------------------------------------------------------------
