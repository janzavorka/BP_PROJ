//>>>>> Zracovává přijatou herí desku <<<<<
 /*   Princip:
  *    - Postupně zpracuje přijatou herní duesku
  *
  */

void processBoard(){
  if(getMyPlayerNumber() > 0){
    switch(board[gb_code]){
      case 0: //Nic se neděje
        break;
      case 1: //Podle čísla hráče překreslit nebo hrát
        drawPage(3);
        break;
      case 3:
        drawPage(2);
        break;

      case 100: //Remíza
          //Překreslí pole
          drawPage(3);
          //Informuje o remíze
          LCD.setTextColor(YELLOW, BLACK);
          LCD.setTextSize(3);
          LCD.setCursor(20, 20);
          LCD.println("Remiza");
          //delay(10000);
          //prepareNewGame();
        break;

      case 101:
      case 102:
      case 103:
      case 104:
      case 105:
        //Překreslí pole
        drawPage(3);
        //Vypíše vítěze
        byte winner = board[gb_code] - 100;
        LCD.setTextColor(getPlayerColor(winner), BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(20, 20);
        if(winner == getMyPlayerNumber()){
          LCD.println("Vitez");
        }
        else{
          LCD.println("Vyhral hrac");
          LCD.setCursor(250, 20);
          LCD.println(winner);
        }
        break;
    }
  }
  else{
    //Serial.println("Nejsi ve hre");
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Zjištění barvy hráče <<<<<
 /*   Princip:
  *    - argument je index v poli board, podle něho je zjištěna daná barva
  *    - návratová hodnota je ona barva
  */

uint16_t getPlayerColor(byte player){
  return uint16_t(board[colorAddr [player-1]]) | uint16_t(board[colorAddr [player-1] +1] << 8);
}
//------------------------------------------------------------------------------------------------------
//>>>>> Zjištění moje číslo hráče <<<<<
 /*   Princip:
  *    - Projde jednotlivé IP v boardu a pokusí se najít shodu s mojí IP = moje číslo
  *    - pokud nenajde shodu vrátí -1
  */

byte getMyPlayerNumber(){
  //bool haveNumber = false;
  IPAddress locIP = Ethernet.localIP();

  for(byte i = 0; i < maxPlayers; i++){
    /*Serial.println(board[105]);
  Serial.println(board[106]);
  Serial.println(board[107]);
  Serial.println(board[108]);*/
    if(board[IPaddr[i]] == locIP[0] && board[IPaddr[i]+1] == locIP[1] && board[IPaddr[i]+2] == locIP[2] && board[IPaddr[i]+3] == locIP[3]){
     // Serial.print("Cislo: "); Serial.println(i+1);
      return (i+1);
    }
  }
  return 0;
}
//------------------------------------------------------------------------------------------------------
