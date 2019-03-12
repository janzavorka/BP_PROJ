//>>>>> Vykreslení základního rámčku <<<<<
 /*   Princip:
  *    - Vykreslí modrý rámeček kolem celého displeje
  */
void drawMainFrame(uint16_t color){
  LCD.setColor(color); //Nastavení barvy
  LCD.drawRect(0, 0, 320, 240); //Vykreslení čtverce (souřadnice levý horní a pravý dolní roh)
  LCD.drawRect(1, 1, 319, 239);

}

//------------------------------------------------------------------------------------------------------
//>>>>> Vykreslení základního hrací mřížku <<<<<
 /*   Princip:
  *    - vykreslí modrou mříž jak hrací plochu
  *    - velikost 11x8 polí
  */
void drawMesh(uint16_t color){
  LCD.setColor(color); //Nastavení barvy
  for(int i = 0; i < meshX; i++){ //Svislé
    LCD.drawLine((int)i*resX/meshX, 0, (int)i*resX/meshX, resY);
  }
  for(int i = 0; i < meshY; i++){ //Vodorovné
    LCD.drawLine(0,(int)i*resY/meshY, resX, (int)i*resY/meshY);
  }
}

//------------------------------------------------------------------------------------------------------
//>>>>> Vykreslí do mřížky kolečka na základě hodnoty v board <<<<<
 /*   Princip:
  *    -
  *
  */
void drawPoints(){
  byte row = 0;
  byte column = 0;
  uint16_t colors[maxPlayers];

  for(byte i = 0; i < maxPlayers; i++){ //Zjištění jednotlivých barev
    colors[i] = getPlayerColor(colorAddr[i]);
  }

  for (int i = 0; i < meshX*meshY; i++){ //Překreslí všechny puntíky
      if(board[i] != 0){
        row = i/meshX;
        column = i - row*meshX;
        LCD.setColor(getPlayerColor(board[i])); //Nastaví barvu hráče podle čísla v poli
        LCD.fillCircle(column * resX/meshX + (resX/meshX)/2, row * resY/meshY + (resY/meshY)/2, 10);
      }
   }
}


//------------------------------------------------------------------------------------------------------
//>>>>> Vykreslení danou obrazovku podle ID <<<<<
 /*   Princip:
  *    - 0: Úvodní obrazovka
  *    - 1: Připojování k serveru
  *    - 2: Připojeno, čekání na zahájení hry
  *    - 3: Probíhající hra, kreslení mřížky a bodů
  *    - 4: Jako 3 + hláška (výhra, porhra apod.)
  */

  void drawPage (byte id){
    if(id != lastPage){ //Pokud byla předchozí stránka jiná, dojde k vyčištění displeje
      lastPage = id;
      LCD.clrScr();
    }
    button_index = 0;
    switch(id){
      case 0:
        //LCD.setColor(BLACK);
        //LCD.fillRect(20, 40, 310, 90);
        drawMainFrame(BLUE);
        drawHead(YELLOW);
        LCD.setTextSize(2);
        LCD.setCursor(20, 45);
        LCD.println("Moje IP: ");
        LCD.setCursor(150, 45);
        LCD.println(Ethernet.localIP());
        LCD.setCursor(20, 70);
        LCD.println("Server IP: ");
        LCD.setCursor(150, 70);
        LCD.println(serverAddress);
        buttons[button_index] = buttonRect(50, 270, 150, 210, 00, 1);
        LCD.setTextColor(BLACK, LIGHTGREY);
        LCD.setTextSize(3);
        LCD.setCursor(90,170);
        LCD.println("PRIPOJIT");
        screenRefresh = false;
        break;
      case 1:
        drawMainFrame(BLUE);
        //LCD.setColor(BLACK);
        //LCD.fillRect(20, 40, 310, 90);
        drawHead(YELLOW);
        LCD.setCursor(20, 50);
        LCD.setTextSize(2);
        LCD.println("Pripojuji se k serveru");
        buttons[button_index] = buttonRect(50, 270, 150, 210, 10, 1);
        LCD.setTextColor(RED, LIGHTGREY);
        LCD.setTextSize(3);

        LCD.print("PRERUSIT", 90,170);
        //screenRefresh = false;
        break;
      case 2:
        //LCD.clrScr();
        //button_index = 0;
        //LCD.setColor(BLACK);
        //LCD.fillRect(20, 40, 310, 90);
        drawMainFrame(BLUE);
        drawHead(getPlayerColor(getMyPlayerNumber()));
        LCD.setTextColor(getPlayerColor(getMyPlayerNumber()), BLACK);
        LCD.setTextSize(2);
        LCD.setCursor(90, 50);
        LCD.println("Pripojeno");
        LCD.setCursor(20, 70);
        LCD.println("Cekam na zahajeni hry");
        buttons[button_index] = buttonRect(50, 270, 150, 210, 20, 1);
        LCD.setTextColor(BLACK, LIGHTGREY);
        LCD.setTextSize(3);
        LCD.setCursor(100,170);
        LCD.println("ODPOJIT");
        break;
      case 3:
        if(board[gb_actPlayer] == getMyPlayerNumber()){ //Pokud jsem na řadě, obrazovka se vykreslí mojí barvou
          drawMainFrame(getPlayerColor(getMyPlayerNumber()));
          drawMesh(getPlayerColor(getMyPlayerNumber()));
          drawPoints();
        }
        else{ //Pokud hraje někdo jiný, mřížka bude šedá
          drawMainFrame(LIGHTGREY);
          drawMesh(LIGHTGREY);
          drawPoints();
        }
        break;
      case 4: //Stejná jako 3, navíc vytvoří černý obdélník por výpis hlášky
        drawPage(3);
        LCD.setColor(BLACK);
        LCD.fillRect(0,0, 320, 50);
    }
  }


//------------------------------------------------------------------------------------------------------
//>>>>> Vykreslení danou obrazovku podle ID <<<<<
 /*   Princip:
  *    - Vykreslí hlavičku "Piškvorky" podle požadované barvy (argument)
  */
void drawHead (uint16_t color){
  LCD.setTextColor(color, BLACK);
  LCD.setTextSize(3);
  LCD.print("Piskvorky", 80, 10, 1);
}

//------------------------------------------------------------------------------------------------------
