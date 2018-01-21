#include "serial.h"
#include "LiquidCrystal_PCF8574.h"

static uint8_t isDisplayInitialized = 0;
void unsetDisplayInitialized( )
{
	isDisplayInitialized = 0;
}
void writeSerialMessage( const char* message )
{
	writeSerialMessage( message, 1 );
}
void writeSerialMessage( const char* message, const uint8_t line )
{
    writeI2CMessage( message, line );

	/*
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
		unsetDisplayInitialized( );
		Serial.begin( 115200 );
		Serial.println( message );
		Serial.end( );
	}
	*/
}
void hexdump( const uint8_t* a, const uint8_t length )
{
	char str[ length * 2 ];
	String hexStr;
	for( uint8_t i = 0; i < length; i++ )
	{
		hexStr = String( a[ i ], HEX );
		if ( a[ i ] <= 0x0F )
		{
			str[ i * 2 ]     = '0';
			str[ i * 2 + 1 ] = hexStr.charAt( 0 );
		}
		else
		{
			str[ i * 2 ]     = hexStr.charAt( 0 );
			str[ i * 2 + 1 ] = hexStr.charAt( 1 );
		}
	}
	writeSerialMessage( str );
}

static void writeI2CMessage( const char* message, const uint8_t line )
{
	//SoftwareWire wire( RX_PIN, TX_PIN );
	LiquidCrystal_PCF8574 lcd( 0x27 );  // set the LCD address to 0x27 for a 16 chars and 2 line display

	if ( !isDisplayInitialized )
	{
		lcd.begin( 16, 2 ); // initialize the lcd
		isDisplayInitialized = 1;
	}

	if ( line == 0 )
	{
		lcd.clear( );
	}
	lcd.setBacklight( 255 );
	lcd.setCursor( 0, line );
	lcd.print( message );

	for( uint8_t diff = strlen( message ) ; diff <= 16; diff++ )
	{
		lcd.setCursor( diff, line );
		lcd.print( " " );
	}
}

