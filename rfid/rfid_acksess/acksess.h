#ifndef Acksess_h
#define Acksess_h

#include "Arduino.h"
#include <PN532.h>
#include "DB.h"

struct User {
  byte uid[6];        //6 bytes of id
  byte secret[8];     //8 bytes secret
  uint8_t flags;      // 1xxxxxxx = Is aduld
                      // x1xxxxxx = Is admin
};

#define STATE_VERIFY 0
#define STATE_GRANTED 1
#define STATE_CHILD 2
#define STATE_DENIED 3
#define STATE_ADMIN 4

struct user;
bool getKeyCode(PN532 nfc, byte addr[8]);
int getUser(DB db, User &user);
bool updateUser(DB db, User &user, int index);
bool compare( const uint8_t one[], const uint8_t two[], const uint8_t length );
void hexdump(byte* string, int size);

#endif
