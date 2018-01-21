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

bool ibutton_challenge(byte page, byte* challenge, uint8_t data[32], uint8_t mac[20], DS1961 sha) {
  if (! sha.ReadAuthWithChallenge(NULL, page * 32, challenge, data, mac)) {
    return false;
  }
  Serial.print("<");
  hexdump(data, 32);
  Serial.print(" ");
  hexdump(mac, 20);
  Serial.println(">");  
  return true;
}

bool getUser(DB db, User &user) {
  User dbUser;
  for(int i = 1; i <= db.nRecs(); i++) {
    db.read(i, DB_REC dbUser);
    if(compare(dbUser.uid, user.uid, 6)) {
      memcpy(user.secret, dbUser.secret, 3);
      user.flags = dbUser.flags;
      return true;
    }
  }
  return false;
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
