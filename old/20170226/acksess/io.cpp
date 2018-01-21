#include "io.h"

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

void setupIo( )
{
	pinMode( ledPin, OUTPUT );
	pinMode( buttonPin, INPUT_PULLUP );
	pinMode( relayPin, OUTPUT );
}

uint8_t isFireButtonPushed( )
{
	return ( digitalRead( buttonPin ) == LOW );
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
