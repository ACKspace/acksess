#include <OneWire.h>
#include <EEPROM.h>
#include "ds1961.h"
#include "ds1961_sha.h"
#include "DB.h"
#include "acksess.h"

OneWire ds(2);
DS1961  sha(&ds);
DB db;

String keyStatus = "";
User user;

void setup() {
  Serial.begin(115200);       //init serial
  while (!Serial);
  db.open(0);                 //init database
  Serial.println("INIT DONE");
}

void loop() {
  keyStatus = getKeyCode(ds, user.uid);
  if (keyStatus == "ok") {
    unsigned char page = {0x00};
    byte challenge[3] = {0x00, 0x00, 0x00};
    uint8_t data[32], mac[20], secret[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    sha.WriteSecret(user.uid, secret);
    if (sha.ReadAuthWithChallenge(user.uid, page * 32, challenge, data, mac)) {
      uint8_t macCalculated[20];
      calcmac_readauthpage(macCalculated, page * 32, data, secret, challenge, user.uid);
      Serial.println(compare(mac, macCalculated, 20));
      Serial.print("<");
      hexdump(data, 32);
      Serial.print(" ");
      hexdump(mac, 20);
      Serial.print(" ");
      hexdump(macCalculated, 20);
      Serial.println(">");
    } else {
      Serial.println("CHALLENGE ERROR");
      return;
    }
  }
  else if (keyStatus != "") Serial.println(keyStatus);
  delay(1000);
}

bool compare( const uint8_t one[], const uint8_t two[], const uint8_t length ) {
  bool equal = 1;
  for(uint8_t i = 0; (i < length) && equal; i++) {
    equal = (one[i] == two[i]);
  }
  return equal;
}
