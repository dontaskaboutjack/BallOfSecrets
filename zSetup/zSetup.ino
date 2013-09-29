#include <math.h>
#include <SdFat.h>
#include <WaveRP.h>
#include <SdFatUtil.h>
#include <ctype.h>
#include "main.h"

void setup(){
  Serial.begin(9600);
  waveSetup(); //remainder of original setup functions from the library
  checkButtonsSetup(); //setup from rewritten debounce code
  pinMode(recordingLED,OUTPUT); //allows the led to glow at full brightness/
}

//Main loop file of the ball of secrets
//It incorporates some of the code from the other files (WaveRecordPlay) which need to be looped through
//The main chunk of the loop is the central area where its essentially debounce code but for currentComb,
//which represents the sum of the pressed buttons.

void loop(){
  //FROM WAVERECORDPLAY
  if (file.isOpen()) file.close();
  //scan root dir to build track list and set lastTrack
  scanRoot();
  while (Serial.read() >= 0) {}
  //PgmPrintln("\ntype a command or h for help");
  int16_t track = -1;
  uint8_t c;


  while(track < 256){
    //start by checking the switches to get the currentCoimb
    currentComb = check_switches(); //get the current value of the switches, prints currentComb

    if(currentComb > 0){ //Only proceed if some button is pressed

      if ((ccDebounceTimer + 1000) < millis()) { //is it time to debounce?
        //Serial.println("passed time check, now entering debounce check");
        if(currentComb == previousComb){ //check that the buttons pressed are the same
          searchForFile(currentComb); //proceed with playing or recording
        }
        ccDebounceTimer = millis(); //set the timer
        previousComb = currentComb; //set previous to current.
      }
    }
    //while (!Serial.available()) {} //loops in here s/serial input; keeping for debugging
    c = Serial.read();
    if (!isdigit(c)); break;
    track = (track < 0 ? 0 : 10 * track) + c - '0';
  }
}
