uint8_t playBegin(char* name) {
  if (!file.open(&root, name, O_READ)) {
    PgmPrint("Can't open: ");
    Serial.println(name);
  }

  if (!wave.play(&file)) {
    // If track does not exist
    if(currentComb>0) {
      trackRecord(currentComb);
    }
    else if(currentComb == 0) {
      return false;
    }
    else{
      PgmPrint("Can't play: ");
      Serial.println(name);
      file.close();
    }
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
  file.close();
#if PRINT_DEBUG_INFO
  if (wave.errors()) {
    PgmPrint("busyErrors: ");
    Serial.println(wave.errors(), DEC);
  }
#endif
}

unsigned long startRecord(void) {
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

// delete a track without checking
void trackDelete(int16_t track) {
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
  if (0 <= number && number <= 64) {
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
        trackDelete(track);
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
  if(startRecord() < 100) {
    trackDelete(track);
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

// Setup serial port and SD card
void wave_shield_setup(void) {
  delay(10);
  if (!card.init()) error("card.init");
  if (!vol.init(&card)) error("vol.init");
  if (!root.openRoot(&vol)) error("openRoot");
  card_info();
}
