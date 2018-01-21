#include "acksess.h"
#include "Arduino.h"
#include <OneWire.h>
#include "ds1961.h"

struct User {
  byte uid[6];        //6 bytes of id
  byte secret[3];     //3 bytes secret
  boolean isAdult;    //1 bit isAdult
  boolean isAdmin;    //1 bit isAdmin
} user;

String getKeyCode(OneWire ds, byte addr[8]){
  byte data[12];

  if(!ds.search(addr)){
    ds.reset_search();
    return "";
  }

  if(OneWire::crc8(addr,7) != addr[7]){
    return "CRC invalid";
  }

  if(addr[0] != 0x33){
    return "not DS1961S";
  }
  ds.reset();
  return "ok";
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

void hexdump(byte* string, int size) {
  for (int i = 0; i < size; i++) {
    Serial.print(string[i] >> 4, HEX);
    Serial.print(string[i] & 0xF, HEX);
  }
}
