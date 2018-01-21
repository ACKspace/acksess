#include "key.h"
#include "hexdump.h"
#include <EEPROM.h>

uint8_t readKey( OneWire onewire, uint8_t* id )
{
	uint8_t hasId = 0;

	if ( !onewire.search( id ) )
	{
		// No onewire device found.
		onewire.reset_search( );
	}
	else if ( OneWire::crc8( id, 7 ) != id[ 7 ] )
	{
		// CRC is invalid.
 	}
	else
	{
		// Onewire device succesfully detected.
		onewire.reset( );
		hasId = 1;
	}

	return hasId;
}

void writeEeprom( )
{
	uint8_t id[ ] = { 0x33, 0x3A, 0x58, 0x25, 0x05, 0x00, 0x00, 0x1E };
	uid_t uid = constructUid( id );
	uid.position = 1;
	writeUid( &uid );
}

static uid_t constructUid( uint8_t* id )
{
	uid_t uid;

	// Set flags.
	uid.flags = 0xFF;

	// Copy the given id to the given uid.
	for( uint8_t i = 0; i < sizeof ( uid.id ); i++ )
	{
		uid.id[ i ] = id[ i ];
	}

	// Generate a randomized secret for the given uid.
	for( uint8_t i = 0; i < sizeof( uid.secret ); i++ )
	{
		uid.secret[ i ] = random( 0xFF ); 
	}

	return uid;
}

uint8_t isValidUid ( uid_t* uid )
{
	// Check if position is greater than zero.
	uint8_t isValidUid = ( uid->position != 0 );

	if ( isValidUid )
	{
		// Check at least one of the bytes from the secret
		// do not contain unwritten EERPOM ( value 0xFF ).
		isValidUid = 0;
		for( uint8_t i = 0; ( i < sizeof( uid->id ) ) && !isValidUid; i++ )
		{
			if ( uid->id[ i ] != 0xFF )
			{
				isValidUid = 1;
			}
		}
	}

	return isValidUid;
}

uint8_t isValidUidPosition ( uint16_t position )
{
	uint8_t	isValidPosition = ( (position >= 1) && ( position <= maxUidPosition( ) ) );
	return isValidPosition;
}

uint16_t maxUidPosition( )
{
	uint16_t maxUidPosition = E2END / ( sizeof( uid_t ) - sizeof( uint16_t ) );
	return maxUidPosition;
}

uint8_t readUid ( uint16_t position, uid_t* uid )
{
	uint8_t read = 0;

	if ( isValidUidPosition ( position ) )
	{
		uint16_t pos = position - 1;

		// Read flags from EEPROM.
		uid->flags = EEPROM.read( pos );
		pos++;

		// Read id from EEPROM.
		for( uint8_t i = 0; i < sizeof( uid->id ); i++ )
		{
			uid->id[ i ] = EEPROM.read( pos );
			pos++;
		}

		// Read secret from EEPROM.
		for( uint8_t i = 0; i < sizeof( uid->secret ); i++ )
		{
			uid->secret[ i ] = EEPROM.read( pos );
			pos++;
		}

		// Set position.
		uid->position = position;

		read = ( isValidUid( uid ) );
	}

	return read;
}

uint8_t writeUid( uid_t* uid )
{
	uint8_t written = 0;

	if ( isValidUid( uid ) ) {
		// EEPROM write position.
		uint16_t position = ( uid->position - 1 ) * ( sizeof( uid ) - sizeof( uid->position ) );

		// Write flags.
		EEPROM.write( position, uid->flags );
		position++;

		// Write id.
		for( uint8_t i = 0; i < sizeof( uid->id ); i++ )
		{
			EEPROM.write( position, uid->id[ i ] );
			position++;
		}

		// Write secret.
		for( uint8_t i = 0; i < sizeof( uid->secret ); i++ )
		{
			EEPROM.write( position, uid->secret[ i ] );
			position++;
		}

		// Succesfully written.
		written = 1;
	}

	return written;
}

uint8_t findUid( uint8_t* id, uid_t* uid )
{
	uint8_t found = 0;
	
	for( uint8_t pos = 1; ( pos <= maxUidPosition ) && !found; pos++ )
	{
		found = ( readUid( pos, uid ) && compare( uid->id, id, ID_LENGTH ) );
	}

	return found;
}

static uint8_t compare(uint8_t one[], uint8_t two[], uint8_t length) {
	uint8_t compare = 1;
	for(uint8_t i = 0; (i < length) && compare; i++) {
	    compare = (one[i] == two[i]);
	}
	return compare;
}
	
uint8_t authenticateUid( uid_t* uid )
{
}


