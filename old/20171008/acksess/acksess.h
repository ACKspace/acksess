#ifndef Acksess_h
#define Acksess_h

#include "Arduino.h"

class Acksess {
  public:
    struct User;
    void getKeyCode();
    bool ibutton_challenge(byte page, byte* challenge);
    void hexdump(byte* string, int size);
};

#endif
