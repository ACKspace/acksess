#include <OneWire.h>
#include <EEPROM.h>
#include "key.h"
#include "hexdump.h"

static OneWire onewire( 2 );
static DS1961 ds( &onewire );

void setup( )
{
	Serial.begin( 115200 );
	Serial.println( "Reset" );
	delay( 1000 );

	writeEeprom( );
}

void loop( )
{
	normalMode( );
	delay( 1000 );
}

void normalMode( )
{
	uint8_t id[ 8 ];
	if ( readKey( onewire, id ) ) {
		if ( authenticateId( ds, id ) )
		{
			hexdump( id, ID_LENGTH );
		}
	}
	for( uint8_t i = 0; i < 40; i++ ) {
		Serial.print( EEPROM.read( i ), HEX );
		Serial.print( " " );
	}
	Serial.println( );
}

void adminMode( )
{
}
