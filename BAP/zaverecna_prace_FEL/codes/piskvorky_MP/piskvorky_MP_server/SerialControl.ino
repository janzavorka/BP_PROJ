/*>>>>>>> Piškvorky s arduinem po LAN <<<<<<<
*  !!! Součást programu pro server, samostatně nefunkční !!!
*
* - Autor: Jan Závorka
* - Email: zavorja4@fel.cvut.cz
* - Domovská stránka projektu: https://github.com/janzavorka/BP_PROJ
* - Seznam souborů: piskvorky_MP_server.ino; boardControl.ino; communication.ino; gameControl.ino; indicatioLED.ino; SerialControl.ino
*
* --- Popis:
* - Řídí komunikace po sériové lince
* - Vyhodnocuje přijaté příkazy
*/

//>>>>> Ovládání přes sériovou linku <<<<<
  /*  Princip:
   *    - Využívá funkci serialEvent(), která je automaticky volána v loop()
   *    - Přijme data, pokud je přijatá nová řádka, data vyhodnotí
   */
void serialEvent(){
  char new_char = 0;
  while(Serial.available()){ //Pokud je co číst
    if(buffik.length() >= (max_buffik-1)){ //Pokud je přijatých dat moc, zahodí je (špatný příkaz)
      print_WC();
    }
    new_char = (char)Serial.read();
    if((new_char == '\n') || ((byte)new_char == (byte)10)){ //Pokud je přijatý znak nová řádka, dojde k vyhodnocení
      new_char = 0;
      Serial.flush();
      processBuffik();
    }
    else{//Jinak přidej přijatý znak do buffiku
      buffik += new_char;
    }
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Funkce pro vypsání špatného příkazu <<<<<
  /*  Popis:
  *     - Vypíše hlášku, že byl přijat špatný příkaz
   *    - Zadaný příkaz vypíše na sériovou linku
   */
void print_WC(){
  Serial.print("Neplatny prikaz ");
  Serial.println(buffik);
  buffik = ""; //Resetuje buffik
}
//------------------------------------------------------------------------------------------------------
//>>>>> Funkce pro provedení příkazů předaných sériovou linkou<<<<<
  /*  Princip:
   *    - Provede zadané příkazy
   */
void processBuffik(){
  // ******* příkaz HELP *******
  if(buffik.equals("help")){
    printLine('_', 70);
    Serial.println("Napoveda pro piskvorkovy server:");
    Serial.print("help          Vypise tuto napovedu \n");
    Serial.print("info          Vypise informace o piskvorkovem serveru \n");
    Serial.print("players       Vypise seznam hracu a jejich IP adresy \n");
    Serial.print("kick 'x'      Odpoji hrace cislo x \n");
    Serial.print("nextP         Prepne na dalsiho hrace \n");
    Serial.print("start         Spusti hru (pokud jsou k dispozici alespon dva hraci \n");
    Serial.print("reset         Prerusi hru a resetuje herni pole \n");
    printLine('_', 70);
    buffik="";
  }
  // ******* příkaz INFO *******
  else if (buffik.equals("info")){
    printLine('_', 70);
    Serial.println("Informace o piskvorkovem serveru");

    Serial.print("Pouzity WIZnet controler: ");
    switch (getHWcontroller()) {
      case 0:
        Serial.println("no HW found");
        break;
      case 1:
        Serial.println("W5100");
        break;
      case 2:
        Serial.println("W5200");
        break;
      case 3:
        Serial.println("W5500");
        break;
      default:
        Serial.println("undefined");
    }

    Serial.print("MAC adresa: ");
    for(byte i = 0; i < 6; i++){
      Serial.print(mac[i], HEX);
      if(i < 5){
        Serial.print(":");
      }
    }
    Serial.println();

    Serial.print("IP adresa: ");
    Serial.println(Ethernet.localIP());

    Serial.print("Port: ");
    Serial.println(localPort);

    Serial.print("Datum vydane verze v tomto zarizeni: ");
    Serial.println(makeDate);

    Serial.print("Domovska stranka projektu: https://github.com/janzavorka/BP_PROJ\n");
    printLine('_', 70);
    buffik="";
  }
  // ******* příkaz players *******
  else if(buffik.equals("clients") || buffik.equals("players")){
    byte clientsCount = 0;
    printLine('_', 70);
    Serial.println("Vypisuji informace o pripojenych hracich");
    Serial.print("Cislo   IP adresa \n");
    for(int i = 0; i < maxPlayers; i++){ //Vypisuje cisla aktivnich hracu a jejich IP adresy
      if(clients[i] && clients[i].connected()){
          clientsCount++;
          Serial.print(i+1);
          Serial.print("       ");
          Serial.print(clients[i].remoteIP());
          Serial.print("\n");
      }
    }
   Serial.print("Celkovy pocet hracu: ");
   Serial.print(clientsCount);
   Serial.print("\n");
   printLine('_', 70);
   buffik="";
  }
  // ******* příkaz START *******
  else if(buffik.equals("start")){
    startGame();
    buffik="";
  }
  // ******* příkaz RESET *******
  else if(buffik.equals("reset")){
    stopGame();
    buffik="";
  }
  // ******* příkaz KICK player *******
  else if(buffik.startsWith("kick")){
    byte spaceIndex = 0;
    int kickedPlayer = 0;
    String num = "";
    num.reserve(3);

    spaceIndex = buffik.indexOf(' '); //Lokalizuje v prikazu cislo hrace
    num = buffik.substring(spaceIndex+1, buffik.length());
    kickedPlayer = num.toInt(); //A prevede ho na int

    if(kickedPlayer > 0 && kickedPlayer <= maxPlayers){
      if(clients[kickedPlayer-1]){ //Pokud je dany hrac aktivni, je odpojen
        disconnectPlayer(kickedPlayer);
      }
      else{
        Serial.print("Hrac cislo ");
        Serial.print(kickedPlayer);
        Serial.print(" neni pripojen!\n");
      }
    }
    else{
      Serial.println("Neplatne cislo hrace!");
    }
    buffik="";
  }
  // ******* příkaz nextP *******
  else if(buffik.equals("nextP")){
    shiftPlayer(); //Předá hru dalšímu hráči
    buffik="";
  }
  else {
    print_WC();
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Vypíše daný znak na sériovou linku <<<<<
  /*  Princip:
   *    - Vypisuje zadaný počet zadaných znaků na sériovou linkou
   *    - Pro oddělovače a tabulky
   */
void printLine(byte chr, byte count){
  for(byte i = 0; i < count; i++){
    Serial.print((char)chr);
  }
  Serial.println();
}
