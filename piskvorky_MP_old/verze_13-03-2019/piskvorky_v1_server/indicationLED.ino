/*
* Řízení informační LED
* pro správné fungování je nutná knihovna SimpleTimer
* více informací o jednotlivých stavech LED je k dispozici na githubu
*/



RGB_LED::RGB_LED(){}
RGB_LED::RGB_LED(byte Red, byte Green, byte Blue, byte intens){
  p_RED = Red;
  p_GREEN = Green;
  p_BLUE = Blue;
  intensity = intens;
  //Nastaví piny jako výstupní
  pinMode(p_RED, OUTPUT);
  pinMode(p_GREEN, OUTPUT);
  pinMode(p_BLUE, OUTPUT);
  //Zhasnutí LED
  analogWrite(p_RED, 255);
  analogWrite(p_GREEN, 255);
  analogWrite(p_BLUE, 255);
}

//Změna statické barvy
void RGB_LED::changeStaticColor(byte color[]){
  for(byte i = 0; i < 3; i++){
    static_col[i] = color[i];
  }
  if (!timer.isEnabled(blinkTimerID)){ //POkud LED nebliká, provede se změna barvy
    LEDon();
  }
}

//Změna intenzity LED
void RGB_LED::changeIntensity(byte intens){
  intensity = intens;
  if (!timer.isEnabled(blinkTimerID)){ //POkud LED nebliká, provede se změna intenzity
    LEDon();
  }
}

//Zabliká LED (kolikrát určeno proměnou times), times se násobí dvakrát aby seděl počet bliknutí
void RGB_LED::changeBlinkColor(byte color[], byte times){
  for(byte i = 0; i < 3; i++){
    blink_col[i] = color[i];
  }
  if(!timer.isEnabled(blinkTimerID)){ //Hlídá zda neběží jiné blikání
    blinkTimerID  = timer.setTimer(250, std::bind(&RGB_LED::blink,this), times*2+1);
    blinkCount = times*2+1;
  }
  else{
    timer.deleteTimer(blinkTimerID);
    blinkTimerID  = timer.setTimer(250, std::bind(&RGB_LED::blink,this), times*2+1);
    blinkCount = times*2+1;
  }
}

//Bliká samotnou LED, pokud je počet bliknutí nula, vrátí se ke statické barvy
void RGB_LED::blink(){
  blinkCount --;
  if(blinkState){
    byte L_RED = (byte)(blink_col[0]*(intensity/100.0)*(LED_br/100.0));
    byte L_GREEN = (byte)(blink_col[1]*(intensity/100.0)*(LED_br/100.0));
    byte L_BLUE = (byte)(blink_col[2]*(intensity/100.0)*(LED_br/100.0));
    analogWrite(p_RED, 255-L_RED);
    analogWrite(p_GREEN, 255-L_GREEN);
    analogWrite(p_BLUE, 255-L_BLUE);
  }
  else{
    LEDoff();
  }
  blinkState = !blinkState;
  if(blinkCount==0){
    blinkState = false;
    LEDon();
  }
}

//Rozsvítí LED statickou barvou
void RGB_LED::LEDon(){
  byte L_RED = (byte)(static_col[0]*(intensity/100.0)*(LED_br/100.0));
  byte L_GREEN = (byte)(static_col[1]*(intensity/100.0)*(LED_br/100.0));
  byte L_BLUE = (byte)(static_col[2]*(intensity/100.0)*(LED_br/100.0));
  analogWrite(p_RED, 255-L_RED);
  analogWrite(p_GREEN, 255-L_GREEN);
  analogWrite(p_BLUE, 255-L_BLUE);
}

//Zhasne LED
void RGB_LED::LEDoff(){
  analogWrite(p_RED, 255);
  analogWrite(p_GREEN, 255);
  analogWrite(p_BLUE, 255);
}
