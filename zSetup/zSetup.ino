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

  // close previously opened file if not
  if (file.isOpen()) {
    file.close();
  }

  //scan root dir to build track list and set lastTrack
  scanRoot();
  get_combination();
  play_track();
}
