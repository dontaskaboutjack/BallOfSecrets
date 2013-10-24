// Print error message and halt
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

// Blink LED with provided interval
void blinkLED(unsigned long interval) {
  static unsigned long previousMillis = 0;

  if(millis()-previousMillis > interval){
    previousMillis = millis();

    if(ledState == LOW) {
      ledState = HIGH;
    }else {
      ledState = LOW;
    }

    digitalWrite(recordingLED, ledState);
  }
}

// Clear all bits in track list
void listClear(void)  {
  memset(trackList, 0, sizeof(trackList));
}

// Return bit for track n
uint8_t listGet(uint8_t n) {
  return (trackList[n >> 3] >> (n & 7)) & 1;
}

// Set bit for track n
void listSet(uint8_t n) {
  trackList[n >> 3] |= 1 << (n & 7);
}

// Scan root directory for track list and recover partial tracks
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