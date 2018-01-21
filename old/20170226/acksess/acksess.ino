 #include <OneWire.h>
#include <EEPROM.h>
#include "key.h"
#include "io.h"
#include "hexdump.h"

#define readerPin 2 // the number of the iButton reader pin
static OneWire onewire( readerPin );
static DS1961 ds( &onewire );

void setup( )
{
	Serial.begin( 115200 );
	Serial.println( "Reset" );

	setupIo( );

	//writeEeprom( );
}

void loop( )
{
	normalMode( );
}

void normalMode( )
{
	uint8_t readKeyResult;
	uint8_t id[ ID_LENGTH ];
	uid_t uid;
	uint8_t buttonPushed;
	uint8_t admin;
	uint8_t blink          = 0;
	uint8_t blinkDirection = 0;
	uint8_t tamper         = 0;

	Serial.println( "Normal mode" );

	while( 1 )
	{
		buttonPushed = 0;
		admin        = 0;

		if ( buttonPushed = isFireButtonPushed( ) )
		{
			Serial.println( "Firebutton pushed" );
			unlockSoundLockDoor( );
			tamper = 0;
		}

		if ( blink == 0 )
		{
			readKeyResult = readKey( onewire, id );
			if ( readKeyResult == READ_KEY_INVALID_CRC ) 
			{
				tamper = 1;
			}
			else if ( readKeyResult == READ_KEY_SUCCESS )
			{
				Serial.print( "Id: " ); hexdump( id, ID_LENGTH );
				if ( findUid( id, &uid ) )
				{
					tamper = 0;
					if ( authenticateUid( ds, uid ) )
					{
						Serial.println( "Authenticated" );
						if ( isAdult( uid ) )
						{
							Serial.println( "Is adult: open the door" );
							unlockSoundLockDoor( );
							admin = isAdmin( uid );
						}
						else
						{
							Serial.println( "Is _not_ adult: filtered" );
							filteredSound( );
						}
					}
				} else {
					tamper = 1;
				}
			}

			if ( buttonPushed && admin )
			{
				adminMode( );
			}
		}

		if ( tamper )
		{
			for ( uint8_t i = 0; i < 4; i++ )
			{
				analogWrite( ledPin, 0xFF );
				delay( 100 );
				analogWrite( ledPin, 0x00 );
				delay( 100 );
			}
			delay( 400 );
		}
		else
		{
			if ( !blinkDirection )
			{
				blink++;
			}
			else
			{
				blink--;
			}
	    	analogWrite( ledPin, blink );
			delay( 2 );
			if ( blink == 0xFF )
			{
				blinkDirection = 1;
			}
			if ( blink == 0x00 )
			{
				blinkDirection = 0;
				delay( 4000 );
			}
		}
	}
}

void adminMode( )
{
	Serial.println( "Admin mode" );
	normalMode( );
}
