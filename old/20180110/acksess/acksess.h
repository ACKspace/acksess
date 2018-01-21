#ifndef Acksess_h
#define Acksess_h

#include "Arduino.h"
#include <OneWire.h>
#include "ds1961.h"

struct User {
  byte uid[6];        //6 bytes of id
  byte secret[3];     //3 bytes secret
  uint8_t flags;      // 0xxxxxxx = Is not aduld
                      // x0xxxxxx = Is not admin
};

struct user;
extern String getKeyCode(OneWire ds, byte addr[8]);
bool ibutton_challenge(byte page, byte* challenge, uint8_t data[32], uint8_t mac[20], DS1961 sha);
void hexdump(byte* string, int size);

#endif
