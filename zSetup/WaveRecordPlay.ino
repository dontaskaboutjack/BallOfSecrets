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

void blinkLED(unsigned long interval) {
  static unsigned long previousMillis = 0;

  if(millis()-previousMillis > interval){
    previous Millis = millis();

    if(ledState == LOW) {
      ledState = HIGH;
    }else {
      ledState = LOW;
    }

    digitalWrite(recordingLED, ledState);
  }
}

// clear all bits in track list
void listClear(void)  {
  memset(trackList, 0, sizeof(trackList));
}

// return bit for track n
uint8_t listGet(uint8_t n) {
  return (trackList[n >> 3] >> (n & 7)) & 1;
}

// set bit for track n
void listSet(uint8_t n) {
  trackList[n >> 3] |= 1 << (n & 7);
}

uint8_t playBegin(char* name) {
  if (!file.open(&root, name, O_READ)) {
    PgmPrint("Can't open: ");
    Serial.println(name);
    trackRecord(currentComb);
  }

  // If track does not exist
  if (!wave.play(&file)) {
    if(currentComb>0) {
      trackRecord(currentComb);
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

void playFile(char* name) {
  if (!playBegin(name)) {
    return;
  }
  while (wave.isPlaying()) {
    get_combination();
    if(!compare_combination()){
      wave.stop();
      file.close();
      return;
    }
  }
  file.close();l
#if PRINT_DEBUG_INFO
  if (wave.errors()) {
    PgmPrint("busyErrors: ");
    Serial.println(wave.errors(), DEC);
  }
#endif
}

unsigned long recordManualControl(void) {
  PgmPrintln("Recording");
  uint8_t nl = 0;
  unsigned long timer = millis();
  while (wave.isRecording()) {
    blinkLED(1000);
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

    get_combination();
    if (!compare_combination() || (millis() - timer > 10000)) {
      digitalWrite(recordingLED,LOW);
      wave.stop();
      PgmPrint("Duration:");
      Serial.println(millis()-timer);
    }
  }

  previousRecord = 1;
  return (millis() - timer);
}

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

// play a track
void trackPlay(int16_t track) {
  if(track > 0) {
    PgmPrint("Playing track number:");
    Serial.println(track);
    char name[13];
    if (!trackName(track, name)) {
      return;
    }
    if(compare_combination()) {
      unsigned long timer = millis();
      while(millis() - timer < 2000) {
        get_combination();
        if(currentComb == 0) {
          return;
        }
      }
      if(compare_combination()) {
        trackDeleteNoCheck(track);
        trackRecord(track);
        return;
      }
    }
    playFile(name);
  }
}

// record a track
void trackRecord(int16_t track) {
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
  if(recordManualControl() < 100) {
    trackDeleteNoCheck(track);
  } else {
    wave.trim(&file);
    file.close();
  }
#if PRINT_DEBUG_INFO
  if (wave.errors() ){
    PgmPrint("busyErrors: ");
    Serial.println(wave.errors(), DEC);
  }
#endif // PRINT_DEBUG_INFO
}

// SD Card information
void card_info(void) {
  PgmPrint("FreeRam: ");
  Serial.println(FreeRam());
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
void wave_shield_setup(void) {
  delay(10);
  if (!card.init()) error("card.init");
  if (!vol.init(&card)) error("vol.init");
  if (!root.openRoot(&vol)) error("openRoot");
  card_info();
}
