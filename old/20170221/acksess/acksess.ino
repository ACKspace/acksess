#include "key.h"
//#include "hexdump.h"

#include <OneWire.h>
#include <EEPROM.h>

static OneWire onewire( 2 );

void setup( )
{
	Serial.begin( 9600 );
	Serial.println( "Reset" );
	delay( 1000 );

	writeEeprom( );
	uid_t uid;
	readUid( 1, &uid );
	for( uint8_t i = 0; i < sizeof( uid.id ); i++ )
	{
		Serial.print( i ); Serial.print( " " ); Serial.println( uid.id[ i ], HEX );
	}
	delay( 1000 );
	/*
	for( uint8_t i = 0; i < 20; i++ )
	{
		Serial.println( EEPROM.read( i ), HEX );
	}
	*/
	uid_t uidSearch;
	if ( findUid( uid.id, &uidSearch ) )
	{
		for( uint8_t i = 0; i < sizeof( uidSearch.id ); i++ )
		{
			Serial.print( i ); Serial.print( " " ); Serial.println( uidSearch.id[ i ], HEX );
		}
	}
}

void loop( )
{
	normalMode( );
}

void normalMode( )
{
	uint8_t id[ 8 ];
	/*
	onewire.reset_search();
	while( !onewire.search( id ) ) { }
	onewire.reset( );
	*/
	if ( readKey( onewire, id ) ) {
		for( uint8_t i = 0; i < ID_LENGTH; i++ )
		{
			Serial.print( id [ i ], HEX );
			if ( i < ID_LENGTH - 1 )
			{
				Serial.print( ":" );
			}
		}
		Serial.println( );
		delay(1000);
	}
	delay( 1000 );
}

void adminMode( )
{
}
