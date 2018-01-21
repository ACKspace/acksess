#include "ledSoundDoor.h"
#include <Arduino.h>
#include "serial.h"

void setupPins( )
{
	pinMode( ledPin, OUTPUT );
	pinMode( buttonPin, INPUT );
	pinMode( relayPin, OUTPUT );

	//attachInterrupt( digitalPinToInterrupt( buttonPin ), unlockDoorInterrupt, HIGH );
}

void writeLed( uint8_t value )
{
	analogWrite( ledPin, value );
}

void initNormalMode( )
{
	writeSerialMessage( "Normal mode", 0 );
	writeLed( 0 );
 	lockDoor( );
}

void exitNormalMode( )
{	
}

void initAdminMode( )
{
	unlockDoor( );
	unsetDisplayInitialized( );

    tone( buzzerPin, 1600, 300 );
    delay( 600 );
    tone( buzzerPin, 1600, 300 );
    delay( 300 );
    tone( buzzerPin, 1300, 300 );
    delay( 300 );
    tone( buzzerPin, 1000, 300 );
    delay( 300 );
    tone( buzzerPin, 700, 1000 );
    delay( 1000 );

	writeSerialMessage( "Admin mode", 0 );
}
void exitAdminMode( )
{
    tone( buzzerPin, 700, 300 );
    delay( 600 );
    tone( buzzerPin, 700, 300 );
    delay( 300 );
    tone( buzzerPin, 1000, 300 );
    delay( 300 );
    tone( buzzerPin, 1300, 300 );
    delay( 300 );
    tone( buzzerPin, 1600, 1000 );
    delay( 1000 );
}


void filteredSound( )
{
	writeSerialMessage( "Filtered", 1 );
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
	uint8_t isPushed = ( digitalRead( buttonPin ) == LOW );
	return isPushed;
}

static volatile uint8_t isDoorUnlocked = 0;
uint8_t hasDoorBeenUnlocked( )
{
	return isDoorUnlocked;
}
void resetHasDoorBeenUnlocked( )
{
	isDoorUnlocked = 0;
}
void unlockDoorInterrupt( )
{
	digitalWrite( relayPin, HIGH );
	isDoorUnlocked = 1;
}

void unlockDoor( uint8_t sound = 1 )
{
	digitalWrite( relayPin, HIGH );
 	// Blink the led fast for about 3 seconds
	for ( uint8_t i = 0; i < 2; i++ )  // 250+250*6 500+500*3
	{
		writeLed( 0xFF );
		if ( sound )
		{
			tone( buzzerPin, 1000, 250 );
		}
		delay( 250 );
		writeLed( 0 );
		delay( 250 );
	}
	writeLed( 0xFF );
	writeSerialMessage( "Door unlocked", 1 );
	delay( 4000 );
}
void lockDoor( uint8_t sound = 1 )
{
	if ( sound )
	{
		tone( buzzerPin, 1000, 1000 );
	}
	delay( 1000 );
	writeSerialMessage( "Door locked", 1 );
	digitalWrite( relayPin, LOW );
}
void unlockLockDoor( uint8_t sound = 1 )
{
	unlockDoor( sound );
	lockDoor( sound );
}
