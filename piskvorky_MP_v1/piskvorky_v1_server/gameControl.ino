//------------------------------------------------------------------------------------------------------
//>>>>> Zastaví běžící hru <<<<<
 /*   Princip:   
  *    - 
  */
void stopGame (){
  Serial.println("Zastavuji hru, tlacitko");
  board[gb_code] = 3;
  delay(5);
  sendBoard(3);
  serverPhase = 0;
}
//------------------------------------------------------------------------------------------------------
//>>>>> Vrátí šíslo dalšího hráče <<<<<
 /*   Princip:   
  *    - zkontroluje pole clientts, aby se jednalo o hráče, který je připojen
  *    - hrac1 je číslo předchozího (aktuálně hrajícího hráče)
  */
byte getNextPlayerNumber(byte player1){
  do{
    player1++;
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
//>>>>> Zkontroluje běh hry <<<<<
 /*   Princip:   
  *    - zkontroleuje běh hry
  *    - vyhodnocuje výhru hráče
  *    - vyhodnocuje remízu (vysoký počet herních kol)
  *    - automaticky zapíše do kódu v boardu
  */
void checkGame(byte cross, byte player){
  if(board[gb_round] >= meshX*meshY){
    board[gb_code] = 100;
  }
  else{
    byte row = 0;
    byte column = 0;
    byte count = 0; //počet puntíků za sebou
    bool win = false;
    row = cross/11;
    column = cross%11;
  
     for(byte i = 0; i < meshX; i++){ //v řádku
        if(board[11*row + i ] == player){
          count++;
        }
        else{
          count = 0;
        }
        if (count >= crossNum){
          win = true;
          break;
        }
      
     }
    count = 0;
     for(int i = 0; i < meshY; i++){ //v sloupec
        if(board[column + i*11 ] == player){
          count++;
        }
        else{
          count = 0;
        }
        if (count >= crossNum){
          win = true;
          break;
        }
      
     }
     count = 0;
      //Do kříže
     byte index = 0;
     index = cross % 12;
     while (index < meshX*meshY){
        if(board[index] == player){
          count++;
        }
        else{
          count = 0;
        }
        index += 12;
        if (count >= crossNum){
          win = true;
          break;
        }
     }
  
     count = 0;
     index = cross % 10;
     while (index < meshX*meshY){
        if(board[index] == player){
          count++;
        }
        else{
          count = 0;
        }
        index += 10;
        if (count >= crossNum){
          win = true;
          break;
        }
     }
    if(win){
      Serial.print("Vyhral hrac: "); Serial.println(player);
      board[gb_code] = 100+player; //Pokud byla zaznamenána výhra, zeznamená se kód s čílem hráče do příslušného pole
      sendBoard(100+player);
      delay(7000);
      stopGame ();
    }
    else{
      Serial.println("Nikdo nevyhral, pokracuji");
    }
  }
}
//------------------------------------------------------------------------------------------------------

//>>>>> spustí hru <<<<<
 /*   Princip:   
  *    - 
  */
void startGame (){
  if(serverPhase == 2){
    Serial.println("Aktualne bezi hra");
  }
  else{
    Serial.println("Spoustim hru, tlacitko");
    serverPhase = 1;
  }
}
//------------------------------------------------------------------------------------------------------
