#include <EEPROM.h>
#include <Wire.h>
#include <PN532.h>
#include <PN532_I2C.h>
#include <sha1.h>
#include "DB.h"
#include "acksess.h"
#include <avr/wdt.h>

// Don't use pin 13 as it's used by the bootloader and will trigger the door to open on reboot
#define triggerPin 8

DB db;
User user;
uint8_t state;
PN532_I2C pn532_i2c(Wire);
PN532 nfc(pn532_i2c);

unsigned char page = {0x00};
byte challenge[3] = {0x09, 0x0A, 0x0B};
uint8_t data[32], mac[20], macCalculated[20], addr[4], keyA[6], sector, *hash, shaData[9];
int userIndex;
bool success;

void setup() {
  wdt_enable(WDTO_8S);

  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);

  Serial.begin(115200);       //init serial
  //while (!Serial);
  db.open(0);                 //init database
  pinMode(triggerPin, INPUT); //set triggerPin to input to not set state high or low

  Serial.println("Looking for PN532...");
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find PN53x board");
    exit(0); // Die
  }
  nfc.SAMConfig();

  Serial.println("INIT DONE");
}

void loop() {
  wdt_reset();

  switch (state) {
    case STATE_VERIFY:
      if (!getKeyCode(nfc, addr)) break; // Check if there is a valid ibutton available and read it
      memcpy(user.uid, addr, 4);
      userIndex = getUser(db, user);
      if(userIndex < 0) {state = STATE_DENIED; break;} // Check if user exists in the database and fetch the secret

      Serial.print("addr: ");
      hexdump(addr, 8);
      Serial.print("uid: ");
      hexdump(user.uid, 4);
      Serial.print("secret: ");
      hexdump(user.secret, 8);
      Serial.print("flags: ");
      hexdump(user.flags, 1);

      sector = random(0, 16);
      memcpy(shaData, user.secret, 8);
      shaData[8] = sector;
      Sha1.init();
      for (int i = 0; i < 8; i++)
      {
        Sha1.print(formatHexString(shaData[i]));
      }
      Sha1.print(shaData[8]);
      hash = Sha1.result();

      memcpy(keyA, hash, 6);
      success = nfc.mifareclassic_AuthenticateBlock(user.uid, 4, sector*4, 0, keyA);
      
      if (success)
      {
        uint8_t data[16];
        success = nfc.mifareclassic_ReadDataBlock(sector*4+1, data);
        if (success)
        {
          if(compare(hash + 6, data, 14))
          {
            state = STATE_GRANTED;
          }
          else
          {
            state = STATE_DENIED;
            Serial.println("Wrong hash!");
          }
        }
        else
        {
          state = STATE_DENIED;
        }
      }
      else
      {
        state = STATE_DENIED;
      }

      if(!(user.flags&0b10000000)) {state = STATE_CHILD; break;} // Check if the user is a adult
      if(user.flags&0b01000000) {state = STATE_ADMIN; break;} // Check if the user is a admin
      break;
    case STATE_GRANTED:
      Serial.println("STATE_GRANTED");
      pinMode(triggerPin, OUTPUT);
      digitalWrite(triggerPin, LOW);
      delay(1000);
      pinMode(triggerPin, INPUT);
      state = STATE_VERIFY;
      break;
    case STATE_CHILD:
      Serial.println("STATE_CHILD");
      delay(1000);
      state = STATE_VERIFY;
      break;
    case STATE_DENIED:
      Serial.println("STATE_DENIED");
      delay(1000);
      state = STATE_VERIFY;
      break;
    case STATE_ADMIN:
      Serial.println("STATE_ADMIN");
      delay(1000);
      state = STATE_VERIFY;
      break;
  }
}

String formatHexString(int hexInt)
{
  String hexString =  String(hexInt, HEX);
  if (hexInt < 17)
  {
    hexString = '0' + hexString;
  }
  return hexString;
}
