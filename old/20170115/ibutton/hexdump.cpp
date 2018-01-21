#include <stdint.h>
#include <Arduino.h>

static void originalHexdump(uint8_t *data, int size, int modulo)
{
    int i, len;
    int addr = 0;
    uint8_t b;

    while (size > 0) {
        if ((modulo > 0) && (size > modulo)) {
            len = modulo;
        } else {
            len = size;
        }
        size -= len;

        for (i = 0; i < len; i++) {
            b = data[addr++];
            Serial.print((b >> 4) & 0xF, HEX);
            Serial.print((b >> 0) & 0xF, HEX);
        }
    }
}

void hexdump(uint8_t* data, uint8_t size)
{
  originalHexdump(data, size, 0);  
  Serial.println();
}

