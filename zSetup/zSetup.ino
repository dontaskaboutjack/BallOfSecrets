#include <math.h> // for calculating the current combination
int currentComb; //stores the combination of buttons that are currently pressed
int recordingLED = 7; 

void setup(){
  Serial.begin(9600);
  waveSetup(); //remainder of original setup functions from the library
  checkButtonsSetup(); //setup from rewritten debounce code
  pinMode(recordingLED,OUTPUT); //allows the led to glow at full brightness/
}
