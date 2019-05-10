/*>>>>>>> Piškvorky s arduinem po LAN <<<<<<<
*  !!! Součást programu pro klienta, samostatně nefunkční !!!
*
* - Autor: Jan Závorka
* - Email: zavorja4@fel.cvut.cz
* - Domovská stránka projektu: https://github.com/janzavorka/BP_PROJ
* - Seznam souborů: piskvorky_MP_client.ino; communication.ino; gameControl.ino; DisplayControl.ino
*
* --- Popis:
* - Část zodpovědná za vykreslování navázání a udržení spojení a výměnu dat.
*/

//>>>>> Připojení k serveru <<<<<
 /*   Princip:
  *    - pokusí se připojit k serveru, v případě spojení zapíše pošle 100 (informace, že chce být připojen)
  *    - v případě úspěšného spoojení vrátí true
  */
bool connectToServer(){
  if(!client.connected()){
    //Serial.println("Pokus o spojeni");
    client.connect(serverAddress, localPort);
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
  *    - Zkusí jestli server něco odeslal
  *    - přijímá postupně jednotlivé subpackety, kontroluje kontrolní součet
  *    - správně přijaté subpackety potvrzuje v poli boardAck
  *    - v případě, že nesedí kontrolní součet, vyžádá si data znovu
  */
void recieveBoard (){
  byte index = 0;
  byte subBoard [11];
  int checkSum = 0;
  if (client.connected() && client.available() > 10){ //client.available();
    while(index < 11){
      subBoard[index] = client.read();
      if(index <= 8){
        checkSum += subBoard[index];
      }
      index++;
      //delay(3);
    }
    if((int(subBoard[9]) | int(subBoard[10]) << 8) == checkSum){ //Pokud sedí kontrolní součet = data byla přijata správně
      boardAck[subBoard[8]] = true; //Potvrzení přijetí, na pozici 8 v subBoard je pořadové číslo packetu
      for(byte i = 0; i < 8; i++){ //Zápis dat do herní desky
        board[subBoard[8]*8 + i] = subBoard[i];
      }
      //Kontrola zda byla deska přijata
      for(byte i = 0; i < packetLength/8; i++){
    if(!boardAck[i]){
      return;
    }
  }
  //Nulování potvrzovacího pole
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
//>>>>> Odeslání dat serveru <<<<<
 /*   Princip:
  *    - Odešle serveru daná data
  *    - Aby je server vyhodnotil, je nutné je poslat 2x po sobě, pro lepší odolnost se odesílá 3x
  *    - pokud je code == 10: bude se přenášet vyplnění pole, pokud je code == 20 jedná se o request pro znovuodeslání části pole board
  */
void sendData(byte message, byte code){
  if(client.connected()){
    byte data[] = {code, message};
    for(byte i = 0; i < 3; i++){
      client.write(data, 2);
      delay(2);
    }
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Resetování potvrzovacího pole pro přijatá data <<<<<
 /*   Princip:
  *    - resetuje pole, které kontroluje zda daná data byla přijata (vyplní false)
  */
void resetBoardAck(){
  for(byte i = 0; i < packetLength/8; i++){
    boardAck[i] = false;
  }
}
//------------------------------------------------------------------------------------------------------

//>>>>> Odpojení od serveru <<<<<
 /*   Princip:
  *    - zruší spojení se serverem a vypíše chybovou hlášku na displej
  */
void disconnectFromServer(){
  client.stop();
  serverConnection = false;
  board[gb_code] = 0;
  LCD.setColor(BLACK);
  LCD.fillRect(0,0, 320, 50);
  LCD.setTextColor(MAROON, BLACK);
  LCD.setTextSize(3);
  LCD.print("ODPOJEN", 20, 20);
  lastPage = 255;
  delay(5000);
  drawPage(0);

}
//------------------------------------------------------------------------------------------------------
