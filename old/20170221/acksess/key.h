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
static uid_t constructUid( uint8_t* id );
uint8_t isValidUid ( uid_t* uid );
uint8_t authenticateId(uint8_t* id);
uint8_t isValidUidPosition ( uint16_t position );
uint16_t maxUidPosition( );
uint8_t readUid ( uint16_t position, uid_t* uid );
uint8_t findUid( uint8_t* id, uid_t* uid );
uint8_t authenticateUid( uid_t* uid );
uint8_t writeUid( uid_t* uid );
static uint8_t compare(uint8_t one[], uint8_t two[], uint8_t length);
