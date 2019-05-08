/*>>>>>>> Piškvorky s arduinem po LAN <<<<<<<
*  !!! Součást programu pro server, samostatně nefunkční !!!
*
* - Autor: Jan Závorka
* - Email: zavorja4@fel.cvut.cz
* - Domovská stránka projektu: https://github.com/janzavorka/BP_PROJ
* - Seznam souborů: piskvorky_MP_server.ino; boardControl.ino; communication.ino; gameControl.ino; indicatioLED.ino; SerialControl.ino
*
* --- Popis:
* - Funkce zodpovědné za řízení herní desky (board) - pole, které uchovává stav hry a další informace
*/

//>>>>> Připraví a nastaví herní desku <<<<<
 /*   Princip:
  *    - nastaví potřebné odnoty pro začátek hry hodnoty pro začátek výhry
  */
void setBoard(){
  cleanBoard(); //Vyčistit herní desku
  syncBoardIPs(); //Synchronizace IP adresy
  board[gb_code] = 0; //Nastaví kód na "nedělej nic"
  board[gb_actPlayer] = 0; //Aktuální hráč nula (bude vybrán až při startu)

  //Přiřazení barev pro jednotlivé hráče, změnu lze provést v souboru piskvorky_MP_server.ino
  board[gb_PC1] = byte(PLAYER1COLOR & 0xFF);
  board[gb_PC1+1] = byte(PLAYER1COLOR >> 8);
  //
  board[gb_PC2] = byte(PLAYER2COLOR & 0xFF);
  board[gb_PC2+1] = byte(PLAYER2COLOR >> 8);
  //
  board[gb_PC3] = byte(PLAYER3COLOR & 0xFF);
  board[gb_PC3+1] = byte(PLAYER3COLOR >> 8);
  //
  board[gb_PC4] = byte(PLAYER4COLOR & 0xFF);
  board[gb_PC5+1] = byte(PLAYER4COLOR >> 8);
  //
  board[gb_PC5] = byte(PLAYER5COLOR & 0xFF);
  board[gb_PC5+1] = byte(PLAYER5COLOR >> 8);


}
//------------------------------------------------------------------------------------------------------
//>>>>> vynuluje herní desku <<<<<
 /*   Princip:
  *    - vyplní herní desku (board) nulami
  */
void cleanBoard(){
   //Nulování herní desky
  DEBUG_PRINTLN("Nulovani pole pro herni desku");

  for (int i = 0; i < packetLength; i++){
    board[i] = 0;
  }
}
//------------------------------------------------------------------------------------------------------
