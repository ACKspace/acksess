
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ds1961.h"
//#include "ds1961_sha.h"

static OneWire ow(2);
static DS1961 ds(&ow);

void hexdump(uint8_t *data, int size, int modulo)
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
    
    Serial.println();
}

//constants used in SHA computation
static const uint32_t KTN[4] = { 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };

// calculation used for the SHA MAC
static uint32_t NLF(uint32_t B, uint32_t C, uint32_t D, int n)
{
    if (n < 20)
        return ((B & C) | ((~B) & D));
    else if (n < 40)
        return (B ^ C ^ D);
    else if (n < 60)
        return ((B & C) | (B & D) | (C & D));
    else
        return (B ^ C ^ D);
}

//----------------------------------------------------------------------
// computes a SHA given the 64 byte MT digest buffer.  The resulting 5
// long values are stored in the given long array, hash.
//
// Note: This algorithm is the SHA-1 algorithm as specified in the
// datasheet for the DS1961S, where the last step of the official
// FIPS-180 SHA routine is omitted (which only involves the addition of
// constant values).
//
// 'MT'        - buffer containing the message digest
// 'hash'      - result buffer
//
void ComputeSHAVM(const uint32_t MT[], uint32_t hash[])
{
    uint32_t MTword[80];
    int i;
    uint32_t ShftTmp;
    uint32_t Temp;

    for (i = 0; i < 16; i++) {
        MTword[i] = MT[i];
    }

    for (; i < 80; i++) {
        ShftTmp = MTword[i - 3] ^ MTword[i - 8] ^ MTword[i - 14] ^ MTword[i - 16];
        MTword[i] = ((ShftTmp << 1) & 0xFFFFFFFE) | ((ShftTmp >> 31) & 0x00000001);
    }

    hash[0] = 0x67452301;
    hash[1] = 0xEFCDAB89;
    hash[2] = 0x98BADCFE;
    hash[3] = 0x10325476;
    hash[4] = 0xC3D2E1F0;

    for (i = 0; i < 80; i++) {
        ShftTmp = ((hash[0] << 5) & 0xFFFFFFE0) | ((hash[0] >> 27) & 0x0000001F);
        Temp = NLF(hash[1], hash[2], hash[3], i) + hash[4] + KTN[i / 20] + MTword[i] + ShftTmp;
        hash[4] = hash[3];
        hash[3] = hash[2];
        hash[2] = ((hash[1] << 30) & 0xC0000000) | ((hash[1] >> 2) & 0x3FFFFFFF);
        hash[1] = hash[0];
        hash[0] = Temp;
    }
}

//----------------------------------------------------------------------
// Converts the 5 long numbers that represent the result of a SHA
// computation into the 20 bytes (with proper byte ordering) that the
// SHA iButton's expect.
//
// 'hash'      - result of SHA calculation
// 'MAC'       - 20-byte, LSB-first message authentication code for SHA
//                iButtons.
//
void HashToMAC(const uint32_t hash[], uint8_t MAC[])
{
    uint32_t temp;
    int i, j, offset;

    //iButtons use LSB first, so we have to turn
    //the result around a little bit.  Instead of
    //result A-B-C-D-E, our result is E-D-C-B-A,
    //where each letter represents four bytes of
    //the result.
    for (j = 4; j >= 0; j--) {
        temp = hash[j];
        offset = (4 - j) * 4;
        for (i = 0; i < 4; i++) {
            MAC[i + offset] = (uint8_t) temp;
            temp >>= 8;
        }
    }
}

void calcmac_readauthpage(uint8_t mac[], uint8_t addr, uint8_t pp[], uint8_t ss[], uint8_t ch[], uint8_t id[])
{
    uint32_t input[16];
    uint32_t hash[16];
    uint8_t i;
  
    input[0] = ((uint32_t)ss[0] << 24) | ((uint32_t)ss[1] << 16) | ((uint32_t)ss[2] << 8) | (uint32_t)ss[3];
    for (i = 0; i < 32; i += 4) {
        input[i/4 + 1] = ((uint32_t)pp[i] << 24) | ((uint32_t)pp[i + 1] << 16) | ((uint32_t)pp[i + 2] << 8) | (uint32_t)pp[i + 3];
    }
    input[9] = 0xFFFFFFFF;
    uint32_t mp = (8 << 3) | (((uint32_t)addr >> 5) & 7);
    input[10] = ((uint32_t)mp << 24) | ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | ((uint32_t)id[2]);
    input[11] = ((uint32_t)id[3] << 24) | ((uint32_t)id[4] << 16) | ((uint32_t)id[5] << 8) | (uint32_t)id[6];
    input[12] = ((uint32_t)ss[4] << 24) | ((uint32_t)ss[5] << 16) | ((uint32_t)ss[6] << 8) | (uint32_t)ss[7];
    input[13] = ((uint32_t)ch[0] << 24) | ((uint32_t)ch[1] << 16) | ((uint32_t)ch[2] << 8) | 0x80;
    input[14] = 0;
    input[15] = 0x1B8;

    ComputeSHAVM(input, hash);
    HashToMAC(hash, mac);
}

uint8_t compareMacs(uint8_t mac1[], uint8_t mac2[]) {
  uint8_t compare = 1;
  uint8_t length = 20;  
  for(uint8_t i = 0; (i < length) && compare; i++) {
    if (mac1[i] != mac2[i]) {
      compare = 0;
    }
  }
  return compare;
}

void setup() {
  uint8_t serialDebug = 0;
  Serial.begin(9600);
  if (serialDebug) {
    Serial.println("reset");
  }  
  
  // Read id.
  uint8_t id[8];
  ow.reset_search();
  while(!ow.search(id)) {}
  
  if (serialDebug) {
    Serial.print("Found id: ");
    hexdump(id, 8, 0);
  }
  
  // Write secret.
  uint8_t secret[] = { // 8 byte };
  uint8_t writeSecret = 0;
  if (writeSecret) {
    if (ds.WriteSecret(id, secret)) {
      if (serialDebug) {
        Serial.print("Secret written: ");
        hexdump(secret, 8, 0);
      }
    }
  }
  
  // Read mac from iButton.
  randomSeed(analogRead(2));
  uint8_t challenge[3] = { random(0, 256), random(0, 256), random(0, 256) };
  uint8_t data[32];
  uint8_t macIButton[20];
  if (ds.ReadAuthWithChallenge(id, 0, challenge, data, macIButton)) {
    if (serialDebug) {
      hexdump(challenge, 3, 0);
      hexdump(data, 32, 0);
      hexdump(macIButton, 20, 0);
    }
  }
  
  // Calculate mac.
  uint8_t macCalculated[20];
  calcmac_readauthpage(macCalculated, 0, data, secret, challenge, id);
  if (serialDebug) {
    hexdump(macCalculated, 20, 0);
  }
  
  // Compare macs.
  compareMacs(macIButton, macCalculated) ? Serial.println("Key is valid") : Serial.println("Key is _NOT_ valid.");
}

void loop() {
}

