#include <OneWire.h>
#include <EEPROM.h>
#include "Entropy.h"
#include "ledSoundDoor.h"
#include "key.h"
#include "serial.h"

static OneWire onewire( 4 );
static DS1961 ds( &onewire );

void setup( )
{
	//writeEeprom( );

	setupPins( );
	Entropy.initialize();

	writeSerialMessage( "Reset", 0 );
	unlockDoor( );
	initNormalMode( );
}

void loop( )
{
	uint8_t  readKeyResult;
	uint8_t  id[ ID_LENGTH ];
	uid_t    uid;
	uid_t    uidIndex;
	uint8_t  buttonPushed      = 0;
	uint8_t  adminUid;
	uint16_t blink             = 0;
	uint16_t blinkModulo;
	uint8_t  blinkDirection    = 0;
	uint8_t  tampered          = 0;
	uint32_t time              = 0;
	uint8_t  adminMode         = 0;
	uint8_t  previousAdminMode = adminMode;

	while( 1 )
	{
		// If the mode has been changed
		// run the corresponding exit and init commands.
		if ( adminMode != previousAdminMode )
		{
			if ( adminMode )
			{
				exitNormalMode( );
				initAdminMode( );
			}
			else
			{
				exitAdminMode( );
				initNormalMode( );
			}
			previousAdminMode = adminMode;
		}

		// The current mode is normal mode.
		if ( !adminMode )
		{
			// There hasn't been an admin key successfully authenticated.
			adminUid = 0;

			// Check if the button interrupt
			// has been triggered.
			buttonPushed = hasDoorBeenUnlocked( );
			if ( buttonPushed )
			{
				resetHasDoorBeenUnlocked( );
				unlockLockDoor( ( ( millis( ) - time ) > 1000 ) );
				time = millis( );
			}

			readKeyResult = readKey( onewire, id );
			if ( readKeyResult == READ_KEY_INVALID_CRC ) 
			{
				tampered = 1;
				delay( 5000 );
			}
			else if ( readKeyResult == READ_KEY_SUCCESS )
			{
				if ( ! findUid( id, &uid ) )
				{
					tampered = 1;
					delay( 5000 );
				}
				else 
				{
					tampered = 0;
					if ( authenticateUid( ds, uid ) )
					{
						if ( isAdult( uid ) )
						{
							unlockLockDoor( );
							adminUid = isAdmin( uid );
						}
						else
						{
							filteredSound( );
						}
					}
				}
			}

			if ( blink == 0xFFFF )
			{
				blinkDirection = 1;
			}
			if ( blink == 0x00 )
			{
				blinkDirection = 0;
			}
			blinkDirection ? blink-- : blink++;

			blinkModulo = blink % 0xFFF;
			if ( tampered )
			{
				writeLed( ( ( blinkModulo % 100 ) < 20 ) ? 0xFF : 0 );
			}
			else
			{
				if ( blinkModulo <= 0xFF )
				{
					writeLed( blinkModulo );
				}
				else if (blinkModulo <= ( 2 * 0xFF ) )
				{
					writeLed( ( 2 * 0xFF ) - blinkModulo );
				}
			}

			// Enter admin mode if the button was pushed
			// and an admin key was succesfully authenticated.
			adminMode = ( buttonPushed && adminUid );
		}
		// The current mode is admin mode.
		else
		{
			if ( time == millis( ) )
			{
				writeSerialMessage( "", 1 );
				time = 0;
				uidIndex.position = 0;
			}
			if ( isFireButtonPushedBlocking( ) )
			{
				time = millis( ) + ( 10 * 1000 );
				uidIndex = findNextUid( uidIndex );
				if ( isValidUid( &uidIndex ) )
				{
					hexdump( uidIndex.id, ID_LENGTH );
				}
			}
			else if ( readKey( onewire, id ) == READ_KEY_SUCCESS )
			{
				if ( ! findUid( id, &uid ) )
				{
					uid = constructUid( id );
					if ( writeUid( &uid ) )
					{
						hexdump( uid.id, ID_LENGTH );
						delay( 5000 );
						writeSerialMessage( "Added!", 1 );
						delay( 2000 );
						writeSerialMessage( "", 1 );
					}
				}
				else if ( isAdmin( uid ) )
				{
					if ( isValidUid( &uidIndex ) )
					{
						if ( compareUid( uid, uidIndex ) )
						{
							writeSerialMessage( "Own key!", 1 );
							delay( 5000 );
							writeSerialMessage( "", 1 );
							hexdump( uidIndex.id, ID_LENGTH );
							time = millis( ) + ( 10 * 1000 );
						}
						else
						{
							removeUid( &uidIndex );
							writeSerialMessage( "Removed", 1 );
							time = 0;
							delay( 10000 );
							writeSerialMessage( "", 1 );
						}
					}
					else
					{
						adminMode = 0;
					}
				}
			}			
		}
	}
}
