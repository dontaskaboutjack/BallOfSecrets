// print error message and halt
void error(char* str) {
  PgmPrint("error: ");
  Serial.println(str);
  if (card.errorCode()) {
    PgmPrint("sdError: ");
    Serial.println(card.errorCode(), HEX);
    PgmPrint("sdData: ");
    Serial.println(card.errorData(), HEX);
  }
  while(1);
}

// print help message
void help(void) {
  PgmPrintln("a     play all WAV files in the root dir");
  PgmPrintln("c     clear - deletes all tracks");
  PgmPrintln("d     delete last track");
  PgmPrintln("<n>d  delete track number <n>");
  PgmPrintln("h     help");
  PgmPrintln("l     list track numbers");
  PgmPrintln("p     play last track");
  PgmPrintln("<n>p  play track number <n>");
  PgmPrintln("r     record new track as last track");
  PgmPrintln("<n>r  record over deleted track <n>");
  PgmPrintln("v     record new track voice activated");
  PgmPrintln("<n>v  record over deleted track voice activated");
}

// used
// clear all bits in track list
void listClear(void)  {
  memset(trackList, 0, sizeof(trackList));
}

// return bit for track n
uint8_t listGet(uint8_t n) {
  return (trackList[n >> 3] >> (n & 7)) & 1;
}

// print list of tracks in ten columns with each column four characters wide
void listPrint(void) {
  PgmPrintln("\nTrack list:");
  uint8_t n = 0;
  uint8_t nc = 0;
  do {
    if (!listGet(n)) continue;
    if (n < 100) Serial.print(' ');
    if (n < 10) Serial.print(' ');
    Serial.print(n, DEC);
    if (++nc == 10) {
      Serial.println();
      nc = 0;
    } else {
      Serial.print(' ');
    }
  } while (n++ != 255);
  if (nc) Serial.println();
}

// set bit for track n
void listSet(uint8_t n) {
  trackList[n >> 3] |= 1 << (n & 7);
}

// check for pause resume BMCHANGE
void pauseResume(void) {
  //coded added here trying to get the ability to delete files, but it would just corrupt the SD Card. Leaving in for later attempts.
  if ((check_switches() == potentialDeleteWhilePlaying) && (deleteWhilePlayingTimer + 3000 < millis())) {
    wave.stop();
    file.close();
    Serial.println("trying to delete");
  }

  if (!Serial.available()) return;
  uint8_t c = Serial.read();
  while (Serial.read() >= 0) {}
  if (c == 's') {
    wave.stop();
  } else if (c == 'p') {
    wave.pause();
    while (wave.isPaused()) {
      PgmPrintln("\nPaused - type 's' to stop 'r' to resume");
      while (!Serial.available()) {}
      c = Serial.read();
      if (c == 's') wave.stop();
      if (c == 'r') wave.resume();
    }
  }
}

// play all files in the root dir
void playAll(void) {
  dir_t dir;
  char name[13];
  uint8_t np = 0;
  root.rewind();
  while (root.readDir(&dir) == sizeof(dir)) {
    //only play wave files
    if (strncmp_P((char *)&dir.name[8], PSTR("WAV"), 3)) continue;
    // remember current dir position
    uint32_t pos = root.curPosition();
    // format file name
    SdFile::dirName(dir, name);
    if (!playBegin(name)) continue;
    PgmPrintln(", type 's' to skip file 'a' to abort");
    while (wave.isPlaying()) {
      if (Serial.available()) {
        uint8_t c = Serial.read();
        while (Serial.read() >= 0) {}
        if (c == 's' || c == 'a') {
          wave.stop();
          file.close();
          if (c == 'a') return;
        }
      }
    }
    file.close();
    // restore dir position
    root.seekSet(pos);
  }
}
//------------------------------------------------------------------------------
// start file playing BMCHANGE
//Function now only allows for checking whether a file exists at that location, and if not, recording it.
uint8_t playBegin(char* name) {
  deleteWhilePlayingTimer = millis(); //start the timer for deleting
  potentialDeleteWhilePlaying = currentComb;
  if (!file.open(&root, name, O_READ)) { //if we cant open it, not sure what to do here.
    PgmPrint("Can't open: ");
    Serial.println(name);
  }
  if (!wave.play(&file)) { // if we cant play the file
    if(currentComb>0) { //and if we actually did call a file
      recordFromButtons(currentComb); //then record one for that combination
      return false;
    }
    else if(currentComb == 0) {
      return false;
    }
    PgmPrint("Can't play: ");
    Serial.println(name);
    file.close();
    return false;
  }
#if PRINT_FILE_INFO
  Serial.print(wave.bitsPerSample, DEC);
  PgmPrint("-bit, ");
  Serial.print(wave.sampleRate/1000);
  PgmPrintln(" kps");
#endif // PRINT_FILE_INFO
#if PRINT_DEBUG_INFO
  if (wave.sdEndPosition > file.fileSize()) {
    PgmPrint("play Size mismatch,");
    Serial.print(file.fileSize());
    Serial.print(',');
    Serial.println(wave.sdEndPosition);
  }
#endif // PRINT_DEBUG_INFO
  PgmPrint("Playing: ");
  Serial.print(name);
  return true;
}
//------------------------------------------------------------------------------
// play a file, sends it to playBegin to do the work. Calls pauseResume
void playFile(char* name) {
  if (!playBegin(name)) {
    return;
  }
  PgmPrintln(", type 's' to stop 'p' to pause");
  while (wave.isPlaying()) {
    pauseResume();
  }
  file.close();
#if PRINT_DEBUG_INFO
  if (wave.errors()) {
    PgmPrint("busyErrors: ");
    Serial.println(wave.errors(), DEC);
  }
#endif // PRINT_DEBUG_INFO
}
//-----------------------------------------------------------------------------
//BMCHANGE Altered this function so that it would only record for 5000 seconds with another debouncer
void recordManualControl(void) {
  PgmPrintln("Recording - type 's' to stop 'p' to pause");
  uint8_t nl = 0;
  unsigned long timer = millis(); //timer to track this recording = time at recording start
  while (wave.isRecording()) {
    digitalWrite(recordingLED,HIGH);
#if DISPLAY_RECORD_LEVEL > 0
    wave.adcClearRange();
    delay(500);
#if DISPLAY_RECORD_LEVEL > 1
    Serial.print(wave.adcGetMax(), DEC);
    Serial.print(',');
    Serial.println(wave.adcGetMin(), DEC);
#else // #if DISPLAY_RECORD_LEVEL > 1
    Serial.print(wave.adcGetRange(), DEC);
    if (++nl % 8) {
      Serial.print(' ');
    } else {
      Serial.println();
    }
#endif // DISPLAY_RECORD_LEVEL > 1
#endif // DISPLAY_RECORD_LEVEL > 0
    // check for pause/stop
    pauseResume();
    Serial.println("just left pauseResume");
    if (currentComb==0) {
      Serial.println("currentComb>0 check passed");
      Serial.print("timer is ");
      Serial.print(timer);
      Serial.print(" and millis is ");
      Serial.println(millis());
      if ((timer + 5000) < millis()) {
        Serial.println("stopping wave");
        wave.stop();
      }
    }
    digitalWrite(recordingLED,LOW);
  }
}
//-----------------------------------------------------------------------------
#define SAR_TIMEOUT 4
#define SAR_THRESHOLD 40
void recordSoundActivated(void) {
  uint32_t t;
  wave.pause();
  uint8_t n = 0;
  wave.adcClearRange();
  PgmPrintln("Recording - type 's' to stop");
  while (1) {
    if (wave.adcGetRange() >= SAR_THRESHOLD) {
      if (wave.isPaused()) {
        wave.resume();
        Serial.print('r');
        if (++n % 40 == 0) Serial.println();
      }
      t = millis();
      wave.adcClearRange();
    } else if (!wave.isPaused()) {
      if ((millis() - t) > 1000*SAR_TIMEOUT) {
        wave.pause();
        Serial.print('p');
        if (++n % 40 == 0) Serial.println();
      }
    }
    if (Serial.read() == 's') {
      wave.stop();
      return;
    }
  }
}

// used
// scan root directory for track list and recover partial tracks
void scanRoot(void) {
  dir_t dir;
  char name[13];
  listClear();
  root.rewind();
  lastTrack = -1;
  while (root.readDir(&dir) == sizeof(dir)) {
    // only accept TRACKnnn.WAV with nnn < 256
    if (strncmp_P((char *)dir.name, PSTR("TRACK"), 5)) continue;
    if (strncmp_P((char *)&dir.name[8], PSTR("WAV"), 3)) continue;
    int16_t n = 0;
    uint8_t i;
    for (i = 5; i < 8 ; i++) {
      char c = (char)dir.name[i];
      if (!isdigit(c)) break;
      n *= 10;
      n += c - '0';
    }
    // nnn must be three digits and less than 256
    if (i != 8 || n > 255) continue;
    if (n > lastTrack) lastTrack = n;
    // mark track found
    listSet(n);
    if (dir.fileSize != MAX_FILE_SIZE) continue;
    // try to recover untrimmed file
    uint32_t pos = root.curPosition();
    if (!trackName(n, name)
      || !file.open(&root, name, O_READ |O_WRITE)
      || !wave.trim(&file)) {
      if (!file.truncate(0)) {
        PgmPrint("Can't trim: ");
        Serial.println(name);
      }
    }
    file.close();
    root.seekSet(pos);
  }
}

// delete all tracks on SD
void trackClear(void) {
  char name[13];
  while (Serial.read() >= 0) {}
  PgmPrintln("Type Y to delete all tracks!");
  while (!Serial.available()) {}
  if (Serial.read() != 'Y') {
    PgmPrintln("Delete all canceled!");
    return;
  }
  for (uint16_t i = 0; i < 256; i++) {
    if (!listGet(i)) continue;
    if (!trackName(i, name)) return;
    if (!SdFile::remove(&root, name)) {
      PgmPrint("Delete failed for: ");
      Serial.println(name);
      return;
    }
  }
  PgmPrintln("Deleted all tracks!");
}
//------------------------------------------------------------------------------
// delete a track
void trackDelete(int16_t track) {
  char name[13];
  if (!trackName(track, name)) return;
  while (Serial.read() >= 0) {}
  PgmPrint("Type y to delete: ");
  while (!Serial.available()) {}
  if (Serial.read() != 'y') {
    PgmPrintln("Delete canceled!");
    return;
  }
  if (SdFile::remove(&root, name)) {
    PgmPrintln("Deleted!");
  } else {
    PgmPrintln("Delete failed!");
  }
}
//------------------------------------------------------------------------------
// delete a track without checking
void trackDeleteNoCheck(int16_t track) {
  char name[13];
  if (!trackName(track, name)) return;
  if (SdFile::remove(&root, name)) {
    PgmPrintln("Deleted!");
  } else {
    PgmPrintln("Delete failed!");
  }
}
//------------------------------------------------------------------------------
// format a track name in 8.3 format
uint8_t trackName(int16_t number, char* name) {
  if (0 <= number && number <= 255) {
    strcpy_P(name, PSTR("TRACK000.WAV"));
    name[5] = '0' + number/100;
    name[6] = '0' + (number/10)%10;
    name[7] = '0' + number%10;
    return true;
  }
  PgmPrint("Invalid track number: ");
  Serial.println(number);
  return false;
}
//------------------------------------------------------------------------------
// play a track
void trackPlay(int16_t track) {
  char name[13];
  if (!trackName(track, name)) {return;}
  playFile(name);
}
//------------------------------------------------------------------------------
// record a track
void trackRecord(int16_t track, uint8_t mode) {
  char name[13];
  if (track < 0) track = lastTrack + 1;
  if (!trackName(track , name)) return;
  if (file.open(&root, name, O_READ)) {
    PgmPrint("Track already exists. Use '");
    Serial.print(track);
    Serial.print("d' to delete it.");
    file.close();
    return;
  }
  PgmPrint("Creating: ");
  Serial.println(name);
  if (!file.createContiguous(&root, name, MAX_FILE_SIZE)) {
    PgmPrintln("Create failed");
    return;
  }
  if(!wave.record(&file, RECORD_RATE, MIC_ANALOG_PIN, ADC_REFERENCE)) {
    PgmPrintln("Record failed");
    file.remove();
    return;
  }
  if (mode == 'v') {
    recordSoundActivated();
  } else {
    recordManualControl();
  }
  // trim unused space from file
  wave.trim(&file);
  file.close();
#if PRINT_DEBUG_INFO
  if (wave.errors() ){
    PgmPrint("busyErrors: ");
    Serial.println(wave.errors(), DEC);
  }
#endif // PRINT_DEBUG_INFO
}

// SD Card information
void card_info(void) {
  uint8_t bpc = vol.blocksPerCluster();
  PgmPrint("BlocksPerCluster: ");
  Serial.println(bpc, DEC);
  uint8_t align = vol.dataStartBlock() & 0X3F;
  PgmPrint("Data alignment: ");
  Serial.println(align, DEC);
  PgmPrint("sdCard size: ");
  Serial.print(card.cardSize()/2000UL);PgmPrintln(" MB");
  if (align || bpc < 64) {
    PgmPrintln("\nFor best results use a 2 GB or larger card.");
    PgmPrintln("Format the card with 64 blocksPerCluster and alignment = 0.");
    PgmPrintln("If possible use SDFormater from www.sdcard.org/consumers/formatter/");
  }
  if (!card.eraseSingleBlockEnable()) {
    PgmPrintln("\nCard is not erase capable and can't be used for recording!");
  }
}

// Setup serial port and SD card
void waveShieldSetup(void) {
  delay(10);
  PgmPrint("FreeRam: ");
  Serial.println(FreeRam());
  if (!card.init()) error("card.init");
  if (!vol.init(&card)) error("vol.init");
  if (!root.openRoot(&vol)) error("openRoot");
  card_info();
}
