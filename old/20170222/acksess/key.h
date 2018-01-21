#include <OneWire.h>
#include "ds1961.h"
#include "ds1961_sha.h"

#define ID_LENGTH 8
#define SECRET_LENGTH 8
typedef struct uid_t {
	uint16_t position;
	uint8_t flags;
	uint8_t id[ ID_LENGTH ];
	uint8_t secret[ SECRET_LENGTH ];
};

uint8_t readKey( OneWire onewire, uint8_t* id );
void writeEeprom( );
static uid_t constructUid( const uint8_t* id );
uint8_t isValidUid ( const uid_t* uid );
uint8_t isValidUidPosition ( const  uint16_t position );
uint16_t maxUidPosition( );
uint8_t readUid ( const uint16_t position, uid_t* uid );
uint8_t findUid( const uint8_t* id, uid_t* uid );
uint8_t authenticateId( DS1961 ds, const uint8_t* id);
uint8_t authenticateUid( DS1961 ds, const uid_t uid );
uint8_t writeUid( const  uid_t* uid );
static uint8_t compare( const uint8_t one[], const uint8_t two[], const uint8_t length);
static uint8_t randomByte();
