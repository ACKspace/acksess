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
byte challenge[3] = {0x09, 0x0A, 0x0B};
uint8_t data[32], mac[20], macCalculated[20];
uint8_t addr[8];
int userIndex;

void setup() {
  Serial.begin(115200);       //init serial
  while (!Serial);
  db.open(0);                 //init database
  Serial.println("INIT DONE");
}

void loop() {
  switch (state) {
    case STATE_VERIFY:
      if (!getKeyCode(ds, addr)) break; // Check if there is a valid ibutton available and read it
      memcpy(user.uid, addr, 6);
      userIndex = getUser(db, user);
      if(userIndex < 0) {state = STATE_DENIED; break;} // Check if user exists in the database and fetch the secret

      Serial.print("addr: ");
      hexdump(addr, 8);
      Serial.print("uid: ");
      hexdump(user.uid, 6);
      Serial.print("secret: ");
      hexdump(user.secret, 8);
      Serial.print("flags: ");
      hexdump(user.flags, 1);

      if(user.secret[0] == 0x00){
        Serial.println("Updating secret!");
        // TODO: Generate secret
        uint8_t secret[8] = {0x56, 0x56, 0x96, 0x56, 0x56, 0x55, 0x76, 0x92};
        memcpy(user.secret, secret, 8);
        sha.WriteSecret(addr, user.secret);
        updateUser(db, user, userIndex);
      }

      
      // TODO: Generate challenge
      if (!sha.ReadAuthWithChallenge(addr, page * 32, challenge, data, mac)) break; // Send the challenge to the ibutton
      calcmac_readauthpage(macCalculated, page * 32, data, user.secret, challenge, addr); // Calculate what the secret should be
      if(!compare(mac, macCalculated, 20)) {state = STATE_DENIED; break;} // Check is the two MACs match
      if(!(user.flags&0b10000000)) {state = STATE_CHILD; break;} // Check if the user is a adult
      if(user.flags&0b01000000) {state = STATE_ADMIN; break;} // Check if the user is a admin
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
