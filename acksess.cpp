#include "acksess.h"
#include "Arduino.h"
#include <OneWire.h>
#include "ds1961.h"

bool getKeyCode(OneWire ds, byte addr[8]){
  byte data[12];

  if(!ds.search(addr)){
    ds.reset_search();
    return false;
  }

  if(OneWire::crc8(addr,7) != addr[7]){
    return false;
  }

  if(addr[0] != 0x33){
    return false;
  }
  ds.reset();
  return true;
}

int getUser(DB db, User &user) {
  User dbUser;
  for(int i = 1; i <= db.nRecs(); i++) {
    db.read(i, DB_REC dbUser);
    if(compare(dbUser.uid, user.uid, 6)) {
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

void admin(DB db) {
  Serial.print('A');
  while(true){
    while (!Serial.available());
    char command = Serial.read();
    switch (command) {
      case 'a':
        Serial.write(db.nRecs());
        User dbUser;
        for (int i = 1; i <= db.nRecs(); i++) {
          db.read(i, DB_REC dbUser);
          for(int ii = 0; ii < 6; ii++)
            Serial.write(dbUser.uid[ii]);
          Serial.write(dbUser.flags);
        }
        break;
      case 'b':
        Serial.println("Add user");
        User newRecord;
        byte newRecordBuffer[15];
        Serial.readBytes(newRecordBuffer, 15);
        memcpy(&newRecord, newRecordBuffer, 15);
        db.append(DB_REC newRecord);
        break;
      case 'c':
        Serial.println("Remove user");
        break;
      case 'd':
        Serial.println("Set flags");
        break;
      case 'e':
        Serial.println("Clear secret");
        break;
    }
  }
}

