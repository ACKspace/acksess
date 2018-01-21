#include <OneWire.h>
#include "ds1961.h"

typedef struct uid_t {
  uint8_t flags;
  uint8_t id[8];
  uint8_t secret[8];
};

uint8_t getKeyCode( OneWire ow, uint8_t* addr );
boolean authenticateKey( DS1961 ds, byte* _button, bool _includeDisAllowed, bool _includeEeprom, bool _includeAllowed, bool _includeMaster );
bool getKey( const int& _nIndex, byte* _key );
void printKey( const byte* _button );
void printKey( const int& _nIndex );    
void addKey( const byte* _button );
void deleteKey( const byte* _nIndex );

static int findKey( const byte* _button, uid_t* uid );
static uint8_t compareArray(uint8_t first[], uint8_t second[], uint8_t length);  

