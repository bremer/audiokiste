// Audio Box
// (c) Matthias Bremer 2020 (arduino@neulesum.de)
// Licensed under CC0
//
// forked from
// Audio Advent Calendar
// (c) by Mischka 2011-2018 (mischka@mailbox.org)
// Licensed under CC0

// feel free to optimize and modify the code, please share your
// mods on the internet and send me a mail with the link

#include <JQ6500_Serial.h>
#include <SoftwareSerial.h>
#include <Wire.h>
// TODO for later implementation
//#include "RTClib.h"
#include <Keypad.h>
#include <EEPROM.h>

// size of the key pad
const byte COLS = 3;
const byte ROWS = 4;
const int LED = 13;
const unsigned int RADIOPLAY_FOLDER = 0; // in this folder you cannot skip tracks
const unsigned int VOLUME_MAX = 20; // 0-30
// Adresses for persistence
const unsigned int ADRESS_FILEINDEX = 0;
const unsigned int ADRESS_VOLUME = 5;

JQ6500_Serial mp3(2,3); // create an mp3 object
// TODO for later implementation
//RTC_DS1307 rtc;         // create an rtc object

// define key pad
char hexaKeys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte colPins[COLS] = { 12, 11, 10 }; // digital pins for key pad
byte rowPins[ROWS] = { 9, 8, 7, 6 }; // digital pins for key pad
Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

char pressedKey;
unsigned int folder = 9999;

unsigned int readVolume() {
  int volume = EEPROM.read(ADRESS_VOLUME);
  if (volume < 0 || volume > VOLUME_MAX) {
    // default if there is an other value in EEPROM
    volume = 10;
  }
  Serial.print("read volume ");
  Serial.print(volume);
  Serial.println();
  return volume;
}

void setup() {
  Serial.begin(57600);
  Serial.println("Audiokiste");
  
  pinMode(4, INPUT);
  pinMode(13, OUTPUT);

  // setup the mp3 module 
  mp3.begin(9600);
  mp3.reset();
  delay(500);
  mp3.setVolume(readVolume()); // 0-30 - start with x
  // TODO ask user which one is better
  // mp3.setLoopMode(MP3_LOOP_NONE);
  mp3.setLoopMode(MP3_LOOP_FOLDER);

  // setup rtc
  // TODO for later implementation
  // Wire.begin();
  // rtc.begin();

  // comment out after first flashing
  // rtc.adjust(DateTime(__DATE__, __TIME__));

  // TODO for later implementation
  // TODO speak current date at start up
  // DateTime now = rtc.now();
  // folder for date: 99
  // 001.mp3 .. 031.mp3 days
  // 101.mp3 .. 112.mp3 months
  //  mp3.playFileNumberInFolderNumber(now.day() ,1);
  //  delay(2000);
  //  mp3.playFileNumberInFolderNumber(now.month() ,1);
}

void persistVolume(unsigned int volume) {
  unsigned int volumePersisted = EEPROM.read(ADRESS_VOLUME);
  if (volume != volumePersisted) {
    EEPROM.write(ADRESS_VOLUME, volume);
    Serial.print("Volume persisted");
    Serial.print(volume);
    Serial.println();
  }
}

void persistCurrentTrack() {
  // TODO check if it is better to persist the tracknumer in folder
  int fileIndex = mp3.currentFileIndexNumber(MP3_SRC_SDCARD);
  int fileIndexPersistent;
  EEPROM.get(ADRESS_FILEINDEX, fileIndexPersistent);
  if (fileIndexPersistent != fileIndex) {
    EEPROM.put(ADRESS_FILEINDEX, fileIndex);
    Serial.print("persist Track ");
    Serial.print(fileIndex);
    Serial.println(); 
  }
}

void play(int nextFolder) {
  if(folder != nextFolder) {
    Serial.print("switch from folder ");
    Serial.print(folder);
    Serial.print(" to ");
    Serial.print(nextFolder);
    Serial.println();
    folder = nextFolder;
    mp3.playFileNumberInFolderNumber(folder, 1); 
  } else {
    mp3.next();
  }

  Serial.print("play folder ");
  Serial.print(folder);
  Serial.print(" File ");
  Serial.print(mp3.currentFileIndexNumber(MP3_SRC_SDCARD));
  Serial.println();
}

void playRadio(int nextFolder) {
  Serial.println("Radio play folder");
  if(folder != nextFolder) {
    Serial.print("play folder ");
    Serial.print(nextFolder);
    Serial.println();
    folder = nextFolder;

    int fileIndex;
    EEPROM.get(ADRESS_FILEINDEX, fileIndex);
    mp3.playFileByIndexNumber(fileIndex);
    delay(500);
    if(mp3.getStatus() != MP3_STATUS_PLAYING) {
      // it seems like there is no matching file
      mp3.playFileNumberInFolderNumber(folder, 1);
      Serial.println("play track 1");
      persistCurrentTrack();  
    }
  } else {
    if(mp3.getStatus() != MP3_STATUS_STOPPED) {
      Serial.println("do not skip tracks");
      return;
    }
    mp3.next();
    persistCurrentTrack();
  }
}

void loop() {
  pressedKey = keypad.getKey();
  if (pressedKey) {
    Serial.print("key ");
    Serial.print(pressedKey);
    Serial.print(" pressed");
    Serial.println();

    // select a folder
    if(pressedKey >= '0' && pressedKey <= '9'){
      int nextFolder = pressedKey - 48;
      if(RADIOPLAY_FOLDER == nextFolder) {
        playRadio(nextFolder);
      } else {
        play(nextFolder);        
      }
    } else if(pressedKey == '#') {
      // TODO persist current volume
      if (mp3.getVolume() <= VOLUME_MAX) {
        mp3.volumeUp();
        Serial.print("volume up to ");
        Serial.print(mp3.getVolume());
        Serial.println();
        persistVolume(mp3.getVolume());
      }
    } else if(pressedKey == '*') {
      mp3.volumeDn();
      Serial.print("volume down to ");
      Serial.print(mp3.getVolume());
      Serial.println();
      persistVolume(mp3.getVolume());
    }
  }

  // no key pressed, but radio track played to the end
  if(RADIOPLAY_FOLDER == folder && mp3.getStatus() == MP3_STATUS_STOPPED) {
    // TODO stop radio play tracks at the end
    // TODO check if it's nessesary
    persistCurrentTrack();
  }

}
