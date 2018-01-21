#include <OneWire.h>
#include <EEPROM.h>
#include "ds1961.h"
#include "ds1961_sha.h"
#include "DB.h"
#include "acksess.h"

OneWire ds(2);
DS1961  sha(&ds);
DB db;

User user;
uint8_t state;

unsigned char page = {0x00};
byte challenge[3] = {0x00, 0x00, 0x00};
uint8_t data[32], mac[20], macCalculated[20];

void setup() {
  Serial.begin(115200);       //init serial
  while (!Serial);
  db.open(0);                 //init database
  Serial.println("INIT DONE");
  user.flags = 0b10000000;
}

void loop() {
  switch (state) {
    case STATE_VERIFY:
      if (!getKeyCode(ds, user.uid)) break; // Check if there is a valid ibutton available and read it
      sha.WriteSecret(user.uid, user.secret); // TODO: Fetch the secret from the DB
      // TODO: Generate secret
      if (!sha.ReadAuthWithChallenge(user.uid, page * 32, challenge, data, mac)) break; // Send the challenge to the ibutton
      calcmac_readauthpage(macCalculated, page * 32, data, user.secret, challenge, user.uid); // Calculate what the secret should be
      if(!compare(mac, macCalculated, 20)) {state = STATE_DENIED; break;} // Check is the two MACs match
      if((user.flags&0b10000000) == 0) {state = STATE_CHILD; break;} // Check if the user is a adult
      if((user.flags&0b01000000) > 0) {state = STATE_ADMIN; break;} // Check if the user is a admin
      state = STATE_GRANTED; // The user passed all checks so should be granted access
      break;
    case STATE_GRANTED:
      Serial.println("STATE_GRANTED");
      delay(1000);
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

bool compare( const uint8_t one[], const uint8_t two[], const uint8_t length ) {
  bool equal = 1;
  for (uint8_t i = 0; (i < length) && equal; i++) {
    equal = (one[i] == two[i]);
  }
  return equal;
}
