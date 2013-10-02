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
int check_switches()
{
  static byte previousstate[NUMBUTTONS];
  static byte currentstate[NUMBUTTONS];
  static long lasttime;
  currentComb = 0;

  // debounce only when >10ms have passed since last run
  if ((lasttime + DEBOUNCE) > millis()) {
    return -1;
  }
  lasttime = millis();

  for (byte i = 0; i < NUMBUTTONS; i++) {
    currentstate[i] = digitalRead(buttons[i]);
    if(currentstate[i] == 1) {
      pressed[i] = (currentstate[i] == previousstate[i]) ? 1:0;
      if (pressed[i]) {
        int binPower = (NUMBUTTONS-i-1);
        currentComb += (0.5 + pow(2,binPower)); //need the .5 because its a float
      }
    }
    else {
      pressed[i] = 0;
    }
    previousstate[i] = currentstate[i];
  }
  return currentComb;
}

