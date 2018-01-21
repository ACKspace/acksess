#include <OneWire.h>
#include <EEPROM.h>
#include "ds1961.h"
#include "ds1961_sha.h"
#include "DB.h"
#include "acksess.h"

OneWire ds(2);
DS1961  sha(&ds);
DB db;

byte addr[8];
String keyStatus="";

void setup() {
  Serial.begin(115200);       //init serial
  db.open(1);                 //init database
  Serial.println("INIT DONE");
}

void loop() {
  keyStatus = getKeyCode(ds, addr);
  if(keyStatus=="ok"){
    for(byte i=5;i>0;i--){
      Serial.print(":");
      Serial.print(addr[i], HEX);
    }
    Serial.println("");
    unsigned char page[1];
    char challenge[3];
    uint8_t data[32];
    uint8_t mac[20];
    if (! ibutton_challenge(page[0], (byte*) challenge, data[32], mac[20], sha)) {
      Serial.println("CHALLENGE ERROR");
      //if (!ds.reset()) error();
      return;
    }
  }
  else if(keyStatus!="") Serial.println(keyStatus);
  delay(1000);
}

