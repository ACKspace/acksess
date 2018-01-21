#include <OneWire.h>
#include <EEPROM.h>
#include "key.h"
#include "LiquidCrystal_PCF8574.h"
#include "hexdump.h"

#define readerPin 2     // the number of the iButton reader pin
#define relayPin  5     // the number of the relay pin
#define ledPin    3     // the number of the LED pin (change to 13 to see the onboard led)
#define buttonPin 4     // the number of the pushbutton pin
#define buzzerPin 9     // the number of the buzzer pin

#define RX_PIN 12
#define TX_PIN 13

static OneWire onewire( readerPin );
static DS1961 ds( &onewire );

void writeMessage( const String& message, byte line = 0 )
{
	// Sniff TX pin to see wheter it is serial (high) or I2C (low)
	pinMode( TX_PIN, INPUT );
	pinMode( RX_PIN, INPUT );
	delay( 20 );

	if ( !digitalRead( RX_PIN ) || !digitalRead( TX_PIN ) )
	{
	    writeI2CMessage( message, line );
	}
	else
	{
		writeSerialMessage( message );
	}
}

void writeMessage( const int& message, const int& type = DEC, byte line = 0 )
{
	String strMessage( message, type );
	writeMessage( message, line );
}

void writeSerialMessage( const String& message )
{  
	Serial.println( message );
}

uint8_t isDisplayInitialized = 0;
void writeI2CMessage( const String& message, byte line )
{
	SoftwareWire wire( RX_PIN, TX_PIN );
	LiquidCrystal_PCF8574 lcd( 0x27 );  // set the LCD address to 0x27 for a 16 chars and 2 line display
	lcd.setWire( wire );

	if ( !isDisplayInitialized )
	{
		lcd.begin( 16, 2 ); // initialize the lcd
		isDisplayInitialized = 1;
	}
	else
	{
		lcd.begin2a( 16, 2 ); // fake initialize
	}

	lcd.setBacklight( 255 );
	lcd.setCursor(0, line);
	lcd.print( message );
}

void setup( )
{
	Serial.begin( 115200 );
	Serial.println( "Reset" );

	pinMode( ledPin, OUTPUT );
	pinMode( buttonPin, INPUT_PULLUP );
	pinMode( relayPin, OUTPUT );

	//writeEeprom( );
}

void loop( )
{
	adminMode( );
}

void normalMode( )
{
	uint8_t readKeyResult;
	uint8_t id[ ID_LENGTH ];
	uid_t uid;
	uint8_t  buttonPushed;
	uint8_t  admin;
	uint16_t blink          = 0;
	uint16_t blinkModulo;
	uint8_t  blinkDirection = 0;
	uint32_t tamper         = 0;

	Serial.println( "Normal mode" );
	lockDoor( );
	analogWrite( ledPin, 0 );

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

		if ( tamper )
		{
			tamper--;
		}
		else
		{
			//if ( blink % 0xFF == 0) 
			if ( 1 )
			{
				readKeyResult = readKey( onewire, id );
				if ( readKeyResult == READ_KEY_INVALID_CRC ) 
				{
					Serial.println( "Tamper detected!" );
					tamper = 0x4FFFF;
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
								Serial.println( "Open the door" );
								unlockSoundLockDoor( );
								admin = isAdmin( uid );
							}
							else
							{
								Serial.println( "Filtered" );
								filteredSound( );
							}
						}
					} else {
						Serial.println( "Tamper detected!" );
						tamper = 0x4FFFF;
					}
				}

				if ( buttonPushed && admin )
				{
					adminMode( );
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
		if ( !blinkDirection )
		{
			blink++;
		}
		else
		{
			blink--;
		}
		blinkModulo = blink % 0xFFF;
		if ( tamper )
		{
			if ( ( blinkModulo < 100 ) || 
				( ( blinkModulo >= 150 ) && ( blinkModulo < 250 ) ) ||
				( ( blinkModulo >= 300 ) && ( blinkModulo < 400 ) ) ||
				( ( blinkModulo >= 450 ) && ( blinkModulo < 500 ) )
			)
			{
		    	analogWrite( ledPin, 0xFF );
			}
			else
			{
		    	analogWrite( ledPin, 0 );
			}
		}
		else
		{
			if ( blinkModulo < ( 0xFF * 2 ) )
			{
				if ( blinkModulo < 0xFF )
				{
			    	analogWrite( ledPin, ( blinkModulo ) );
				}
				else
				{
			    	analogWrite( ledPin, ( 0xFF - ( blinkModulo - 0xFF ) ) );
				}
				delay( 2 );
			}
			if ( blinkModulo == ( 0xFF * 2 ) )
			{
		    	analogWrite( ledPin, 0 );
			}
		}
	}
}

void adminMode( )
{
	uint8_t id[ ID_LENGTH ];
	uid_t uid;
	uid_t uidIndex;
	uint32_t time = 0;

	Serial.println( "Admin mode" );
	writeMessage( "Admin mode" );
	unlockDoor( );
	analogWrite( ledPin, 0xFF );

	while ( 1 )
	{
		if ( time == millis( ) )
		{
			Serial.println( "Reset delete index" );
			time = 0;
			uidIndex.position = 0;
		}
		if ( isFireButtonPushedBlocking( ) )
		{
			time = millis( ) + ( 10 * 1000 );
			uidIndex = findNextUid( uidIndex );
			Serial.print( uidIndex.position ); Serial.print( " " ) ;
			hexdump( uidIndex.id, ID_LENGTH );
		}
		else if ( readKey( onewire, id ) == READ_KEY_SUCCESS )
		{
			if ( ! findUid( id, &uid ) )
			{
				uid = constructUid( id );
				if ( writeUid( &uid ) )
				{
					Serial.println( "Written: " );
					hexdump( uid.id, ID_LENGTH );
					normalMode( );
				}
			}
			else
			{
				if ( isAdmin( uid ) )
				{
					if ( isValidUid( &uidIndex ) )
					{
						Serial.println( "Remove" );
						uidIndex.position = 0;
						time = 0;
						delay( 5000 );
					}
					else
					{
						normalMode( );
					}
				}
			}
		}
	}
}

void openDoorSound( )
{
 	// Blink the led fast for about 3 seconds
	for ( uint8_t i = 0; i < 2; i++ )  // 250+250*6 500+500*3
	{
		analogWrite( ledPin, 0xFF );
		tone( buzzerPin, 1000, 250 );
		delay( 250 );
		analogWrite( ledPin, 0x00 );
		delay( 250 );
	}

	analogWrite( ledPin, 0xFF );
	delay( 4000 );

	tone( buzzerPin, 1000, 1000 );
	delay( 1000 );
}

void filteredSound( )
{
	tone( buzzerPin, 600, 500 );
	delay( 1000 );
	tone( buzzerPin, 600, 500 );
	delay( 1000 );
	tone( buzzerPin, 600, 1000 );
}

uint8_t isFireButtonPushedBlocking( )
{
	uint8_t pushed = isFireButtonPushed( );
	if ( pushed )
	{
		while ( isFireButtonPushed( ) )
		{
		}
	}
	return pushed;
}
uint8_t isFireButtonPushed( )
{
	return digitalRead( buttonPin ) == LOW;
}


void unlockDoor( )
{
	Serial.println( "Unlock door" );
	digitalWrite( relayPin, HIGH );
}

void lockDoor( )
{
	Serial.println( "Lock door" );
	digitalWrite( relayPin, LOW );
}

void unlockSoundLockDoor( )
{
	unlockDoor( );
	openDoorSound( );
	lockDoor( );
}
