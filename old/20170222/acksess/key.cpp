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
	for( uint8_t i = 0; i <= 20; i++ )
	{
		EEPROM.write( i, 0xFF );
	}

	uint8_t id[ ] = { 0x33, 0x3A, 0x58, 0x25, 0x05, 0x00, 0x00, 0x1E };
	uid_t uid = constructUid( id );
	uid.position = 2;
	writeUid( &uid );
}

static uint8_t randomByte()
{
	uint8_t randomByte = (uint8_t) random( 0xFF );
	return randomByte;
}

static uid_t constructUid( const uint8_t* id )
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
	/*
	for( uint8_t i = 0; i < sizeof( uid.secret ); i++ )
	{
		uid.secret[ i ] = randomByte( ); 
	}
	*/
	uid.secret[ 0 ] = 0x11;
	uid.secret[ 1 ] = 0x22;
	uid.secret[ 2 ] = 0x33;
	uid.secret[ 3 ] = 0x44;
	uid.secret[ 4 ] = 0x55;
	uid.secret[ 5 ] = 0x66;
	uid.secret[ 6 ] = 0x77;
	uid.secret[ 7 ] = 0x89;

	return uid;
}

uint8_t isValidUid ( const uid_t* uid )
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

uint8_t isValidUidPosition( const uint16_t position )
{
	uint8_t	isValidPosition = ( (position >= 1) && ( position <= maxUidPosition( ) ) );
	return isValidPosition;
}

uint16_t maxUidPosition( )
{
	uint16_t maxUidPosition = E2END / ( sizeof( uid_t ) - sizeof( uint16_t ) );
	return maxUidPosition;
}

uint8_t readUid ( const uint16_t position, uid_t* uid )
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

static uint16_t getEepromPosition( const uid_t* uid )
{
	uint16_t position = ( uid->position - 1 ) * ( sizeof( uid_t ) - sizeof( uid->position ) );
	return position;
}

static uint8_t writeFlags( const uid_t* uid )
{
	uint8_t written = 0;

	uint16_t position = getEepromPosition( uid );
	Serial.print( "Position" ); Serial.println( uid->position );
	EEPROM.write( position, uid->flags );

	return written;
}

uint8_t writeUid( const uid_t* uid )
{
	uint8_t written = 0;

	if ( isValidUid( uid ) ) {
		// EEPROM write position.
		uint16_t position = getEepromPosition( uid );

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

uint8_t findUid( const uint8_t* id, uid_t* uid )
{
	uint8_t found = 0;
	
	for( uint8_t pos = 1; ( pos <= maxUidPosition ) && !found; pos++ )
	{
		found = ( readUid( pos, uid ) && compare( uid->id, id, ID_LENGTH ) );
	}

	return found;
}

static uint8_t compare(const uint8_t one[], const uint8_t two[], const uint8_t length) {
	uint8_t compare = 1;
	for(uint8_t i = 0; (i < length) && compare; i++) {
	    compare = (one[i] == two[i]);
	}
	return compare;
}

#define FLAG_INDEX_SECRET_WRITTEN 0

static uint8_t getFlag( const uid_t uid, const uint8_t index )
{
	uint8_t flag = ( ( ( 1 << index ) ) & uid.flags );
	flag = ! flag;
	return flag;
}
static uint8_t setFlag( uid_t* uid, const uint8_t index )
{
	uint8_t a = ( 1 << index );
	a = ~a;
	uid->flags = uid->flags & a;

	uint8_t flag = getFlag( *uid, index );
	return flag;
}

static uint8_t hasSecretBeenWritten( const uid_t uid )
{
	uint8_t written = getFlag( uid, FLAG_INDEX_SECRET_WRITTEN );
	return written;
}
static uint8_t setSecretBeenWritten( uid_t* uid )
{
	setFlag( uid, FLAG_INDEX_SECRET_WRITTEN );
}
static uint8_t writeSecret ( DS1961 ds, uid_t* uid )
{
	uint8_t written = 0;

	if ( !hasSecretBeenWritten( *uid ) )
	{
		Serial.println( "Write secret" );
		if ( ds.WriteSecret(uid->id, uid->secret ) )
		{
			setSecretBeenWritten( uid );
			writeFlags( uid );
		}
	}	

	return written;
}

uint8_t authenticateId( DS1961 ds, const uint8_t* id )
{
	uint8_t authenticate = 0;

	uid_t uid;
	if ( findUid( id, &uid ) )
	{
		authenticate = authenticateUid( ds, uid );
	}

	return authenticate;
}	
uint8_t authenticateUid( DS1961 ds, const uid_t uid )
{
	uint8_t authenticate = 0;

	writeSecret( ds, &uid );

	uint8_t challenge[3] = { randomByte( ), randomByte( ), randomByte( ) };
	uint8_t data[32];
	uint8_t macDS[20];
	if ( ds.ReadAuthWithChallenge( uid.id, 0, challenge, data, macDS ) )
	{
		//hexdump( macDS, 20 );

		uint8_t macCalculated[20];
		uint8_t address = 0;
		calcmac_readauthpage(macCalculated, address, data, uid.secret, challenge, uid.id);
		//hexdump( macCalculated, 20 );

		authenticate = compare(macDS, macCalculated, 20);
	}

	return authenticate;
}


