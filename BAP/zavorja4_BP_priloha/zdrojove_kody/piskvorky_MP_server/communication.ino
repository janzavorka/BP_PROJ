/*>>>>>>> Piškvorky s arduinem po LAN <<<<<<<
*  !!! Součást programu pro server, samostatně nefunkční !!!
*
* - Autor: Jan Závorka
* - Email: zavorja4@fel.cvut.cz
* - Domovská stránka projektu: https://github.com/janzavorka/BP_PROJ
* - Seznam souborů: piskvorky_MP_server.ino; boardControl.ino; communication.ino; gameControl.ino; indicatioLED.ino; SerialControl.ino
*
* --- Popis:
* - Funkce zodpovědné za řízení komunikace po síti
* - Spojení, distribuci a vyhodnocování dat
*/

//>>>>> Odešle herní desku všem beze změny kódu <<<<<
 /*   Princip:
  *    - odešle všem hráčům herní pole
  *    - void fce(void): vhodné pro použití s timery
  */
void sendBoardR(){
  sendBoard(board[gb_code]);
}
//------------------------------------------------------------------------------------------------------
//>>>>> Odešle herní desku všem <<<<<
 /*   Princip:
  *    - odešle herní desku (pole board) všem připojeným hráčům
  *    - podle argumentu změní řídicí kód
  */
void sendBoard(byte code){
  for(byte i = 0; i < maxPlayers; i++){
    if(clients[i] && clients[i].connected()){
      sendBoard(code, i+1);
    }
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Odešle herní desku danému hráči <<<<<
 /*   Princip:
  *    - rozdělí herní desku (board) na jednotlivé části, vybaví je pořadovým číslem a kontrolním součtem a odešle je danému hráči
  *    - řídicího kód v argumentu
  */
void sendBoard(byte code, byte playerNum){
  const byte subBoard_length =  boardPart + 3; //Délka části pole board, která bude odeslána; //Délka dat, která se bude odesílat (vč. pořadí a kontrolního součtu
  byte subBoard[subBoard_length]; //POle k odeslání
  int checkSum = 0; //Kontrolní součet
  board[gb_code] = code;

  DEBUG_PRINT("Odesilam data hraci ");
  DEBUG_PRINTLN(playerNum);

  for(byte i = 0; i < packetLength/boardPart; i++){
    checkSum = 0; //Resetování kontrolního součtu

    DEBUG0STC_PRINT("\nSubpacket cislo ");
    DEBUG0STC_PRINT(i);
    DEBUG0STC_PRINT(" hraci ");
    DEBUG0STC_PRINT(playerNum);
    DEBUG0STC_PRINT("  ");

    for(byte k = 0; k < boardPart; k++){
          subBoard[k] = board[k + boardPart*i];
          checkSum += subBoard[k];
          DEBUG0STC_PRINT(board[k + boardPart*i]);
    }
    subBoard[8] = i; //Pořadové číslo packetu
    checkSum += subBoard[8];
    //Vložení kontrolního součtu
    subBoard[9] = byte(checkSum & 0xFF);
    subBoard[10] = byte(checkSum >> 8);

    //Odeslání
    clients[playerNum -1].write(subBoard, subBoard_length);
    delay(1);
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Odešle část herního pole <<<<<
 /*   Princip:
  *    - odešle vyžádanou část (argument) pole danému hráči
  *    - voláno pro stavy, kdy si client vyžádá určitou část znova (nebyla správně přijata)
  */
void sendBoardPart(byte part, byte playerNum){
  const byte subBoard_length =  boardPart + 3; //Délka části pole board, která bude odeslána; //Délka dat, která se bude odesílat (vč. pořadí a kontrolního součtu
  byte subBoard[subBoard_length]; //Pole k odeslání
  int checkSum = 0; //Kontrolní součet

  checkSum = 0;
  for(byte k = 0; k < boardPart; k++){
        subBoard[k] = board[k + boardPart*part];
        checkSum += subBoard[k];
  }
  subBoard[8] = part; //Pořadové číslo packetu
  checkSum += subBoard[8];
  //Vložení kontrolního součtu
  subBoard[9] = byte(checkSum & 0xFF);
  subBoard[10] = byte(checkSum >> 8);

  //Odeslání
  clients[playerNum -1].write(subBoard, subBoard_length);
  delay(5);
}
//------------------------------------------------------------------------------------------------------
//>>>>> Synchronizuj IP hráčů s herní deskou <<<<<
 /*   Princip:
  *    - Synchronizuje IP adresy v boardu a s IP adresy v seznamu clientů (clients[])
  */
void syncBoardIPs(){
  for(byte i = 0; i < maxPlayers; i++){
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
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Zkontroluje, zda nejsou k dispozici data od hráče <<<<<
 /*   Princip:
  *    - kontroluje, zda některý z hráčů něco neodeslal
  *    - pokud odeslal zavolá fci k přijetí dat
  */
void checkIncommingData(){
  for(int i = 0; i < maxPlayers; i++){
    if(clients[i].available() > 1){
      DEBUG_PRINT("Prijata data od hrace ");
      DEBUG_PRINTLN(i+1);
      recieveData(i);
    }
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Přijme data od hráče <<<<<
 /*   Princip:
  *    - Přijme data od clienta
  *    - Pokud byly přijmuty oba packety, budou vyhodnoceny (porovnány)
  *    - Před vyhodnocením se provede smazání dalších dat, které client souběžně poslal (client může posílat data max 1x za sekundu)
  *       tímto se zabrání dalším chybám pokud by se do komunikace připletla nějká další data (jinak by vždy příjem končil neshodou obou packetů)
  */
void recieveData(byte index){
      for(byte i = 0; i < 2; i++){
        if(clientsData[index][i][2] == 0 && clients[index].available() > 1){ //Pokud tato data nebyla vyplněna a nějaká jsou k dispozici - přečti je
          clientsData[index][i][0] = clients[index].read();
          delay(2);
          clientsData[index][i][1] = clients[index].read();
          clientsData[index][i][2] = 1; //Vyúlnění, že data byla přijata

          DEBUG0CTS_PRINT("Prijem dat od uzivatele ");
          DEBUG0CTS_PRINT(index);
          DEBUG0CTS_PRINT("     ");
          DEBUG0CTS_PRINT(clientsData[index][i][0]);
          DEBUG0CTS_PRINT("; ");
          DEBUG0CTS_PRINTLN(clientsData[index][i][1]);
        }
      }

   if(clientsData[index][1][2] == 1){ //Byly vyplněny oba packety (pro porovnání)
     delay(8);
     DEBUG0CTS_PRINT("Zahazuji");
      while(clients[index].available() > 0){ //Zahození všech ostatních dat
        char bin = clients[index].read();
        DEBUG0CTS_PRINT(bin);
      }
      DEBUG0CTS_PRINT("\n");
      processClientData(index);
   }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Smaže přijatá data daného clienta <<<<<
 /*   Princip:
  *    - Vyplní pole pro uchování dat od clienta nulami
  */
void resetClientData(byte index){
  for(byte i = 0; i < 2; i++){
     clientsData [index][i][0] = 0;
     clientsData [index][i][1] = 0;
     clientsData [index][i][2] = 0;
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Vyhodnotí přijatá data <<<<<
 /*   Princip:
  *    - Vyhodnotí data přijatá od clienta
  *    - Dojde k porovnání, zda oba přijaté packety sedí (jsou stejné)
  */
void processClientData(byte index){
  if(clientsData[index][0][0] == clientsData[index][1][0] && clientsData[index][0][1] == clientsData[index][1][1]){
    if(clientsData[index][0][0] == 10){
      //Přijatá data o tahu
      if(fillPlayerToken(clientsData[index][0][1], index+1)){ //Vyplnění pole proběhlo v pořádku
          checkGame(clientsData[index][0][1], index+1);
      }
      else{ //Pokud se to nepovedlo, pošle se deska znovu
          sendBoard(1);
      }
    }
    else if(clientsData[index][0][0] == 20){ //Pokud byla vyžádána nějaká část packetu znovu
      sendBoardPart(clientsData[index][0][1], index+1); //Odešle se část pole znovu
    }
    resetClientData(index);
  }
  else{
    DEBUG0CTS_PRINT("Od hrace ");
    DEBUG0CTS_PRINT(index+1);
    DEBUG0CTS_PRINTLN(" byla prijata chybna data, zahazuji");
    resetClientData(index);
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Zrealizuje odpojení daného hráče <<<<<
/*   Princip:
 *    - Koretkně odpojí daného clienta
 *    - Vynuluje jeho adresu v herním poli
 *    - Odpojení signalizuje LED a prostřednictví speciálního kódu informuje ostatní hráče
 */
void disconnectPlayer(byte player){
    byte index = player -1;
    Serial.print("Odpojuji hrace cislo: ");
    Serial.println(player);
    clients[index].stop();
    board[IPaddr[index]] = 0; //Přepsání IP adresy v herní poli na nuly
    board[IPaddr[index]+1] = 0;
    board[IPaddr[index]+2] = 0;
    board[IPaddr[index]+3] = 0;
    if(player == board[gb_actPlayer]){ //Pokud byl odpojený hráč na řadě, přepni na dalšího hráče
      shiftPlayer();
    }
    signalLED.changeBlinkColor(LEDcol_orange, 3); //Indikace LED
    sendErrMessage(200+player); //Ostatním hráčům se odešle informace
}
//------------------------------------------------------------------------------------------------------
//>>>>> Odešle chybovou právu všem hráčům, po určité době obnoví předchozí stav hry <<<<<
/*   Princip:
 *    - Zapamatuje si původní řídicí kód a do pole vyplní chybový
 *    - Po určité době dojde vrátí původní řídicí kód do pole a odešle data
 */
void sendErrMessage(byte message){
    byte LastCode = board[gb_code]; //Zálohuje původní hodnotu a pak jí vrátí
    sendBoard(message);
    DEBUG0STC_PRINT("Odeslan chybovy kod: ");
    DEBUG0STC_PRINTLN(message);
    board[gb_code] = LastCode;
    delay(50);
    timer.setTimeout(clientErrMessageLast, sendBoardR);
}
//------------------------------------------------------------------------------------------------------
//>>>>> Identifikuje použité ethernet controler <<<<<
 /*   Princip:
 *      - Identifikuje použitý HW controller
  *     -  0=noHW, 1=W5100, 2=W5200, 3=W5500
  */
byte getHWcontroller(){
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    return 0;
  }
  else if (Ethernet.hardwareStatus() == EthernetW5100) {
    return 1;
  }
  else if (Ethernet.hardwareStatus() == EthernetW5200) {
    return 2;
  }
  else if (Ethernet.hardwareStatus() == EthernetW5500) {
    return 3;
  }
}
//------------------------------------------------------------------------------------------------------
