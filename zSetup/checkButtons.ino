//The origin of this code was a page on debouncing multiple buttons located here:
//http://www.adafruit.com/blog/2009/10/20/example-code-for-multi-button-checker-with-debouncing/

byte buttons[] = {6,15,16,17,18,19};
#define NUMBUTTONS sizeof(buttons)
byte pressed[NUMBUTTONS];

//Setup buttons for digitalWrite
void buttonsSetup() {
  byte i;

  // Make input & enable pull-up resistors on switch pins
  for (i=0; i< NUMBUTTONS; i++) {
    pinMode(buttons[i], INPUT);
    digitalWrite(buttons[i], HIGH);
  }
}

// Debounce code
int get_combination() {
  static byte previousstate[NUMBUTTONS];
  static byte currentstate[NUMBUTTONS];
  static long lasttime;

  if ((lasttime + 50) > millis()) {
    return -1;
  }

  currentComb = 0;
  for (byte i = 0; i < NUMBUTTONS; i++) {
    currentstate[i] = digitalRead(buttons[i]);
    if(currentstate[i] == 1) {
      pressed[i] = (currentstate[i] == previousstate[i]) ? 1:0;
      if (pressed[i]) {
        int binPower = (NUMBUTTONS-i-1);
        currentComb += (int) pow(2,binPower);
      }
    }
    else {
      pressed[i] = 0;
    }
    previousstate[i] = currentstate[i];
  }

  lasttime = millis();
}

// play track
void play_track() {
  static int previousComb;

  if(currentComb > 0){
    Serial.println(currentComb);
    if(currentComb != previousComb){
      wave.stop();
      file.close();
    }
    trackPlay(currentComb);
    previousComb = currentComb;
  }
}

