#include <OneWire.h>
#include "ds1961.h"
#include "hexutil.c"

OneWire ds(2);
DS1961  sha(&ds);

byte addr[8];
String keyStatus="";

void setup() {
  Serial.begin(115200);
}

void loop() {
  getKeyCode();
  if(keyStatus=="ok"){
    for(byte i=5;i>0;i--){
      Serial.print(":");
      Serial.print(addr[i], HEX);
    }
    Serial.println("");
    unsigned char page[1];
    char challenge[3];
    if (! ibutton_challenge(page[0], (byte*) challenge)) {
      Serial.println("CHALLENGE ERROR");
      //if (!ds.reset()) error();
      return;
    }
  }
  else if(keyStatus!="") Serial.println(keyStatus);
  delay(1000);
}

void getKeyCode(){
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

bool ibutton_challenge(byte page, byte* challenge) {
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

void hexdump(byte* string, int size) {
  for (int i = 0; i < size; i++) {
    Serial.print(string[i] >> 4, HEX);
    Serial.print(string[i] & 0xF, HEX);
  }
}
