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
       
      case 9: //Vyžádané odpojení ze strany serveru
        disconnectFromServer();
        break;
      case 100: //Remíza
          //Překreslí pole
          drawPage(3);
          //Informuje o remíze
          LCD.setTextColor(YELLOW, BLACK);
          LCD.setTextSize(3);
          LCD.setCursor(20, 20);
          LCD.println("Remiza");
        break;

      case 101:
      case 102:
      case 103:
      case 104:
      case 105:{
        byte winner = board[gb_code] - 100;
        //Překreslí pole
        drawPage(4);
        //Vypíše vítěze
        LCD.setTextColor(getPlayerColor(winner), BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(20, 20);
        if(getMyPlayerNumber() == winner){
          LCD.println("Vitez");
        }
        else{
          LCD.println("Vyhral hrac");
          LCD.setCursor(250, 20);
          LCD.println(winner);
        }
      }
        break;
      case 201:
      case 202:
      case 203:
      case 204:
      case 205:
        LCD.setColor(BLACK);
        LCD.fillRect(0,0, 320, 50);
        LCD.setTextColor(getPlayerColor(board[gb_code]-200), BLACK);
        LCD.setTextSize(3);
        LCD.print("Odpojen hrac", 20, 20);
        LCD.setCursor(250, 20);
        LCD.println(board[gb_code] - 200);
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
