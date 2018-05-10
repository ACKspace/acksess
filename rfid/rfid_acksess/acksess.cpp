#include "acksess.h"
#include "Arduino.h"
#include <Wire.h>
#include <PN532.h>
#include <PN532_I2C.h>

bool getKeyCode(PN532 nfc, byte addr[8]){
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &addr[0], &uidLength);
  
  if (success && uidLength == 4) {
    return true;
  }
  else
  {
    return false;
  }
}

int getUser(DB db, User &user) {
  User dbUser;
  for(int i = 1; i <= db.nRecs(); i++) {
    db.read(i, DB_REC dbUser);
    if(compare(dbUser.uid, user.uid, 4)) {
      memcpy(user.secret, dbUser.secret, 8);
      user.flags = dbUser.flags;
      return i;
    }
  }
  return -1;
}

bool updateUser(DB db, User &user, int index) {
  db.write(index, DB_REC user);
}

void hexdump(byte* string, int size) {
  for (int i = 0; i < size; i++) {
    Serial.print(string[i] >> 4, HEX);
    Serial.print(string[i] & 0xF, HEX);
  }
  Serial.println();
}

bool compare( const uint8_t one[], const uint8_t two[], const uint8_t length ) {
  bool equal = 1;
  for (uint8_t i = 0; (i < length) && equal; i++) {
    equal = (one[i] == two[i]);
  }
  return equal;
}
