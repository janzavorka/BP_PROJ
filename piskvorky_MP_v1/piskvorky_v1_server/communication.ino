//>>>>> Odešle herní desku danému hráči <<<<<
 /*   Princip:
  *    - pomocí server.write odešle celou herní desku
  */
void sendBoard(byte code, byte playerNum){
  const byte boardPart = 8; //Délka části pole board, která bude odeslána
  const byte subBoard_length = 11; //Délka dat, která se bude odesílat (vč. pořadí a kontrolního součtu
  byte subBoard[subBoard_length]; //POle k odeslání
  int checkSum = 0; //Kontrolní součet
  board[gb_code] = code;
  Serial.print("Odesilam data hraci ");
  Serial.println(playerNum);
  
  for(byte i = 0; i < packetLength/boardPart; i++){
    checkSum = 0;
    for(byte k = 0; k < boardPart; k++){
          subBoard[k] = board[k + boardPart*i];
          checkSum += subBoard[k];
          //Serial.println(board[k + boardPart*i]);
    }
    subBoard[8] = i; //POřadové číslo packetu
    checkSum += subBoard[8];
    //Vložení kontrolního součtu
    subBoard[9] = byte(checkSum & 0xFF);
    subBoard[10] = byte(checkSum >> 8);

    //Odeslání
    clients[playerNum -1].write(subBoard, subBoard_length);
    delay(10);
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Odešle herní desku danému všem <<<<<
 /*   Princip:
  *    - pomocí server.write odešle celou herní desku
  */
void sendBoard(byte code){
  for(byte i = 0; i < maxPlayers; i++){
    if(clients[i] && clients[i].connected()){
      sendBoard(code, i+1);
    }
  }
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
  *    -
  */
void checkIncommingData(){
  for(int i = 0; i < maxPlayers; i++){
    if(clients[i].available()){
      recieveData(i);
    }
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Přijme data od hráče <<<<<
 /*   Princip:
  *    - Přijme data od clienta
  *    - Pokud byly přijmutyoba packety, budou vyhodnoceny
  *    - Před vyhodnocením se provede smazání dalích dat, které client souběžně poslal (client může posílat data max 1x za sekundu)
  *       tímto se zabrání dalším chybám pokud by se do komunikace připletla nějká další data (jinak by vždy příjem končil neshodou obou packetů)
  */
void recieveData(byte index){
   while(clients[index].available() > 1){
    Serial.print("Prijem dat od uzivatele "); Serial.print(index); Serial.print("     ");
      for(byte i = 0; i < 2; i++){
        if(clientsData[index][i][2] == 0){ //Pokud tato data nebyla vyplněna
          clientsData[index][i][0] = clients[i].read();
          Serial.print(clientsData[index][i][0]); Serial.print("; ");
          delay(5);
          clientsData[index][i][1] = clients[i].read();
          Serial.print(clientsData[index][i][1]);
          clientsData[index][i][2] = 1;
        }
      }
   }

   if(clientsData[index][1][2] == 1){ //Byly vyplněny oba packety (pro porovnání)
      while(clients[index].available()){ //Zahození všech ostatních dat
        char bin = clients[index].read();
      }
      processClientData(index);
   }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Smaže přijatá data daného clienta <<<<<
 /*   Princip:
  *    -
  */
void resetClientData(byte index){
  for(byte i = 0; i < 2; i++){
     clientsData [index][i][0] = 0;
     clientsData [index][i][1] = 0;
     clientsData [index][i][2] = 0;
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Smaže přijatá data daného clienta <<<<<
 /*   Princip:
  *    -
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
    else if(clientsData[index][0][0] == 20){
      sendBoard(board[gb_code], index+1); //Znovu se pošle hráči celé pole
    }
    resetClientData(index);
  }
  else{
    Serial.print("Od hrace ");
    Serial.print(index+1);
    Serial.print(" byla prijata chybna data, zahazuji");
    resetClientData(index);
  }
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
