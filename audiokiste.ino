// Audio Box
// (c) Matthias Bremer 2020 (arduino@neulesum.de)
// Licensed under CC0
//
// forked from
// Audio Advent Calendar
// (c) by Mischka 2011-2018 (mischka@mailbox.org)
// Licensed under CC0
//
// feel free to optimize and modify the code, please share your
// mods on the internet and send me a mail with the link

#include <JQ6500_Serial.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Keypad.h>
#include <EEPROM.h>

// size of the key pad
const byte COLS = 3;
const byte ROWS = 4;

const int SWITCH_STOPTANZ = 4;
const int SWITCH_SLEEPMODE = 5;

const unsigned int LULLABY_FOLDER = 20; //
const unsigned int RADIOPLAY_FOLDER = 0; // in this folder you cannot skip tracks
const unsigned int VOLUME_MAX = 25; // 0-30

// Adresses for persistence
const unsigned int ADRESS_FILEINDEX = 0;
const unsigned int ADRESS_VOLUME = 5;

JQ6500_Serial mp3(2,3); // create an mp3 object

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

// status varables
char pressedKey;
unsigned int folder = 9999;
unsigned int tracknumber = 1;
int statusStoptanz;
int statusSleepmode;



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
  
  pinMode(SWITCH_STOPTANZ, INPUT);
  pinMode(SWITCH_SLEEPMODE, INPUT);
  pinMode(13, OUTPUT);

  statusSleepmode = digitalRead(SWITCH_STOPTANZ);
  statusSleepmode = digitalRead(SWITCH_SLEEPMODE);

  // setup the mp3 module 
  mp3.begin(9600);
  mp3.reset();
  delay(400);
  mp3.setVolume(readVolume()); // 0-30 - start with x

  // mp3.setLoopMode(MP3_LOOP_NONE);
  mp3.setLoopMode(MP3_LOOP_FOLDER);
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
  int tracknumberPersistent;
  EEPROM.get(ADRESS_FILEINDEX, tracknumberPersistent);
  if (tracknumber != tracknumberPersistent) {
    EEPROM.put(ADRESS_FILEINDEX, tracknumber);
    Serial.print("persist Track ");
    Serial.print(tracknumber);
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
  Serial.print(tracknumber);
  Serial.println();
}

void playRadio(int nextFolder) {
  Serial.println("Radio play folder");
  if(folder != nextFolder) {
    Serial.print("play folder ");
    Serial.print(nextFolder);
    Serial.println();
    folder = nextFolder;

    EEPROM.get(ADRESS_FILEINDEX, tracknumber);
    mp3.playFileNumberInFolderNumber(folder,tracknumber);
    delay(500);
    if(mp3.getStatus() != MP3_STATUS_PLAYING) {
      mp3.playFileNumberInFolderNumber(folder, 1);
      Serial.println("it seems like there is no matching file - play track 1");
    }
  } else {
    mp3.playFileNumberInFolderNumber(folder,++tracknumber);
    persistCurrentTrack();
  }
}

void checkSwitches() {
  
  int newStatusStoptanz = digitalRead(SWITCH_STOPTANZ);
  if (statusStoptanz != newStatusStoptanz) {
    delay(100);
    statusStoptanz = newStatusStoptanz;
    Serial.print("Stoptanz ");
    Serial.print(statusStoptanz);
    Serial.println();
  }

  int newStatusSleepmode = digitalRead(SWITCH_SLEEPMODE);
  if (statusSleepmode != newStatusSleepmode) {
    delay(100);
    statusSleepmode = newStatusSleepmode;
    Serial.print("Sleepmode ");
    Serial.print(statusSleepmode);
    Serial.println();
    // TODO play LULLABY_FOLDER 
  }

}

void loop() {
  checkSwitches();
  
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

}
