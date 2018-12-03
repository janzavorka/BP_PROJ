/*>>>>>>> Piškvorky s arduino Ethernet <<<<<<<
 * - max 5 hráčů
 * - 1x arduino jako server, max 5x arduino jako client
 * 
 */

#include <Ethernet.h>

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

//Herní server
IPAddress serverAddress(10,0,0,8);

//Rozměry sítě
#define meshX 11
#define meshY 8
/* ----------HRA----------*/
byte serverPhase = 0; //označuje fázi hry= 0: čekání na připojení klientů, 1: kontrola klientů, správné nastavení hry, 2: fáze hry
byte lastPlayer = 0; //Označuje posledního hráče, který hrál

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
byte packetLength = 126; 
byte board [126]; //0: nikdo, 1: hráč 1; 2: hráč 2
/* >>>>> Rozložení herního packetu <<<<<
 *  0-89:   Obsazení herních polí (standadně 0, server doplňuje čísla)
 *  90:     Hlášení prostřednictvím kódu 
 *            0:    vše OK, hraje se, překresli obrazovku
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
/* ----------PROTOTYPY----------*/
void cleanBoard(void); //Vyplní herní desku nulami
void syncBoardIPs(void); //Synchronizuje IP adresy v boardu a s IP adresy v seznamu clientů (clients[])
void sendBoard(void); //Odešle herní desku
void setBoard(void); //Připraví herní desku
byte getNextPlayer(byte); //Vrátí číslo dalšího hráče (argument fce je číslo předchozího hráče)
/*
 * >>>>>>>>>> SETUP <<<<<<<<<<
 */
void setup() { 
  //Sériová linka
  Serial.begin(9600);
  randomSeed(analogRead(0));
  delay(10);
  Serial.println("Startuji piskvorkovy server");
  //Ethernet
  Ethernet.begin(mac, serverAddress);
  Serial.print("Moje IP adresa: ");
  Serial.println(Ethernet.localIP());
  delay(200);
  Serial.println("Spoustim server");
  server.begin();
  delay(200);
  setBoard();
  
}

/*
 * >>>>>>>>>> LOOP <<<<<<<<<<
 */
void loop() {
    if(serverPhase == 0){
      EthernetClient newClient = server.accept();
      delay(10);
      if ((newClient)) {
        bool clientOK = true;
        for (byte i = 0; i < maxPlayers; i++) {
          if (!clients[i]) {
              for(byte j = 0; j < maxPlayers; j++){ //Ověření duplicit
                if(clients[j].remoteIP() == newClient.remoteIP()){
                  clientOK = false;
                  sendBoard();
                  break;
                }
              }
              if(clientOK){
                Serial.print("New client number: ");
                Serial.println(i);
                Serial.print("IP: ");
                Serial.println(newClient.remoteIP());
                clients[i] = newClient;
                while(clients[i].available()){ //Vyprázdění
                  Serial.print(clients[i].read());
                }
                Serial.println();
                IPAddress cliIP = clients[i].remoteIP();
                board[IPaddr[i]] = cliIP[0];
                board[IPaddr[i]+1] = cliIP[1];
                board[IPaddr[i]+2] = cliIP[2];
                board[IPaddr[i]+3] = cliIP[3];
                sendBoard();
              }
              break;
            }
          }
        }
    }
    else if(serverPhase == 1){
      Serial.println("Pripravuji hru");
      setBoard();
      byte firstPlayer = random(1, 5);
      delay(50);
      Serial.print("Prvni hraje ");  Serial.println(firstPlayer);
      while(!clients[firstPlayer-1]){
        firstPlayer++;
        if(firstPlayer > maxPlayers){
          firstPlayer = 1;
        }
      }
      board[gb_actPlayer] = firstPlayer;
      serverPhase = 2;
      board[gb_code] = 0;
      sendBoard();
    }
    else if(serverPhase == 2){
      if(clients[board[gb_actPlayer] - 1].available() > 0){
        Serial.print("Hraje hrac: ");
        Serial.println(board[gb_actPlayer]);
        byte place = 0;
        place = clients[board[gb_actPlayer] - 1].read();
        if(place >=0 && place < meshX*meshY){
          board[place] = board[gb_actPlayer];
          board[gb_round]++; //Zvýšení počtu odehraných kol
          board[gb_actPlayer] = getNextPlayerNumber(board[gb_actPlayer]);
        }
        board[gb_code] = 0;
        sendBoard();
      }
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
    }
  }
  
  //Zpracování příkazů
  if(Serial.available()){
    char com = 0;
    com = Serial.read();
    if(com == 's'){
      Serial.println("Zacatek kola");
      serverPhase = 1;
    }
    else if (com == 'r'){
      Serial.println("Ukoncuji hru");
      board[gb_code] = 3;
      delay(5);
      sendBoard();
      serverPhase = 0;
    }
    else{
      Serial.println(com);
    }
  }
  delay(100); 
}


/*
 * >>>>>>>>>> FUNKCE <<<<<<<<<<
 */
 //------------------------------------------------------------------------------------------------------
//>>>>> vynuluje herní desku <<<<<
 /*   Princip:   
  *    - vyplní herná desku nulami
  */
void cleanBoard(){
   //Nulování herní desky
  Serial.println("Nulovani pole pro herni desku");
  for (int i = 0; i < meshX*meshY; i++){
    board[i] = 0;
  }
  board[gb_round] = 0;
  board[gb_actPlayer] = 0;
}
//------------------------------------------------------------------------------------------------------
//>>>>> Synchronizuj IP hráčů <<<<<
 /*   Princip:   
  *    - Synchronizuje IP adresy v boardu a s IP adresy v seznamu clientů (clients[])
  */
void syncBoardIPs(){
  /*for(byte i = 0; i < maxPlayers; i++){
    if(clients[i]){
      IPAddress cliIP = clients[i].remoteIP();
      board[IPaddr[i]] = cliIP[0];
      board[IPaddr[i]+1] = cliIP[1];
      board[IPaddr[i]+2] = cliIP[2];
      board[IPaddr[i]+3] = cliIP[3];
    }
    else{
      board[IPaddr[i]] = 0;
      board[IPaddr[i]+1] = 0;
      board[IPaddr[i]+2] = 0;
      board[IPaddr[i]+3] = 0;
    }
  }*/
}
//------------------------------------------------------------------------------------------------------
//>>>>> Odešle herní desku <<<<<
 /*   Princip:   
  *    - pomocí server.write odešle celou herní desku
  */
void sendBoard(){
  for(byte i = 0; i < maxPlayers; i++){
    if(clients[i]){
      clients[i].write(board, packetLength);
      delay(50);
    }
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Připraví a nastaví herní desku <<<<<
 /*   Princip:   
  *    - 
  */
void setBoard(){
  cleanBoard();
  board[gb_code] = 3;
  board[gb_actPlayer] = 0;
  //Barvy
  board[gb_PC1] = byte(RED & 0xFF);
  board[gb_PC1+1] = byte(RED >> 8);
  //
  board[gb_PC2] = byte(ORANGE & 0xFF);
  board[gb_PC2+1] = byte(ORANGE >> 8);
  //
  board[gb_PC3] = CYAN & 0xFF;
  board[gb_PC3+1] = CYAN >> 8;
  //
  board[gb_PC4] = PURPLE & 0xFF;
  board[gb_PC5+1] = PURPLE >> 8;
  //
  board[gb_PC5] = OLIVE & 0xFF;
  board[gb_PC5+1] = OLIVE >> 8;
  
  
}
//------------------------------------------------------------------------------------------------------
//>>>>> Vrátí šíslo dalšího hráče <<<<<
 /*   Princip:   
  *    - zkontroluje pole clientts, aby se jednalo o hráče, který je připojen
  *    - hrac1 je číslo předchozího (aktuálně hrajícího hráče)
  */
byte getNextPlayerNumber(byte player1){
  player1++;
  do{
    if(player1 > maxPlayers){
      player1 = 1;
    }
    if(player1 < 1){
      player1 = maxPlayers;
    }
  }while(!clients[player1-1]);
  return player1;
}
//------------------------------------------------------------------------------------------------------

