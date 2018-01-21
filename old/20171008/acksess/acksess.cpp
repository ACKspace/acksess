#include "Arduino.h"
#include "acksess.h"

struct Acksess::User {
  byte uid[6];        //6 bytes of id
  byte secret[3];     //3 bytes secret
  boolean isAdult;    //1 bit isAdult
  boolean isAdmin;    //1 bit isAdmin
} user;


void Acksess::getKeyCode(){
  byte present = 0;
  byte data[12];
  keyStatus = "";

  if(!ds.search(addr)){
    ds.reset_search();
    return;
  }

  if(OneWire::crc8(addr,7) != addr[7]){
    keyStatus="CRC invalid";
    return;
  }

  if(addr[0] != 0x33){
    keyStatus="not DS1961S";
    return;
  }
  keyStatus="ok";
  ds.reset();
}

bool Acksess::ibutton_challenge(byte page, byte* challenge) {
  uint8_t data[32];
  uint8_t mac[20];
  
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

void Acksess::hexdump(byte* string, int size) {
  for (int i = 0; i < size; i++) {
    Serial.print(string[i] >> 4, HEX);
    Serial.print(string[i] & 0xF, HEX);
  }
}
