#include <math.h>
#include <SdFat.h>
#include <WaveRP.h>
#include <SdFatUtil.h>
#include <ctype.h>
#include "main.h"

void setup(){
  Serial.begin(9600);
  wave_shield_setup();
  buttons_setup();
  pinMode(recordingLED, OUTPUT); //allows the led to glow at full brightness/
}

void loop(){

  if (file.isOpen()) {
    file.close();
  }

  scanRoot();
  get_combination();
  trackPlay(currentComb);
}
