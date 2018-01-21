#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "ds1961_sha.h"

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
static void ComputeSHAVM(const uint32_t MT[], uint32_t hash[])
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
static void HashToMAC(const uint32_t hash[], uint8_t MAC[])
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

// Debug the original calcmac_readauthpage bitshift code and the new implementation.
#define DEBUG_CALCMAC_READAUTHPAGE 0
// Big endian or little endian (conversion from (uint8_t*) to (uint32_t*)).
#define BYTE_ORDER_BACKWARDS 1

#if BYTE_ORDER_BACKWARDS
  #define DIFF (length - 1 - i)
#else
  #define DIFF (i)
#endif
#if(DEBUG_CALCMAC_READAUTHPAGE)
  #include "hexdump.h"
#endif
void calcmac_readauthpage(uint8_t mac[], uint8_t addr, uint8_t pp[], uint8_t ss[], uint8_t ch[], uint8_t id[])
{
#if(DEBUG_CALCMAC_READAUTHPAGE)
    uint32_t input[16];
#endif    
    uint8_t inputNew[64];
    uint8_t length;
    uint8_t i;
    
#if(DEBUG_CALCMAC_READAUTHPAGE)
    memset(input   , 0, 64);
    memset(inputNew, 0, 64); 
#endif

#if(DEBUG_CALCMAC_READAUTHPAGE)   
    input[0] = ((uint32_t)ss[0] << 24) | ((uint32_t)ss[1] << 16) | ((uint32_t)ss[2] << 8) | (uint32_t)ss[3];
#endif
    length = 4;
    for(i = 0; i < length; i++) {
      inputNew[DIFF + 0] = ss[i];
    }

#if(DEBUG_CALCMAC_READAUTHPAGE)      
    for(uint8_t i = 0; i < 32; i += 4) {
        input[i/4 + 1] = ((uint32_t)pp[i] << 24) | ((uint32_t)pp[i + 1] << 16) | ((uint32_t)pp[i + 2] << 8) | (uint32_t)pp[i + 3];
    }
#endif    
    length = 32;
    for (i = 0; i < length; i++) {
      inputNew[DIFF + 4] = pp[i];
    }

#if(DEBUG_CALCMAC_READAUTHPAGE)    
    input[9] = 0xFFFFFFFF;
#endif
    length = 4;
    for(i = 0; i < length; i++) {
      inputNew[DIFF + 36] = 0xFF;
    }
    
#if(DEBUG_CALCMAC_READAUTHPAGE)
    uint32_t mp = (8 << 3) | (((uint32_t)addr >> 5) & 7);
    input[10] = ((uint32_t)mp << 24) | ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | ((uint32_t)id[2]);
#endif
    length = 4;
    i = 0;
    inputNew[DIFF + 40] = (8 << 3) | ((addr >> 5) & 7);
    for(uint8_t j = 0; j < 3; j++) {
      i = j + 1;
      inputNew[DIFF + 40] = id[j];
    }

#if(DEBUG_CALCMAC_READAUTHPAGE)        
    input[11] = ((uint32_t)id[3] << 24) | ((uint32_t)id[4] << 16) | ((uint32_t)id[5] << 8) | (uint32_t)id[6];
#endif
    length = 4;
    for(i = 0; i < length; i++) {
      inputNew[DIFF + 44] = id[i + 3];
    }

#if(DEBUG_CALCMAC_READAUTHPAGE)    
    input[12] = ((uint32_t)ss[4] << 24) | ((uint32_t)ss[5] << 16) | ((uint32_t)ss[6] << 8) | (uint32_t)ss[7];
#endif
    length = 4;
    for(i = 0; i < length; i++) {
      inputNew[DIFF + 48] = ss[i + 4];
    }

#if(DEBUG_CALCMAC_READAUTHPAGE)       
    input[13] = ((uint32_t)ch[0] << 24) | ((uint32_t)ch[1] << 16) | ((uint32_t)ch[2] << 8) | 0x80;
#endif
    length = 4;
    for(i = 0; i < length; i++) {
      inputNew[DIFF + 52] = ch[i];
    }
    length = 4; 
    i = 3;
    inputNew[DIFF + 52] = 0x80;

#if(DEBUG_CALCMAC_READAUTHPAGE)    
    input[14] = 0;
#endif
    length = 4;
    for(i = 0; i < length; i++) {
      inputNew[DIFF + 56] = 0;
    }
 
#if(DEBUG_CALCMAC_READAUTHPAGE)   
    input[15] = 0x1B8;
#endif
    length = 4;
    i = 0; inputNew[DIFF + 60] = 0x00;
    i = 1; inputNew[DIFF + 60] = 0x00;
    i = 2; inputNew[DIFF + 60] = 0x01;
    i = 3; inputNew[DIFF + 60] = 0xB8;

#if(DEBUG_CALCMAC_READAUTHPAGE)    
    hexdump((uint8_t*) input, 64);
    hexdump((uint8_t*) inputNew, 64);
#endif

    uint32_t hash[16];
    ComputeSHAVM((uint32_t *) inputNew, hash);
    HashToMAC(hash, mac);
}

/*
void calcmac_copyscratchpad(uint8_t mac[], int addr, uint8_t pp[], uint8_t ss[], uint8_t sp[], uint8_t id[])
{
    uint32_t input[16];
    uint32_t hash[16];
    int i;

    input[0] = (ss[0] << 24) | (ss[1] << 16) | (ss[2] << 8) | ss[3];
    for (i = 0; i < 28; i += 4) {
        input[i/4 + 1] = (pp[i] << 24) | (pp[i + 1] << 16) | (pp[i + 2] << 8) | pp[i + 3];
    }
    input[8] = (sp[0] << 24) | (sp[1] << 16) | (sp[2] << 8) | sp[3];
    input[9] = (sp[4] << 24) | (sp[5] << 16) | (sp[6] << 8) | sp[7];
    uint8_t mp = (addr >> 5) & 7;
    input[10] = (mp << 24) | (id[0] << 16) | (id[1] << 8) | id[2];
    input[11] = (id[3] << 24) | (id[4] << 16) | (id[5] << 8) | id[6];
    input[12] = (ss[4] << 24) | (ss[5] << 16) | (ss[6] << 8) | ss[7];
    input[13] = 0xFFFFFF80;
    input[14] = 0;
    input[15] = 0x1B8;

    ComputeSHAVM(input, hash);
    HashToMAC(hash, mac);
}
*/
