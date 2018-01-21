#ifndef Acksess_h
#define Acksess_h

#include "Arduino.h"
#include <OneWire.h>
#include "ds1961.h"

struct user;
extern String getKeyCode(OneWire ds, byte addr[8]);
bool ibutton_challenge(byte page, byte* challenge, uint8_t data[32], uint8_t mac[20], DS1961 sha);
void hexdump(byte* string, int size);

#endif
