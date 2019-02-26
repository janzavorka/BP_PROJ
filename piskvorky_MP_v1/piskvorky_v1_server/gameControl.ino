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
  setBoard(); //Vyčistí herní pole
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
    Serial.println("Spoustim hru");
    serverPhase = 1;
    syncBoardIPs();
    byte ONplayers = 0; //Pocet hracu online
    //Cislo prvniho hrace
    //byte firstPlayer = random(1, maxPlayers);

    for (byte i = 0; i < maxPlayers; i++){ //Kontrola, zda jsou připojení alespoň dva hráči
      if(clients[i]){
        ONplayers++;
      }
    }

    if(ONplayers < 2){
      Serial.print("Je k dispozici jen ");
      Serial.print(ONplayers);
      Serial.println(" hracu, hra nemuze zacit.");
      stopGame();
      return;
    }
    sendBoard(1); //Pošle všem příkaz k překreslení obrazovky (bez hráče)

    board[gb_actPlayer] = getNextPlayerNumber(random(1, maxPlayers)); //Náhodně se vybere číslo začínajícího hráče
    delay(100);
    sendBoard(1);
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Posune (předá) hru dalšímu hráči <<<<<
 /*   Princip:
  *    -
  */

void shiftPlayer(){
    byte nextPlayer = getNextPlayerNumber(board[gb_actPlayer]); //Získej číslo dalšího hráče
    board[gb_actPlayer] = nextPlayer;
    sendBoard(1); //Odešle desku s číslem dalšího hráče a s povelem k překreslení

}
//------------------------------------------------------------------------------------------------------
//>>>>> Vyplní token do herního pole <<<<<
 /*   Princip:
  *    - podle zadaného hráče a čísla pole vyplní token
  *    - pokud je pole mimo rozsah nebo je obsazené, vrátí false
  */

bool fillPlayerToken(byte coord, byte player){
    if(coord => 0 && coord < meshX*meshY){
      if(board[coord] == 0){

      }
      else{
        Serial.print("CHYBA - hrac ");
        Serial.print(player);
        Serial.print(" se pokousi vyplnit obsazene pole: ");
        Serial.println(coord);
        return false;
      }
    }
    else{
      Serial.print("CHYBA - hrac ");
      Serial.print(player);
      Serial.print(" se pokousi vyplnit pole mimo rozsah: ");
      Serial.println(coord);
      return false;
    }
}
//------------------------------------------------------------------------------------------------------
