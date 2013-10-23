// record rate - must be in the range 4000 to 44100 samples per second
// best to use standard values like 8000, 11025, 16000, 22050, 44100
#define RECORD_RATE 22050
//#define RECORD_RATE 44100

// max recorded file size.  Size should be a multiple of cluster size.
// the recorder creates and erases a contiguous file of this size.
// 100*1024*1024 bytes - about 100 MB or 150 minutes at 11025 samples/second
#define MAX_FILE_SIZE 104857600UL  // 100 MB
//#define MAX_FILE_SIZE 1048576000UL // 1 GB

// Analog pin connected to mic preamp
#define MIC_ANALOG_PIN 0

// Voltage Reference Selections for ADC
//#define ADC_REFERENCE ADC_REF_AREF  // use voltage on AREF pin
#define ADC_REFERENCE ADC_REF_AVCC  // use 5V VCC

// print the ADC range while recording if > 0
// print adcMax,adcMin if > 1
#define DISPLAY_RECORD_LEVEL 0

// print file info - useful for debug
#define PRINT_FILE_INFO 0

// print bad wave file size and SD busy errors for debug
#define PRINT_DEBUG_INFO 1

// debounce timer in ms
#define DEBOUNCE 10

#define SAR_TIMEOUT 4
#define SAR_THRESHOLD 40

// global variables
Sd2Card card;           // SD/SDHC card with support for version 2.00 features
SdVolume vol;           // FAT16 or FAT32 volume
SdFile root;            // volume's root directory
SdFile file;            // current file
WaveRP wave;            // wave file recorder/player
byte lastTrack = -1; // Highest track number
uint8_t trackList[32];  // bit list of used tracks
int currentComb = 0; //stores the combination of buttons that are currently pressed
int previousComb = -1;
int previousRecord = 0;
int recordingLED = 7;
int ledState = LOW;