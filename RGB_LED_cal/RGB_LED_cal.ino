/* >>>>>>> KALIBRACE BAREV RGB LED <<<<<<<
 * - Program slouzi k odladeni pomeru jednotlivych slozek RGB pro zobrazeni dane barvy
 * - Ovladani (zmena barvy) pomoci seriove linky
 * - Napoveda pomoci prikazu "help"
 * - Prikaz: "led rrr,ggg,bbb,inten" slouzi pro nastaveni LED
 * - Vice na: https://github.com/janzavorka/BP_PROJ
 */

#define LED_red 6
#define LED_green 7
#define LED_blue 8

#define max_buffik 30
String buffik;

byte LED[] = {255,255,255,0};
//Prototypy
void serialEvent(void);
void print_WC(void);
void zpracuj_buffik(void);
void setLED(void);

void setup(){
  Serial.begin(9600);

  buffik.reserve(max_buffik);

  //output pins
  pinMode(LED_red, OUTPUT);
  pinMode(LED_green, OUTPUT);
  pinMode(LED_blue, OUTPUT);

  //Turn off LED
  analogWrite(LED_red, 255);
  analogWrite(LED_green, 255);
  analogWrite(LED_blue, 255);
}

void loop(){
  delay(10);
}
//------------------------------------------------------------------------------------------------------
//>>>>> Funkce pro zpracovani příkazů <<<<<
  /*  Zpracování příkazů
   *    - Pokud je na sériové lince co číst, uloží se to do stringu 'buffik'
   *    - Pokud je zaznamenána prázdná řádka, buffik se vyhodnotí
   *    - Pokud je zadán delší řetězec než rezervovaný počet znaků, vypíše se chyba
   */
void serialEvent(){
  char novy_char = 0;
  while(Serial.available()){
    if(buffik.length() >= (max_buffik-1)){
      print_WC();
    }
    novy_char = (char)Serial.read();
    if((novy_char == '\n') || ((byte)novy_char == (byte)10)){
      novy_char = 0;
      Serial.flush();
      zpracuj_buffik();
    }
    else{
      buffik += novy_char;
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
void zpracuj_buffik(){
  String num = "";
  num.reserve(4);
  if(buffik.equals("help")){
    Serial.println("_________________________________________________________________");
    Serial.println("HELP - prikazy:");
    Serial.println("led rrr,ggg,bbb,inten");
    Serial.println("                        rrr = intenzita cervene <0; 255>");
    Serial.println("                        ggg = intenzita zelene <0; 255>");
    Serial.println("                        bbb = intenzita modre <0; 255>");
    Serial.println("                        inten = intenzita cele LED <0; 100>");
    Serial.println("_________________________________________________________________");
    buffik="";
  }
  else if (buffik.startsWith("led")){
    byte index1, index2 = 0;
    byte cislo = 0;
    buffik.remove(0,4);
    //for colors
    for(byte i = 0; i < 3; i++){
      index1 = buffik.indexOf(',');
      //index2 = buffik.indexOf(',', index1+1);
      if(index1 < 0){
        Serial.println("err1");
        print_WC();
        return;
      }
      Serial.print("indexy "); Serial.println(index1); //Serial.print("   "); Serial.println(index2);
      num = buffik.substring(0, index1);
      cislo = num.toInt();
      if(cislo < 0 | cislo > 255){
        Serial.println("err2");
        print_WC();
        return;
      }
      LED[i] = cislo;
      buffik.remove(0, index1+1);
    }
    num = buffik.substring(0, buffik.length());
    cislo = num.toInt();
    if(cislo < 0 | cislo > 100){
      Serial.println("err3");
        print_WC();
        return;
      }
     LED[3] = cislo;
     Serial.println("hodnoty");
     Serial.println(LED[0]); 
     Serial.println(LED[1]); 
     Serial.println(LED[2]); 
     Serial.println(LED[3]); 
     Serial.println("========="); 
     setLED();
     buffik="";
  }
  else {
    print_WC();
  }
}
//------------------------------------------------------------------------------------------------------
void setLED(){
  byte RED = (byte)(LED[0]*(LED[3]/100.0));
  byte GREEN = (byte)(LED[1]*(LED[3]/100.0));
  byte BLUE = (byte)(LED[2]*(LED[3]/100.0));
  analogWrite(LED_red, 255-RED);
  analogWrite(LED_green, 255-GREEN);
  analogWrite(LED_blue, 255-BLUE);
}

