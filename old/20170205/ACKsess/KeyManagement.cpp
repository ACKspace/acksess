#include "EEPROMAnything.h"
#include "ds1961.h"
#include "ds1961_sha.h"
#include "KeyManagement.h"
#include "WriteMessage.h"

#define BUTTON_LENGTH 8

// Since I don't know who owns what key (administration!), I add the keys I know
PROGMEM const byte masterButtons[][BUTTON_LENGTH] = {
  { 0x33, 0xbb, 0x6c, 0x25, 0x05, 0x00, 0x00 }, // 000005256CBB: Da Syntax
  { 0x33, 0xd5, 0x63, 0x25, 0x05, 0x00, 0x00 }, // 0000052563D5: Prodigity
  { 0x33, 0xad, 0x6d, 0x25, 0x05, 0x00, 0x00 }, // 000005256DAD: Vicarious
  { 0x33, 0x1f, 0x5f, 0x25, 0x05, 0x00, 0x00 }  // 000005255F1F: xopr
};

PROGMEM const byte allowedButtons[][BUTTON_LENGTH] = {
/*{ 0x33, 0x57, 0x66, 0x25, 0x05, 0x00, 0x00 }, // 000005256657: **NIET UITGEGEVEN**/
/*{ 0x33, 0x75, 0x5f, 0x25, 0x05, 0x00, 0x00 }, // 000005255F75: **NIET UITGEGEVEN (stuiterveer oud)**/
/*{ 0x33, 0x1b, 0x5a, 0x25, 0x05, 0x00, 0x00 }, // 000005255A1B: eagle00789, cool-down starting april 11 2015*/

  { 0x33, 0x3a, 0x58, 0x25, 0x05, 0x00, 0x00, 0x1e } // 1E000525583A33: aes256cbc
/*  
  { 0x33, 0x16, 0x5d, 0x25, 0x05, 0x00, 0x00 }, // 000005255D16: Cloud
  { 0x33, 0xb9, 0x64, 0x25, 0x05, 0x00, 0x00 }, // 0000052564B9: Computer1up
  { 0x33, 0xc2, 0x67, 0x25, 0x05, 0x00, 0x00 }, // 0000052567C2: CoolePascal
  { 0x33, 0x76, 0x67, 0x25, 0x05, 0x00, 0x00 }, // 000005256776: stuiterveer, new
  { 0x33, 0x9d, 0x64, 0x25, 0x05, 0x00, 0x00 }, // 00000525649D: =PsychiC=
  { 0x33, 0x78, 0x6d, 0x25, 0x05, 0x00, 0x00 }, // 000005256D78: Roelke, new
  { 0x33, 0x12, 0x6f, 0x25, 0x05, 0x00, 0x00 }, // 000005256F12: TheOnlyJoey
  { 0x33, 0x45, 0x58, 0x25, 0x05, 0x00, 0x00 }, // 000005255845: Valentijn
  { 0x33, 0x20, 0x6f, 0x25, 0x05, 0x00, 0x00 }, // 000005256F20: wirexbox
  { 0x33, 0x7c, 0x6a, 0x25, 0x05, 0x00, 0x00 }  // 000005256A7C: Zane
*/
/*{ 0x33, 0xae, 0x67, 0x25, 0x05, 0x00, 0x00 }, // 0000052567AE: Jetse (als experimenteersleutel)*/
/*{ 0x33, 0xaf, 0x63, 0x25, 0x05, 0x00, 0x00 }, // 0000052563AF: aes256cbc oud: ingeleverd: kwijt*/
/*{ 0x33, 0xe6, 0x53, 0x25, 0x05, 0x00, 0x00 }, // 0000052553E6: Danny_W*/
/*{ 0x33, 0x4b, 0x54, 0x25, 0x05, 0x00, 0x00 }, // 00000525544B: Roelke, lost around december 2015 or earlier*/
/*{ 0x33, 0xed, 0x59, 0x25, 0x05, 0x00, 0x00 }, // 0000052559ED: stuiterveer, lost around november 15 2015*/
/*{ 0x33, 0x62, 0x66, 0x25, 0x05, 0x00, 0x00 }, // 000005256662: **Vicarious, verloren*/
};

PROGMEM const byte disallowedButtons[][BUTTON_LENGTH] = {
  { 0x33, 0xd2, 0x5e, 0x25, 0x05, 0x00, 0x00 }  // 000005255ED2: Amazing Mike
};

void writeEeprom( ) {
  uid_t uid;
  for( uint8_t i = 0; i < ( sizeof( allowedButtons ) / BUTTON_LENGTH ); i++ ) {
    uid.id     = allowedButtons[i];
    // Hier zijn we gebleven.
  }
}

uint8_t getKeyCode(OneWire ow, uint8_t* addr )
{
  uint8_t hasKeyCode = false;

  if ( !ow.search( addr ) )
  {
    ow.reset_search( );
  } 
  else if ( OneWire::crc8( addr, 7 ) != addr[ 7 ] )
  {
    // CRC invalid;
  } else {
    ow.reset( );
    hasKeyCode = true;
  }

  return hasKeyCode;
}

static int findKey( const byte* _button, uid_t* uid )
{
  int index = -1;

  // Iterate the memory and lookup the key
  for ( int nMemory = 0; nMemory <= ( E2END / sizeof( uid_t ) ); nMemory++ )
  {
    EEPROM_readAnything( nMemory, uid );
    if (compareArray(_button, uid->id, sizeof(uid->id))) {
      index = nMemory;
      break;
    }
  }

  if (index < 0) {
    uid = 0;
  }

  return index;  
}

boolean authenticateKey( DS1961 ds, byte* _button, bool _includeDisAllowed, bool _includeEeprom, bool _includeAllowed, bool _includeMaster )
{
  boolean authenticate = false;
  uid_t* uid;
  
  if ( findKey( _button, uid ) >= 0 ) 
  {
    
    
    /*
    if ( _includeMaster )
    {
      for( byte i = 0; i < (sizeof( masterButtons ) / 6); i++)
      {
        // Match key
        if( _button[ 1 ] == masterButtons[ i ][ 1 ]
         && _button[ 2 ] == masterButtons[ i ][ 2 ]
         && _button[ 3 ] == masterButtons[ i ][ 3 ]
         && _button[ 4 ] == masterButtons[ i ][ 4 ] )
          return true;
      }
    }
  
    if ( _includeDisAllowed )
    {
        for( byte i = 0; i < (sizeof( disallowedButtons ) / 6); i++)
        {
          // Match key
          if( _button[ 1 ] == disallowedButtons[ i ][ 1 ]
           && _button[ 2 ] == disallowedButtons[ i ][ 2 ]
           && _button[ 3 ] == disallowedButtons[ i ][ 3 ]
           && _button[ 4 ] == disallowedButtons[ i ][ 4 ] )
            return true;
        }
    }
    */
  }

  return authenticate;
}

static uint8_t compareArray( uint8_t first[], uint8_t second[], uint8_t length ) {
  uint8_t compare = 1;
  for( uint8_t i = 0; ( i < length ) && compare; i++ ) {
    if ( first[i] != second[i] ) {
      compare = 0;
    }
  }
  return compare;
}


bool getKey( const int& _nIndex, byte* _key )
{
  bool bValid = false;
  _key[ 0 ] = 0xFF;
  if ( ( _key[ 1 ] = EEPROM.read( (_nIndex << 2) + 3) ) != 0xff )
    bValid = true;
  if ( ( _key[ 2 ] = EEPROM.read( (_nIndex << 2) + 2 ) ) != 0xff )
    bValid = true;
  if ( ( _key[ 3 ] = EEPROM.read( (_nIndex << 2) + 1 ) ) != 0xff )
    bValid = true;
  if ( ( _key[ 4 ] = EEPROM.read( (_nIndex << 2) + 0 ) ) != 0xff )
    bValid = true;
  _key[ 5 ] = 0xFF;  

  return bValid;
}

void printKey( const int& _nIndex )
{
  byte key[ 8 ];
  getKey( _nIndex, key );
  printKey( key );
}

void printKey( const byte* _button )
{
  String message = "";
  for(byte i = 0; i < BUTTON_LENGTH; i++) {
    message += String( _button[ i ], HEX );
    if (i < (BUTTON_LENGTH - 1)) {
      message += ":";
    }
  }
  writeMessage(message);
}

void addKey( const byte* _button )
{
}

void deleteKey( const byte* _button )
{
}
