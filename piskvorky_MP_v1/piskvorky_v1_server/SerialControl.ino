//>>>>> Ovládání přes sériovou linku <<<<<
  /*  Princip:
   *    - Využívá funkci serialEvent
   */
void serialEvent(){
  char new_char = 0;
  while(Serial.available()){
    if(buffik.length() >= (max_buffik-1)){
      print_WC();
    }
    new_char = (char)Serial.read();
    if((new_char == '\n') || ((byte)new_char == (byte)10)){
      new_char = 0;
      Serial.flush();
      processBuffik();
    }
    else{
      buffik += new_char;
    }
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Funkce pro vypsání špatného příkazu <<<<<
  /*  Vypíše chybové hlášení
   *    - Vypíše na sériovou linku zadaný text
   */
void print_WC(){
  Serial.println("Neplatny prikaz !");
  Serial.println(buffik);
  buffik = "";
}
//------------------------------------------------------------------------------------------------------
//>>>>> Funkce pro provedení příkazů předaných sériobou linkou<<<<<
  /*  Princip:
   *    - Provede zadané příkazy
   */
void processBuffik(){
  String num = ""; //pro čísla
  num.reserve(4);
  if(buffik.equals("help")){
    printLine('_', 70);
    Serial.println("Napoveda pro piskvorkovy server:");
    Serial.print("help          Vypise tuto napovedu \n");
    Serial.print("info          Vypise informace o piskvorkovem serveru \n");
    Serial.print("clients       Vypise seznam clientu a jejich IP adresy \n");
    Serial.print("start         Spusti hru (pokud jsou k dispozici alespon dva hraci \n");
    Serial.print("reset         Prerusi hru a resetuje herni pole \n");
    printLine('_', 70);
    buffik="";
  }
  else if (buffik.equals("info")){
    Serial.println("tady bude info");
    buffik="";
  }
  else if(buffik.equals("clients")){
    byte clientsCount = 0;
    printLine('_', 70);
    Serial.println("Vypisuji informace o připojených clientech");
    Serial.print("Cislo   IP adresa \n");
    for(int i = 0; i < maxPlayers; i++){
      if(clients[i] && clients[i].connected()){
          clientsCount++;
          Serial.print(i+1);
          Serial.print("       ");
          Serial.print(clients[i].remoteIP());
          Serial.print("\n");
      }
    }
   Serial.print("Celkovy pocet clientu: ");
   Serial.print(clientsCount);
   Serial.print("\n"); 
   printLine('_', 70);
   buffik="";
  }
  else if(buffik.equals("start")){
    startGame();
    buffik="";
  }
  else if(buffik.equals("reset")){
    stopGame();
    buffik="";
  }
  else {
    print_WC();
  }
}
//------------------------------------------------------------------------------------------------------
//>>>>> Vypíše daný znak na sériovou linku <<<<<
  /*  Princip:
   *    - 
   */
void printLine(byte chr, byte count){
  for(byte i = 0; i < count; i++){
    Serial.print((char)chr);
  }
  Serial.println();
}

