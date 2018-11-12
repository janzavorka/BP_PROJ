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
//IP adresa z DHCP serveru
EthernetServer server(localPort);
bool clientConnected = false;
bool serverReady = false;
// buffer pro příchozí data
char packetBuffer[100];  

//Client 
IPAddress serverAddress(192,168,0,180);

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
const int korekceX = -150; 
const int korekceY = 10;
const int maxY = 920;
bool touchScreenAct = true; //Aktivuje/deaktivuje dotykovou plochu - zabránění vícedotykům najednou
//Tlačítka
byte button_index = 0;

/* ----------Piškvorky----------*/
byte board [meshX*meshY]; //0: nikdo, 1: hráč 1; 2: hráč 2
byte gameType = 0; //0 pro server, 1 pro client
byte gamePhase = 0; //0 Menu (serve/client); 1: nastavení serveru; 2: zadání IP; 3: čekání na clienta; 5: výběr barvy; 6: herní plán; 5: hra
bool screenRefresh = false;
byte crossNum = 4; //Počet koleček vedle sebe pro výhru

uint16_t clientColor = RED;
uint16_t serverColor = GREEN;

/* ----------Tlačítka----------*/
class buttonMy{
  private:
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    byte id = 0;
  public:
  buttonMy();
  buttonMy(int corX1, int corY1, int corX2, int corY2, byte idecko, byte mode);
  /*~buttonMy(){
    //buttonMy_index--;
    Serial.println("Tlacitko zniceno");
  }*/

  bool isTouched (int touchX, int touchY);

  byte getID (void);
};

buttonMy::buttonMy(int corX1, int corY1, int corX2, int corY2, byte idecko, byte mode){
    //Serial.println("Vytvarim tlacitko");
    x1 = corX1;
    y1 = corY1;
    x2 = corX2;
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
        //break;
    }
    button_index++;
}

buttonMy::buttonMy(){
  
}


bool buttonMy::isTouched (int touchX, int touchY){
    /*Serial.println("Kontroluji stisk");
    Serial.print("Souradnice x: ");
    Serial.print(touchX);
    Serial.print("; y: ");
    Serial.print(touchY);
    Serial.print("pro tlacitko x: ");
    Serial.print(x1);
    Serial.print("; Y: ");
    Serial.println(x2);*/
    if (touchX >= x1 && touchX <= x2 && touchY >=y1 && touchY <= y2){
      return true;
    }
    else{
      return false;
    }
}

byte buttonMy::getID (void){
    return id;
}

/* ----------Prototypy----------*/
void drawMainFrame(void);
void drawMesh (void); //Vykreslí základní hrací mřížku
void drawPage (byte); //Vypíše danou stránku (na základě ID)
void buttonCom (byte); //Při stisku tlačítka vykoná danou činnost, argument je ID tlačítka

//buttonMy *but = (buttonMy*)malloc((100)*sizeof(buttonMy)); //Pole pro tlačítka
buttonMy but[14];

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
  LCD.crlScr();
  /*drawMainFrame();
  LCD.setTextColor(YELLOW, BLACK);
  LCD.setTextSize(3);
  LCD.setCursor(130, 10);
  LCD.println("Piskvorky");*/
  //Sériová linka
  //Ethernet
  Ethernet.begin(mac, serverAddress); //**********
  delay(50);
  
  //Nulování herní desky
  Serial.println("Nulovani pole pro herni desku");
  for (int i = 0; i < meshX*meshY; i++){
    board[i] = 0;
  }
  screenRefresh = true;
  delay(100);
}

/*
 * >>>>>>>>>> LOOP <<<<<<<<<<
 */
void loop() {
  if (screenRefresh){
    switch (gamePhase){
      case 0:
        drawPage(0);
        break;
      case 1:
        drawPage(1);
        break;
      case 2:
        drawPage(2);
        break;
      case 3:
        drawPage(3);
        break;
      case 4:
        drawPage(4);
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
     for(int i = 0; i < button_index; i++){
      if(but[i].isTouched(TSx,TSy)){
        Serial.print("Tlacitko: ");
        Serial.println(but[i].getID());
        buttonCom(but[i].getID());
        touchScreenAct = false;
        break;
      }
     }
   }
}
  
  

   //SERVER
   if(serverReady && !clientConnected){
    EthernetClient client = server.available();
    if(client){
      clientConnected = true;
      gamePhase = 4;
      screenRefresh = true;
    }
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
void drawMesh(){
  LCD.setColor(0, 0, 255); //Nastavení barvy
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
  *    - 0: základní nastavení: Client nebo server
  *    - velikost 11x8 polí
  */
void drawPage(byte id){
  switch(id){
    case 0:
      drawMainFrame();
      LCD.setTextColor(YELLOW, BLACK);
      LCD.setTextSize(3);
      LCD.setCursor(50, 10);
      LCD.println("Hlavni menu");
      LCD.setTextSize(2);
      LCD.setCursor(50, 40);
      LCD.println("Vyberte typ hry: ");

      but [button_index]  = buttonMy(50,90,270,150, 0,1);
      LCD.setTextColor(BLACK, LIGHTGREY);
      LCD.setTextSize(4);
      LCD.setCursor(80,100);
      LCD.println("SERVER");
      
      but [button_index]  = buttonMy(50,170,270,230, 01,1);
      LCD.setTextColor(BLACK, LIGHTGREY);
      LCD.setTextSize(4);
      LCD.setCursor(80,180);
      LCD.println("CLIENT");
      
      screenRefresh = false;
      break;

    case 1:
      drawMainFrame();
      LCD.setTextColor(YELLOW, BLACK);
      LCD.setTextSize(3);
      LCD.setCursor(10, 10);
      LCD.println("Cekam na clienta");
      LCD.setTextSize(2);
      LCD.setCursor(10,40);
      LCD.println("Server IP: ");
      LCD.setCursor(130,40);
      LCD.println(Ethernet.localIP());
      
      //Nastartuj server
      server.begin();
      serverReady = true;
      screenRefresh = false;
      break;
  
    

    case 5: //Herní plán
      drawMainFrame();
      drawMesh();
      gamePhase = 6;
      screenRefresh = false;
      break;

   case 2:
    byte ipIndex =  12;
    char ipPrint[10] ;
    drawMainFrame();
    LCD.setTextColor(YELLOW, BLACK);
    LCD.setTextSize(3);
    LCD.setCursor(10, 10);
    LCD.println("Pripojeni");
    LCD.setTextSize(2);
    LCD.setCursor(10,40);
    LCD.println("Zadejte IP adresu serveru");

    LCD.setTextSize(2);
    LCD.setCursor(18,75);
    LCD.println("8");
    LCD.setColor(YELLOW);

    for (int i = 0; i < 4; i++){
      for (int j = 0; j < 3; j++){
        LCD.fillRect(15 + j*20 + i*80 ,92, 30+ j*20 + i*80,93);
      }
      if (i !=3){
        LCD.fillCircle(81 + i*80, 91 ,3);
      }
    }

    for (int i = 0; i < 3; i++){
      for (int j = 0; j < 3; j++){
        but [button_index]  = buttonMy(15 + j*35 ,100+i*35,45+j*35,130 + i*35, 20+j+7-3*i,1); 
        LCD.setTextColor(BLACK, LIGHTGREY);
        LCD.setTextSize(3);
        LCD.setCursor(22+ j*35, 105 +i*35);
        LCD.println(j+7-3*i);
      }
    }
    but [button_index] = buttonMy(15, 205, 80, 235, 20, 1);
    LCD.setTextColor(BLACK, LIGHTGREY);
    LCD.setTextSize(3);
    LCD.setCursor(45, 210);
    LCD.println("0");
    but [button_index] = buttonMy(85, 205, 115, 235, 30, 1); 
    LCD.setTextColor(BLACK, LIGHTGREY);
    LCD.setTextSize(1);
    LCD.setCursor(92, 220);
    LCD.println("DEL");
    but [button_index] = buttonMy(120, 100, 150, 165, 31, 1);
    LCD.setTextColor(BLACK, LIGHTGREY);
    LCD.setTextSize(1);
    LCD.setCursor(127, 140);
    LCD.println("BCK");
    but [button_index] = buttonMy(120, 170, 150, 235, 32, 1);
    LCD.setTextColor(BLACK, LIGHTGREY);
    LCD.setTextSize(1);
    LCD.setCursor(127, 220);
    LCD.println("OK");
    
    byte shift = 0;
    for (int i=0; i < 12 ; i++){
      LCD.setTextSize(2);
      LCD.setTextColor(YELLOW, BLACK);
      LCD.setCursor(18 + 20*i + shift,75);
      LCD.println("8");
      //ipPrint[i]
      if (i ==2 || i ==5 || i ==8){
        shift += 20;
      }
    }
    screenRefresh = true;
    gamePhase = 3;
    break;
  case 3:
   /* byte shift = 0;
    for (int i=0; i < 12 ; i++){
      LCD.setTextColor(YELLOW, BLACK);
      LCD.setCursor(18 + 7*i + shift,75);
      LCD.println("8");
      //ipPrint[i]
      if (i % 3 ==0){
        shift += 15;
      }
    }
    screenRefresh = false;*/
    break;
  }
}

//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
//>>>>> Vykoná činnost tlačítka na základě jeho id <<<<<
 /*   Princip:   
  *    - 00: nastaví gameMode na server a vykreslí další stranu 
  *    - 01: nastaví gameMode na client na vykreslí další stranu
  *    
  */
void buttonCom(byte id){
  switch(id){
    case 0: //
    screenRefresh = true;
    button_index = 0;
    gamePhase = 1;
    gameType = 0;
      break;
      
    case 01:
    screenRefresh = true;
    button_index = 0;
    gamePhase = 2;
    gameType = 1;
      break;


    case 20 - 29:
      screenRefresh = true;
      break;
    case 30:
      screenRefresh = true;
      break;

    case 999:
      break;
  }
}

//------------------------------------------------------------------------------------------------------
