//>>>>> Zracovává přijatou herí desku <<<<<
 /*   Princip:
  *    - POstupně zpracuje přijatou herní duesku
  *
  */

void processBoard(){
  switch(board[gb_code]){
    case 0: //Podle čísla hráče překreslit nebo hrát
      if(board[gb_actPlayer] == myNum){
        drawMainFrame(BLUE);
        drawMesh(BLUE);
        gamePhase = 4;
      }
      else{
        drawMainFrame(LIGHTGREY);
        drawMesh(LIGHTGREY);
        drawPoints();
        gamePhase = 3;
      }
      break;

    case 2: //Vložení kolečka do herní desky
      drawMainFrame(BLUE);
      drawMesh(BLUE);
      gamePhase = 4;
      break;

    case 3:
      gamePhase = 2;
      screenRefresh = true;
      break;

    case 100: //Remíza
        //Překreslí pole
        drawMainFrame(LIGHTGREY);
        drawMesh(LIGHTGREY);
        drawPoints();
        //Informuje o remíze
        LCD.setColor(BLACK);
        LCD.fillRect(0,0, 320, 50);
        LCD.setTextColor(YELLOW, BLACK);
        LCD.setTextSize(3);
        LCD.setCursor(20, 20);
        LCD.println("Remiza");
        delay(10000);
        //prepareNewGame();
      break;

    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
      //Překreslí pole
      drawMainFrame(LIGHTGREY);
      drawMesh(LIGHTGREY);
      drawPoints();
      //Vypíše vítěze
      byte winner = board[gb_code] - 100;
      LCD.setColor(BLACK);
      LCD.fillRect(0,0, 320, 50);
      LCD.setTextColor(getPlayerColor(colorAddr[winner - 1]), BLACK);
      LCD.setTextSize(3);
      LCD.setCursor(20, 20);
      if(winner == myNum){
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
//------------------------------------------------------------------------------------------------------
//>>>>> Zjištění barvy hráče <<<<<
 /*   Princip:
  *    - argument je index v poli board, podle něho je zjištěna daná barva
  *    - návratová hodnota je ona barva
  */

uint16_t getPlayerColor(byte start){
  return uint16_t(board[start] | uint16_t(board[start+1]) << 8);
}
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
//>>>>> Zjištění moje číslo hráče <<<<<
 /*   Princip:
  *    - Projde jednotlivé IP v boardu a pokusí se najít shodu s mojí IP = moje číslo
  *    - pokud nenajde shodu vrátí -1
  */

byte getMyPlayerNumber(){
  bool haveNumber = false;
  IPAddress locIP = Ethernet.localIP();
  for(byte i = 0; i < maxPlayers; i++){
    if(board[IPaddr[i]] == locIP[0] && board[IPaddr[i]+1] == locIP[1] && board[IPaddr[i]+2] == locIP[2] && board[IPaddr[i]+3] == locIP[3]){
      Serial.print("Cislo: "); Serial.println(i+1);
      return (i+1);
    }
  }
  return 0;
}
//------------------------------------------------------------------------------------------------------
