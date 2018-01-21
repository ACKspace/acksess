#include <Arduino.h>
#include "WriteMessage.h"
#include "LiquidCrystal_PCF8574.h"

void writeMessage( const String& _strMessage, byte _nLine = 0 )
{
  // Sniff TX pin to see wheter it is serial (high) or I2C (low)
  pinMode( TX_PIN, INPUT );
  pinMode( RX_PIN, INPUT );
  delay( 20 );

  if ( !digitalRead( RX_PIN ) || !digitalRead( TX_PIN ) )
    writeI2CMessage( _strMessage, _nLine );  
  else
    writeSerialMessage( _strMessage );  
}

void writeMessage( const int& _nMessage, const int& _nType = DEC, byte _nLine = 0 )
{
  String strMessage( _nMessage, _nType );
  writeMessage( strMessage, _nLine );
}

void writeSerialMessage( const String& _strMessage )
{  
  Serial.begin( 115200 );
  Serial.println( _strMessage );
  Serial.end();
}

static bool g_bDisplayInitialized = false;
void resetDisplay( )
{
  g_bDisplayInitialized = false;
}
void writeI2CMessage( const String& _strMessage, byte _nLine )
{
  SoftwareWire wire( RX_PIN, TX_PIN );
  LiquidCrystal_PCF8574 lcd( 0x27 );  // set the LCD address to 0x27 for a 16 chars and 2 line display
  lcd.setWire( wire );

  if ( !g_bDisplayInitialized )
  {
    lcd.begin(16, 2); // initialize the lcd
    g_bDisplayInitialized = true;
  }
  else
  {
    lcd.begin2a(16, 2); // fake initialize
  }

  lcd.setBacklight( 255 );

  lcd.setCursor(0, _nLine);
  lcd.print( _strMessage );
}
