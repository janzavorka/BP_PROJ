//>>>>> Připraví a nastaví herní desku <<<<<
 /*   Princip:   
  *    - 
  */
void setBoard(){
  cleanBoard(); //Vyčistit herní desku
  syncBoardIPs(); //Synchronizace IP adresy
  board[gb_code] = 0;
  board[gb_actPlayer] = 0;
  //Barvy
  board[gb_PC1] = byte(RED & 0xFF);
  board[gb_PC1+1] = byte(RED >> 8);
  //
  board[gb_PC2] = byte(GREEN & 0xFF);
  board[gb_PC2+1] = byte(GREEN >> 8);
  //
  board[gb_PC3] = byte(PURPLE & 0xFF);
  board[gb_PC3+1] = byte(PURPLE >> 8);
  //
  board[gb_PC4] = byte(GREENYELLOW & 0xFF);
  board[gb_PC5+1] = byte(GREENYELLOW >> 8);
  //
  board[gb_PC5] = byte(OLIVE & 0xFF);
  board[gb_PC5+1] = byte(OLIVE >> 8);
  
  
}
//------------------------------------------------------------------------------------------------------
//>>>>> vynuluje herní desku <<<<<
 /*   Princip:   
  *    - vyplní herná desku nulami
  */
void cleanBoard(){
   //Nulování herní desky
  Serial.println("Nulovani pole pro herni desku");
  for (int i = 0; i < packetLength; i++){
    board[i] = 0;
  }
}
//------------------------------------------------------------------------------------------------------
