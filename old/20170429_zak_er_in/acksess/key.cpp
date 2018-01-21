#include <EEPROM.h>
#include "Entropy.h"
#include "key.h"
#include "serial.h"
#include "EntropyBuffer.h"

uint8_t readKey( OneWire onewire, uint8_t* id )
{
	uint8_t hasId = READ_KEY_NONE;

	if ( !onewire.search( id ) )
	{
		// No onewire device found.
		onewire.reset_search( );
	}
	else if ( OneWire::crc8( id, 7 ) != id[ 7 ] )
	{
		// CRC is invalid.
		hasId = READ_KEY_INVALID_CRC;
 	}
	else
	{
		// Onewire device succesfully detected.
		onewire.reset( );
		hasId = READ_KEY_SUCCESS;
	}

	return hasId;
}

void writeEeprom( )
{
	for( uint16_t i = 0; i <= E2END; i++ )
	{
		Serial.println( i );
		EEPROM.write( i, 0xFF );
	}

	uint8_t id[ ] = { 0x33, 0x3A, 0x58, 0x25, 0x05, 0x00, 0x00, 0x1E };
	uid_t uid = constructUid( id );
	uid.position = 60;
	setAdmin( &uid );
	writeUid( &uid );
}

static uint8_t randomByte()
{
	uint8_t randomByte = Entropy.randomByte( );
	return randomByte;
}

uid_t constructUid( const uint8_t* id )
{
	uid_t uid;

	// Copy the given id to the given uid.
	for( uint8_t i = 0; i < sizeof ( uid.id ); i++ )
	{
		uid.id[ i ] = id[ i ];
	}

	// Generate a randomized secret for the given uid.
	for( uint8_t i = 0; i < sizeof( uid.secret ); i++ )
	{
		uid.secret[ i ] = randomByte( ); 
	}

	return uid;
}

uint8_t isValidUid( const uid_t* uid )
{
	// Check if position is greater than zero.
	uint8_t isValid = ( uid->position != 0 );

	if ( isValid )
	{
		// Check at least one of the bytes from the secret
		// do not contain unwritten EERPOM ( value 0xFF ).
		isValid = 0;
		for( uint8_t i = 0; ( i < sizeof( uid->id ) ) && !isValid; i++ )
		{
			if ( uid->id[ i ] != 0xFF )
			{
				isValid = 1;
			}
		}
	}

	return isValid;
}

uint8_t isValidUidPosition( const uint16_t position )
{
	uint8_t	isValid = ( (position >= 1) && ( position <= maxUidPosition( ) ) );
	return isValid;
}

uint16_t minUidPosition( )
{
	return 1;
}

uint16_t maxUidPosition( )
{
	uint16_t maxUidPosition = E2END / ( sizeof( uid_t ) - sizeof( uint16_t ) );
	return maxUidPosition;
}

uint8_t readUid( const uint16_t position, uid_t* uid )
{
	uint8_t read = 0;

	if ( isValidUidPosition ( position ) )
	{
		// Set position.
		uid->position = position;

		// Get EEPROM position.
		uint16_t pos = getEepromPosition( position );

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

		read = ( isValidUid( uid ) );
	}

	return read;
}

static uint16_t getEepromPosition(const uint16_t position )
{
	uint16_t eepromPosition = ( position - 1 ) * ( sizeof( uid_t ) - sizeof( uint16_t ) );
	return eepromPosition;
}

static uint8_t writeFlags( const uid_t* uid )
{
	uint8_t written = 0;
	uint16_t position = getEepromPosition( uid->position );
	EEPROM.write( position, uid->flags );
	return written;
}

static uint16_t findAvailableUidPosition( )
{
	uint16_t availablePosition = 0;
	uid_t uid;
	
	for( uint16_t pos = minUidPosition( ); ( pos <= maxUidPosition( ) ) && !availablePosition; pos++ )
	{
		if ( ! readUid( pos, &uid ) )
		{
			availablePosition = pos;
		}
	}

	return availablePosition;
}

uint8_t writeUid( uid_t* uid )
{
	uint8_t written = 0;

	if ( ! isValidUidPosition( uid->position ) )
	{
		uid->position = findAvailableUidPosition( );
	}

	//uint16_t p = getUidSize( );
	//Serial.println( p );
	//Serial.print( "Position: " ); Serial.println( uid->position );

	if ( isValidUid( uid ) )
	{
		// EEPROM write position.
		uint16_t position = getEepromPosition( uid->position );

		// Write flags.
		//Serial.print( "EepromPosition: " ); Serial.println( position );
		EEPROM.write( position, uid->flags );
		position++;

		// Write id.
		for( uint8_t i = 0; i < sizeof( uid->id ); i++ )
		{
			//Serial.print( "EepromPosition: " ); Serial.println( position );
			EEPROM.write( position, uid->id[ i ] );
			position++;
		}

		// Write secret.
		for( uint8_t i = 0; i < sizeof( uid->secret ); i++ )
		{
			//Serial.print( "EepromPosition: " ); Serial.println( position );
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
	uint16_t maxPosition = maxUidPosition( );
	
	for( uint16_t pos = 1; ( pos <= maxPosition ) && !found; pos++ )
	{
		found = ( readUid( pos, uid ) && compare( uid->id, id, ID_LENGTH ) );
	}

	return found;
}

uint8_t compareUid( const uid_t uid1, const uid_t uid2 )
{
	uint8_t equal = compare( uid1.id, uid2.id, ID_LENGTH );
	return equal;
}

static uint8_t compare( const uint8_t one[], const uint8_t two[], const uint8_t length ) {
	uint8_t equal = 1;
	for(uint8_t i = 0; (i < length) && equal; i++) {
	    equal = (one[i] == two[i]);
	}
	return equal;
}

#define FLAG_INDEX_SECRET_WRITTEN 0
#define FLAG_INDEX_ADULT          1
#define FLAG_INDEX_ADMIN          2
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
	return getFlag( uid, FLAG_INDEX_SECRET_WRITTEN );
}
static uint8_t setSecretBeenWritten( uid_t* uid )
{
	return setFlag( uid, FLAG_INDEX_SECRET_WRITTEN );
}
static uint8_t writeSecret( DS1961 ds, uid_t* uid )
{
	uint8_t written = 0;

	if ( !hasSecretBeenWritten( *uid ) )
	{
		//Serial.println( "Write secret" );
		if ( ds.WriteSecret(uid->id, uid->secret ) )
		{
			setSecretBeenWritten( uid );
			writeFlags( uid );
		}
	}	

	return written;
}

uint8_t isAdult( const uid_t uid )
{
	return !getFlag( uid, FLAG_INDEX_ADULT );
}
uint8_t unsetAdult( uid_t* uid )
{
	return setFlag( uid, FLAG_INDEX_ADULT );
}

uint8_t isAdmin( const uid_t uid )
{
	return getFlag( uid, FLAG_INDEX_ADMIN );
}
uint8_t setAdmin( uid_t* uid )
{
	return setFlag( uid, FLAG_INDEX_ADMIN );
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

	uint8_t length = 3;
	uint8_t challenge[ length ];
	entropyBufferGet( challenge, length );
	for( uint8_t i = 0; i < 3; i++ )
	{
		Serial.print( challenge[ i ], HEX ); Serial.print( " " );
	}
	Serial.println( );

	uint8_t data[32];
	uint8_t macDS[20];
	if ( ds.ReadAuthWithChallenge( uid.id, 0, challenge, data, macDS ) )
	{
		uint8_t macCalculated[20];
		uint8_t address = 0;
		calcmac_readauthpage(macCalculated, address, data, uid.secret, challenge, uid.id);
		authenticate = compare(macDS, macCalculated, 20);
	}

	return authenticate;
}

uid_t findNextUid ( const uid_t uid )
{
	return findNextUidByPosition( uid.position );
}

static uid_t findNextUidByPosition( const uint16_t position )
{
	uid_t nextUid;
	uint8_t found = 0;
	uint16_t maxPosition = maxUidPosition( );
	
	for(uint16_t pos = position + 1;  ( pos <= maxPosition ) && !found ; pos++ )
	{
		found = readUid( pos, &nextUid );
	}
	if ( !found )
	{
		nextUid = findNextUidByPosition( 0 );
	}

	return nextUid;
}

static uint8_t getUidSize( )
{
	uint8_t size = sizeof( uid_t ) - sizeof( uint16_t );
	return size;
}

uint8_t removeUid( uid_t* uid )
{
	uint8_t success = 0;

	if ( isValidUid( uid ) )
	{
		uint16_t eepromPosition = getEepromPosition( uid->position );
		uint8_t uidSize         = getUidSize( );
		for( uint8_t pos = eepromPosition; pos < ( eepromPosition + uidSize ); pos++ )
		{
			EEPROM.write( pos, 0xFF );
		}
		uid->position = 0;
		success = 1;
	}

	return success;
}


