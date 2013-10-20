byte buttons[] = {6,15,16,17,18,19};
#define NUMBUTTONS sizeof(buttons)
byte pressed[NUMBUTTONS];

//Setup buttons for digitalWrite
void buttons_setup() {
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
  previousComb = currentComb;
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

//compare combination states
boolean compare_combination(){
  if(previousComb != currentComb){
    return false;
  }
  return true;
}

