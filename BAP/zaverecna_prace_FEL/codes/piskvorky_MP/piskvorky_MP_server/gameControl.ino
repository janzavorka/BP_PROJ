/*>>>>>>> Piškvorky s arduinem po LAN <<<<<<<
*  !!! Součást programu pro server, samostatně nefunkční !!!
*
* - Autor: Jan Závorka
* - Email: zavorja4@fel.cvut.cz
* - Domovská stránka projektu: https://github.com/janzavorka/BP_PROJ
* - Seznam souborů: piskvorky_MP_server.ino; boardControl.ino; communication.ino; gameControl.ino; indicatioLED.ino; SerialControl.ino
*
* --- Popis:
* - Část zodpovědná za řízení průběhu hry
*/

//>>>>> spustí hru <<<<<
 /*   Princip:
  *    - Zajistí nastavení herního pole, vybere prvního hráče a odešle řídicí kód
  */
void startGame (){
  if(serverPhase == 1){
    Serial.println("Aktualne bezi hra");
  }
  else{
    Serial.println("Spoustim hru");
    serverPhase = 1;
    syncBoardIPs();
    byte ONplayers = 0; //Pocet hracu online

    for (byte i = 0; i < maxPlayers; i++){ //Kontrola, zda jsou připojení alespoň dva hráči
      if(clients[i]){
        ONplayers++;
      }
    }

    if(ONplayers < 2){
      Serial.print("Je k dispozici jen ");
      Serial.print(ONplayers);
      Serial.println(" hracu, hra nemuze zacit.");
      signalLED.changeBlinkColor(LEDcol_red, 3);
      stopGame();
      return;
    }
    sendBoard(2); //Pošle všem příkaz k překreslení obrazovky (bez hráče)
    board[gb_round] = 0; //Vynulování počtu odehraných kol
    board[gb_actPlayer] = getNextPlayerNumber(random(1, maxPlayers)); //Náhodně se vybere číslo začínajícího hráče
    DEBUG_PRINT("Zacina hrac: ");
    DEBUG_PRINTLN(board[gb_actPlayer]);
    signalLED.changeStaticColor(LEDcol_green);
    delay(200);
    sendBoard(1);
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Zastaví běžící hru <<<<<
 /*   Princip:
  *    - Pošle ukončující řídicí kód
  *    - Změní barvu ndikační LED
  */
void stopGame (){
  Serial.println("Zastavuji hru");
  delay(5);
  sendBoard(3);
  serverPhase = 0;
  signalLED.changeStaticColor(LEDcol_blue);
  setBoard(); //Vyčistí herní pole
}
//------------------------------------------------------------------------------------------------------
//>>>>> Vrátí šíslo dalšího hráče <<<<<
 /*   Princip:
  *    - zkontroluje pole clients, aby se jednalo o hráče, který je připojen
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
  if(board[gb_round] >= meshX*meshY){ //Pokud hra skončila remízou (jsou obsazena všechna pole)
    board[gb_actPlayer] = 0;
    sendBoard(100);
    signalLED.changeBlinkColor(LEDcol_green, clientMessageLast/400); //Délka signalizace LED dopočtena z délky zobrazení chybové zprávy
    timer.setTimeout(clientMessageLast, stopGame); //Zpráva o remíze se zobrazí na určitou dobu, pak se resetuje hra
  }
  else{ //Vyhodnocení žetonů v poli (děje se pro naposled vyplněný žeton daného hráče)
    byte row = 0;
    byte column = 0;
    byte count = 0; //počet puntíků za sebou
    bool win = false;
    row = cross/11; //v jakém řádku se nachází vyplněný žeton
    column = cross%11; //v jakém sloupci se nachází vyplněný žeton

     for(byte i = 0; i < meshX; i++){ //kontrola celého řádku
        if(board[11*row + i ] == player){
          count++; //Počet žetonů vedle sebe
        }
        else{
          count = 0; //Pokud cizí žeton, počet se vynuluje
        }
        if (count >= crossNum){ //Pokud je dostatečný počet, hráč vyhrál
          win = true;
          break;
        }

     }
    count = 0;
     for(int i = 0; i < meshY; i++){ //kontrola daného sloupce
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

     byte index = 0;
     index = cross % 12;
     while (index < meshX*meshY){ //Kontrola do kříže jeden směr
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
     while (index < meshX*meshY){ //Kontrola do kříže druhý směr
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
    if(win){ //Pokud někdo vyhrál, odešle se zpráva
      Serial.print("Vyhral hrac: ");
      Serial.println(player);
      board[gb_actPlayer] = 0;
      sendBoard(100+player);
      signalLED.changeBlinkColor(LEDcol_green, clientMessageLast/500);
      timer.setTimeout(clientMessageLast, stopGame);

    }
    else{ //Jinak pokračuje další hráč
      #ifdef DEBUG
      Serial.println("Nikdo nevyhral, pokracuji");
      #endif

      shiftPlayer();
    }
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Posune (předá) hru dalšímu hráči <<<<<
 /*   Princip:
  *    - Vloží do pole číslo dalšího hráče
  */

void shiftPlayer(){
    board[gb_actPlayer] = getNextPlayerNumber(board[gb_actPlayer]); //Získej číslo dalšího hráče
    DEBUG0GAMEFL_PRINT("Dalsi hrac na rade je cislo: ");
    DEBUG0GAMEFL_PRINTLN(board[gb_actPlayer]);
    sendBoard(1); //Odešle desku s číslem dalšího hráče a s povelem k překreslení

}
//------------------------------------------------------------------------------------------------------
//>>>>> Vyplní token do herního pole <<<<<
 /*   Princip:
  *    - podle zadaného hráče a čísla pole vyplní token
  *    - pokud je pole mimo rozsah nebo je obsazené, vrátí false
  */

bool fillPlayerToken(byte coord, byte player){
    if(player == board[gb_actPlayer]){
      if(coord >= 0 && coord < meshX*meshY){
        if(board[coord] == 0){
            board[coord] = player;
            board[gb_round]++; //Zvýší se počet kol
            sendBoard(2);
            return true;
        }
        else{
          DEBUG0GAMEFL_PRINT("CHYBA - hrac ");
          DEBUG0GAMEFL_PRINT(player);
          DEBUG0GAMEFL_PRINT(" se pokousi vyplnit obsazene pole: ");
          DEBUG0GAMEFL_PRINTLN(coord);
          return false;
        }
      }
      else{
        DEBUG0GAMEFL_PRINT("CHYBA - hrac ");
        DEBUG0GAMEFL_PRINT(player);
        DEBUG0GAMEFL_PRINT(" se pokousi vyplnit pole mimo rozsah: ");
        DEBUG0GAMEFL_PRINTLN(coord);
        return false;
      }
    }
    else{
      DEBUG0GAMEFL_PRINT("CHYBA - hrac ");
      DEBUG0GAMEFL_PRINT(player);
      DEBUG0GAMEFL_PRINTLN(" neni na tahu");
      return false;
    }
}
//------------------------------------------------------------------------------------------------------
