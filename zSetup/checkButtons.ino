//The origin of this code was a page on debouncing multiple buttons located here:
//http://www.adafruit.com/blog/2009/10/20/example-code-for-multi-button-checker-with-debouncing/

//The code took a significant amount of rewriting however to get it to work - they had some complicated and buggy features.


#define DEBOUNCE 10  // number of ms to debounce
byte buttons[] = {6,15,16,17,18,19}; 
#define NUMBUTTONS sizeof(buttons)
byte pressed[NUMBUTTONS];

//original setup code, now called by the zSetup
void checkButtonsSetup() {
  byte i;
  
  // Make input & enable pull-up resistors on switch pins
  for (i=0; i< NUMBUTTONS; i++) {
    pinMode(buttons[i], INPUT);
    digitalWrite(buttons[i], HIGH);
  }
}

//Function which checks all of the button values and debounces them.
//Returns the sum of the binary indices of the buttons

int check_switches()
{
  //Serial.println("getting to check_switches");
  static byte previousstate[NUMBUTTONS];
  static byte currentstate[NUMBUTTONS];
  static long lasttime;
  byte index;

  if (millis() < lasttime) {
     // we wrapped around, lets just try again
     lasttime = millis();
  }
  
  if ((lasttime + DEBOUNCE) > millis()) {
    // not enough time has passed to debounce
    return -1; 
  }
  // ok we have waited DEBOUNCE milliseconds, lets reset the timer
  lasttime = millis();
  
  //Actual rewritten debounced code
  for (index = 0; index < NUMBUTTONS; index++) {
    currentstate[index] = digitalRead(buttons[index]);
    if(currentstate[index] == 1) {
      if (currentstate[index] == previousstate[index]) {
        pressed[index] = 1;  
      }
      if (currentstate[index] != previousstate[index]) {
        pressed[index] = 0;
      }
    }
    
    if (currentstate[index] == 0){
      pressed[index]=0; 
    }
    
    previousstate[index] = currentstate[index];

  }
  currentComb = 0; //reset the current combination so it can be recalculated
  int isPressed = 0; // index to keep track of how many buttons are pressed.


  for(int i=0;i<sizeof(pressed);i++){
    if (pressed[i]==1){
      isPressed++;
    }
  }

//Code to calculate current combination
  //Basically, each button is a binary. By summing those binaries, we get to a number 1-63,
  //which serves as the "hash key" of sorts for the audio file.
  for (int i = 0; i < NUMBUTTONS; i++) {
    if (pressed[i]) {
      int binPower = (NUMBUTTONS-i-1);
      currentComb += (0.5 + pow(2,binPower)); //need the .5 because its a float
    }
  }

  return currentComb;
}

