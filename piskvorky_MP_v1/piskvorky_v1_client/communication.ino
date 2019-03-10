//>>>>> Připojení k serveru <<<<<
 /*   Princip:
  *    - nekonečná smyčka snaží cí se připojit k serveru
  *    - pokud je připojení úspěšné, změní se fáze hry
  *    - v případě úspěšného spoojení vrátí true
  *    - hodnoty vrácené serverem: 0 = spojení odmítnuto (třeba z důvodu běžící hry); 1-5 = přiřazené číslo hráče
  */
bool connectToServer(){
  byte server_code = 0;
  if(!client.connected()){
    //Serial.println("Pokus o spojeni");
    client.connect(serverAddress, 3333);
    delay(20);
    if (client.connected()){
      //Serial.println("Pripojuji");
      client.write(100); //Aby připojení server správně zaznamenal (kód 250: chci se připojit)
      delay(200);
      return true;
    }
    else{
        //Serial.println("NEPRIPOJENO");
        return false;
    }
  }
}

//------------------------------------------------------------------------------------------------------
//>>>>> Pokud je připojeno k serveru, přijímá herní desku <<<<<
 /*   Princip:
  *    - Zkusí jestli server něco odeslal, přijme celý board
  */
void recieveBoard (){
  byte index = 0;
  byte subBoard [11];
  int checkSum = 0;
  if (client.connected() && client.available() > 10){ //client.available();
    //Serial.println("prijem");
    while(index < 11){
      subBoard[index] = client.read();
      if(index <= 8){
        checkSum += subBoard[index];
      }
      index++;
      delay(3);
    }
    if((int(subBoard[9]) | int(subBoard[10]) << 8) == checkSum){ //Pokud sedí kontrolní součet = data byla přijata správně
      boardAck[subBoard[8]] = true; //Potvrzení přijetí, na pozici 8 v subBoard je pořadové číslo packetu
      for(byte i = 0; i < 8; i++){ //Zápis dat do herní desky
        board[subBoard[8]*8 + i] = subBoard[i];
      }
      //checkRecievedBoard();
      //Kontrola zda byla deska přijata
      for(byte i = 0; i < packetLength/8; i++){
    if(!boardAck[i]){
      return;
    }
  }
  //Nulování
  resetBoardAck();
  //Vyhodnotí herní desku
  
  processBoard();
  //
    }
    else{
      //vyžádej balík znova
      //Serial.println("err SUM");
      sendData(subBoard[8], 20);
    }
  }
  else {
    return;
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> KOntrola, zda byla přijata deska celá, případně její vyhodnocení <<<<<
 /*   Princip:
  *    - Zkontroluje zda byla přijata deska (true vyplněno v board ack)
  *    - Vynuluje pole boardAck
  *    - Zavolá fci pro vyhodnocení board
  */
/*void checkRecievedBoard(void){
  for(byte i = 0; i < packetLength/8; i++){
    if(!boardAck[i]){
      return;
    }
  }
  //Nulování
  resetBoardAck();
  //Vyhodnotí herní desku
  processBoard();
}*/
//------------------------------------------------------------------------------------------------------
//>>>>> Odeslání dat serveru <<<<<
 /*   Princip:
  *    - Odešle serveru daná data
  *    - Aby je server vyhodnotil, je nutné je poslat 2x po sobě, pro lepší odolnost se odesílá 3x
  *    - pokud je code == 10: bude se přenášet vyplnění pole, pokud je code == 20 jedná se o request pro znovuodeslání části pole board
  */
void sendData(byte message, byte code){
  byte data[] = {code, message};
  delay(50);
  for(byte i = 0; i < 3; i++){
    client.write(data, 2);
    delay(3);
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Resetování pole pro přijatá data <<<<<
 /*   Princip:
  *    - resetuje pole, které kontroluje zda daná data byla přijata (vyplní false)
  */
void resetBoardAck(){
  for(byte i = 0; i < packetLength/8; i++){
    boardAck[i] = false;
  }
}
//------------------------------------------------------------------------------------------------------
