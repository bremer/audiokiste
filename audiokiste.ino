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
// 
// Schalter Pause
// Schalter Stopptanz
// * leiser
// # lauter
// 0 Schalaflieder (folder 00)
// 1 - 9 Musik (folder 01 - 09)

// https://sparks.gogo.co.nz/jq6500/doxygen/index.html
#include <JQ6500_Serial.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <avr/sleep.h>

// size of the key pad
const byte COLS = 3;
const byte ROWS = 4;

const int SWITCH_STOPTANZ = 4;
const int SWITCH_PAUSE = 5;

const unsigned int DELAY_STOPTANZ = 5000;
const unsigned int VOLUME_MAX = 25; // 0-30

// Adresses for persistence
const unsigned int ADRESS_FILEINDEX_BASE = 0; // 0-9
const unsigned int ADRESS_VOLUME = 10;

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
int statusPause;
long timeStarttanz = 2147483647L;
long timeStoptanz = 2147483647L;


void setup() {
  Serial.begin(57600);
  Serial.println("Audiokiste");
  
  pinMode(SWITCH_STOPTANZ, INPUT);
  pinMode(SWITCH_PAUSE, INPUT);
  pinMode(13, OUTPUT);

  statusStoptanz = digitalRead(SWITCH_STOPTANZ);
  statusPause = digitalRead(SWITCH_PAUSE);

  // setup the mp3 module 
  mp3.begin(9600);
  mp3.reset();
  delay(300);
  mp3.setVolume(readVolume()); // 0-30 - start with x

  mp3.setLoopMode(MP3_LOOP_FOLDER);

  randomSeed(analogRead(0));
}

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

unsigned int readTrack(unsigned int folder) {
  int track = EEPROM.read(ADRESS_FILEINDEX_BASE + folder);
  if (track < 1 || track > 255) {
    // default if there is an other value in EEPROM
    track = 1;
  }
  Serial.print("read track ");
  Serial.print(track);
  Serial.print(" folder ");
  Serial.print(folder);
  Serial.println();
  return track;
}

void persistVolume(unsigned int volume) {
  unsigned int volumePersisted = EEPROM.read(ADRESS_VOLUME);
  if (volume != volumePersisted) {
    EEPROM.write(ADRESS_VOLUME, volume);
    Serial.print("Volume persisted ");
    Serial.print(volume);
    Serial.println();
  }
}

void persistTrack(unsigned int folder, unsigned int track) {
  unsigned int trackPersisted = EEPROM.read(ADRESS_FILEINDEX_BASE + folder);
  if (track != trackPersisted) {
    EEPROM.write(ADRESS_FILEINDEX_BASE + folder, track);
    Serial.print("Track ");
    Serial.print(track);
    Serial.print(" Folder ");
    Serial.print(folder);
    Serial.print(" persisted");
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
    tracknumber = readTrack(folder);
    mp3.playFileNumberInFolderNumber(folder, tracknumber); 
    delay(500);
    if(mp3.getStatus() != MP3_STATUS_PLAYING) {
      tracknumber = 1;
      mp3.playFileNumberInFolderNumber(folder, tracknumber);
      persistTrack(folder, tracknumber); 
      Serial.println("it seems like there is no matching file - play track 1");
    }

  } else {
    mp3.playFileNumberInFolderNumber(folder, ++tracknumber);
    persistTrack(folder, tracknumber); 
  }

  Serial.print("play folder ");
  Serial.print(folder);
  Serial.print(" File ");
  Serial.print(tracknumber);
  Serial.println();
}

void checkSwitches() {
  
  int newStatusStoptanz = digitalRead(SWITCH_STOPTANZ);
  if (statusStoptanz != newStatusStoptanz) {
    delay(100);
    statusStoptanz = newStatusStoptanz;
    Serial.print("Stoptanz ");
    Serial.print(statusStoptanz);
    Serial.println();
    if(statusStoptanz) {
      timeStarttanz = millis();
      timeStoptanz = random(timeStarttanz + 2000, timeStarttanz + 15000);
    } else {
      mp3.play();
      timeStarttanz = 2147483647L;
      timeStoptanz = 2147483647L;
    }
  }

  int newStatusPause = digitalRead(SWITCH_PAUSE);
  if (statusPause != newStatusPause) {
    delay(100);
    statusPause = newStatusPause;
    Serial.print("Pause ");
    Serial.print(statusPause);
    Serial.println();
    if(statusPause) {
      mp3.pause();
    } else {
      mp3.play();
    } 
  }
}

void checkStoptanz() {
  if(statusStoptanz) {
    long time = millis();
    if(time >= timeStoptanz && mp3.getStatus() == MP3_STATUS_PLAYING) {
      mp3.pause();
      timeStarttanz = time + DELAY_STOPTANZ;
    } else if(time >= timeStarttanz && mp3.getStatus() == MP3_STATUS_PAUSED) {
      mp3.play();
      timeStarttanz = time;
      timeStoptanz = random(time + 5000, time + 20000);
    }
  }
}

//void checkSleepmode() {
//    if(statusSleepmode) {
//      delay(500);
//      if(mp3.getStatus() == MP3_STATUS_STOPPED) {
//        Serial.println("Sleep mode end... sleep");
        // mp3.sleep();
        // Arduino sleep mode
        // attachInterrupt(0, INT_PIN, LOW); // wake up via interupt
        // set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        // sleep_enable();
        // sleep_mode();
        // Continue here after wake up
        // but not implemented yet
        // sleep_disable(); 
//      }
//    }
//}

void loop() {
  checkSwitches();

  checkStoptanz();
  
  pressedKey = keypad.getKey();
  if (pressedKey) {
    Serial.print("key ");
    Serial.print(pressedKey);
    Serial.print(" pressed");
    Serial.println();

    // select a folder
    if(pressedKey >= '0' && pressedKey <= '9'){
      int nextFolder = pressedKey - 48;
      play(nextFolder);        
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
