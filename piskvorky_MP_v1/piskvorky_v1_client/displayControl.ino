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

  for(int i = 0; i < maxPlayers; i++){ //Zjištění jednotlivých barev
    colors[i] = getPlayerColor(colorAddr[i]);
  }

  for (int i = 0; i < meshX*meshY; i++){ //Překreslí všechny puntíky
      if(board[i] != 0){
        row = i/meshX;
        column = i - row*meshX;
        LCD.setColor(colors[board[i] -1]); //Nastaví barvu hráče podle čísla v poli
        LCD.fillCircle(column * resX/meshX + (resX/meshX)/2, row * resY/meshY + (resY/meshY)/2, 10);
      }
   }
}


//------------------------------------------------------------------------------------------------------
