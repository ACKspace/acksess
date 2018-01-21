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
	uint8_t readKeyResult;
	uint8_t id[ ID_LENGTH ];
	uid_t uid;
	uid_t uidIndex;
	uint8_t  buttonPushed     = 0;
	uint8_t  admin;
	uint16_t blink            = 0;
	uint16_t blinkModulo;
	uint8_t  blinkDirection   = 0;
	uint32_t tamper           = 0;
	uint32_t time             = 0;
	uint8_t adminMode         = 0;
	uint8_t previousAdminMode = adminMode;

	while( 1 )
	{
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

		if ( !adminMode )
		{
			admin        = 0;
			buttonPushed = 0;

			if ( ( buttonPushed = hasDoorBeenUnlocked( ) ) )
			{
				resetHasDoorBeenUnlocked( );
				unlockLockDoor( );
				tamper = 0;
			}

			if ( tamper )
			{
				tamper--;
			}
			else
			{
				readKeyResult = readKey( onewire, id );
				if ( readKeyResult == READ_KEY_INVALID_CRC ) 
				{
					tamper = 0xFFFF;
				}
				else if ( readKeyResult == READ_KEY_SUCCESS )
				{
					if ( findUid( id, &uid ) )
					{
						tamper = 0;
						if ( authenticateUid( ds, uid ) )
						{
							if ( isAdult( uid ) )
							{
								unlockLockDoor( );
								admin = isAdmin( uid );
							}
							else
							{
								filteredSound( );
							}
						}
					} else {
						tamper = 0x4FFFF;
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
			if ( tamper )
			{
				( ( blinkModulo < 100 ) || 
					( ( blinkModulo >= 150 ) && ( blinkModulo < 250 ) ) ||
					( ( blinkModulo >= 300 ) && ( blinkModulo < 400 ) ) ||
					( ( blinkModulo >= 450 ) && ( blinkModulo < 500 ) )
				) ?	writeLed( 0xFF ) : writeLed( 0 );
			}
			else
			{
				if ( blinkModulo < ( 0xFF * 2 ) )
				{
					( blinkModulo < 0xFF ) ? writeLed( blinkModulo ) : writeLed( 0xFF - ( blinkModulo - 0xFF ) );
					delay( 2 );
				} else if ( blinkModulo == ( 0xFF * 2 ) )
				{
					writeLed( 0 );
				}
			}

			adminMode = ( buttonPushed && admin );
		}
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
