#include <math.h>
#include <SdFat.h>
#include <WaveRP.h>
#include <SdFatUtil.h>
#include <ctype.h>
#include "main.h"

// Arduino setup
void setup(){
  Serial.begin(9600);
  waveShieldSetup();
  buttonsSetup();
  pinMode(recordingLED, OUTPUT); //allows the led to glow at full brightness/
}

void loop(){

  static int previousComb;
  int16_t track = -1;
  uint8_t c;

  // close previously opened file if not
  if (file.isOpen()) {
    file.close();
  }

  //scan root dir to build track list and set lastTrack
  scanRoot();

  while(track < 256) {

    currentComb = check_switches();
    if(currentComb > 0){

      // debounce the combination of buttons
      if ((ccDebounceTimer + 50) < millis()) {
        if(currentComb == previousComb){
          trackPlay(currentComb);
        }
        ccDebounceTimer = millis(); //set the timer
        previousComb = currentComb; //set previous to current.
      }
    }
  }
}
