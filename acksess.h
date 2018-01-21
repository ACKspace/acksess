#ifndef Acksess_h
#define Acksess_h

#include "Arduino.h"
#include <OneWire.h>
#include "ds1961.h"
#include "DB.h"

struct User {
  byte uid[6];        //6 bytes of id
  byte secret[3];     //3 bytes secret
  uint8_t flags;      // 1xxxxxxx = Is aduld
                      // x1xxxxxx = Is admin
};

#define STATE_VERIFY 0
#define STATE_GRANTED 1
#define STATE_CHILD 2
#define STATE_DENIED 3
#define STATE_ADMIN 4

struct user;
bool getKeyCode(OneWire ds, byte addr[8]);
bool ibutton_challenge(byte page, byte* challenge, uint8_t data[32], uint8_t mac[20], DS1961 sha);
bool getUser(DB db, User &user);
bool compare( const uint8_t one[], const uint8_t two[], const uint8_t length );
void hexdump(byte* string, int size);

#endif
