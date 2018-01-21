// Tested with:
// Arduino:
//   ide 1.8.1
//   Pro Mini (5v Atmega328)
// Libraries:
//   Onewire 2.3.2
//   SoftwareWire 1.4.1

#include "ds1961.h"
#include "KeyManagement.h"
#include "WriteMessage.h"

/* NOTE:
    the joystick contains a pro mini 5v Atmega328
    use the separate ftdi board (DTR, RX, TX, VCC, CTS, GND)
*/
/*
const int relayPin  = 5;     // the number of the relay pin
const int ledPin    = 13;     // the number of the LED pin (change to 13 to see the onboard led)
const int readerPin = 2;     // the number of the iButton reader pin
const int buttonPin = 4;     // the number of the pushbutton pin
const int buzzerPin = 9;     // the number of the buzzer pin
*/
//#define DEBUG
#ifndef DEBUG
#define relayPin  5     // the number of the relay pin
#define ledPin    3     // the number of the LED pin (change to 13 to see the onboard led)
#define readerPin 2     // the number of the iButton reader pin
#define buttonPin 4     // the number of the pushbutton pin
#define buzzerPin 9     // the number of the buzzer pin
#else
#define relayPin  5     // the number of the relay pin
#define ledPin    13     // the number of the LED pin (change to 13 to see the onboard led)
#define readerPin 2     // the number of the iButton reader pin
#define buttonPin 4     // the number of the pushbutton pin
#define buzzerPin 9     // the number of the buzzer pin
#endif

OneWire ow ( readerPin );
DS1961 ds ( &ow );
byte addr[ 8 ];

void setup(void)
{
  pinMode( buttonPin, INPUT_PULLUP );
  pinMode( ledPin, OUTPUT );
  pinMode( relayPin, OUTPUT );

  //DEBUG
  // Power
  //pinMode( 13, OUTPUT );
  //digitalWrite( 13, HIGH );

  pinMode( TX_PIN, INPUT );
  pinMode( RX_PIN, INPUT );

  // Set the reader 'external pullup'
#ifdef DEBUG
#pragma message "Arduino Nano pullup pin active"
  digitalWrite( 3, HIGH );
#endif


  // Unlock door upon power up and (on board) reset
  unlockDoor( );

//writeMessage( "                " );
  writeMessage( "ACKsess v1.3    ", 0 );
  writeMessage( "ROM:" + String( E2END + 1, DEC ) + "->" + String( (E2END + 1) >> 2, DEC ), 1 );

  delay( 1500 );
  //writeMessage( "M:" + String( sizeof( masterButtons ) / BUTTON_LENGTH) + " A:" + ( sizeof( allowedButtons ) / BUTTON_LENGTH ) + " D:" + ( sizeof( disallowedButtons ) / BUTTON_LENGTH ) + "  ", 1 );
  delay( 1500 );

  // Lock door
  lockDoor();
}

byte nState = 0;
byte nLedVal = 0;
bool bTamper = false;
byte g_nMode = 0;

byte cSerial;
int32_t keyIndex;

void loop(void)
{
  switch ( g_nMode )
  {
    case 0:
      // Regular
      idleModus();
      break;

    case 254:
    case 255:
      // Program modus
      programModus();
      break;

  }
}

void idleModus()
{
  switch ( nState )
  {
    case 0: // forward, led fade in
      nLedVal++;
      if ( nLedVal >= 255 )
        nState++;

      if ( bTamper )
          analogWrite( ledPin, nLedVal & 32 );
      else
          analogWrite( ledPin, nLedVal );

      delay( 1 );
      break;

    case 1: // backward, led fade out
      nLedVal--;
      if ( nLedVal <= 0 )
        nState++;

      if ( bTamper )
        analogWrite( ledPin, nLedVal & 32 );
      else
        analogWrite( ledPin, nLedVal );

        delay( 1 );
      break;

    default: // idle
        nState++;
        delay( 500 );

        if ( nState >= 10 )
          nState = 0;

      break;
  };

  // If the external button was pushed, open the door
  if ( digitalRead( buttonPin ) == LOW )
  {
    // ACKsess granted!
    unlockDoor( );
    writeMessage( "Joystick        ", 1 );
    openDoorSound( );
    lockDoor( );

    // Check if the master key has been placed
    if ( getKeyCode( ow, addr ) )
    {
      printKey( addr );

      // Master key present and button pushed? Enter program modus
      if ( authenticateKey( ds, addr, false, false, false, true ) && ( digitalRead( buttonPin ) == LOW ) )
      {
        resetDisplay( );
        writeMessage( "Program mode:add" );
        programBeep( true );

        // Reset key
        keyIndex = -1;
        g_nMode = 255;
      }
    }
    return;
  }

  // Check keys twice each fade and on every idle state step
  if ( (nLedVal == 127) || ( nState > 1 ) )
  {
    // Store the button info and read the keycode
    if ( getKeyCode( ow, addr ) )
    {
      // We have a correct key type, authenticate it
      printKey( addr );

      // Either open the door, or lock the system for 30 seconds
      if ( authenticateKey( ds, addr, false, true, true, true ) )
      {
        // disallowed buttons excluded
        bTamper = false;
        unlockDoor( );
        printKey( addr );
        openDoorSound( );
        lockDoor( );
      }
      else if ( !authenticateKey( ds, addr, true, true, true, true ) && ( keyIndex == -1 ) )
      {
        // disallowed buttons included
        bTamper = true;
        writeMessage( "ACKsess denied!  " );
        tone( buzzerPin, 600, 3000 );
        delay( 10000 );
      }
      else
      {
        writeMessage( "ACKsess filtered  " );
        tone( buzzerPin, 600, 500 );
        delay( 1000 );
        tone( buzzerPin, 600, 500 );
        delay( 1000 );
        tone( buzzerPin, 600, 1000 );
      }
    }
  }
}

void programModus()
{
  // TODO: Every button press is the next index to remove (to be sent to the display)
  //       This will cycle to a none-button
  //       Master key means delete (if index), or exit program modus

  // Check joystick: next (first) index -> master button means delete

  // If the external button was pushed, go to the next delete entry
  if ( digitalRead( buttonPin ) == LOW )
  {
    byte key[8];
    keyIndex = ( keyIndex + 1) % ((E2END + 1) >> 2 );
    while ( !getKey( keyIndex, key ) )
    {
      keyIndex = ( keyIndex + 1) % ((E2END + 1) >> 2 );
    }

    if ( keyIndex >= 0 )
    {
      writeMessage( "Program mode:del" );
      writeMessage( "@" + String( keyIndex ) );
      printKey( keyIndex );
    }

    // Wait until button is released
    byte nCounter = 0;
    while ( digitalRead( buttonPin ) == LOW )
    {
      delay( 10 );
      nCounter++;
      // Reset delete modus if button is pushed long
      if ( nCounter > 50 )
      {
        writeMessage( "Program mode:add" );
        writeMessage( "                ", 1 );
        keyIndex = -1;
        delay( 1000 );
      }
    }

    return;
  }

  if ( getKeyCode( ow, addr ) )
  {
    // Check if key not recognized: add it
    if ( !authenticateKey( ds, addr, true, true, true, true ) )
    {
        //writeMessage( "adding new key\n" );
        tone( buzzerPin, 1300, 300 );
        
        //writeMessage( "Added @ " + String( keyIndex, DEC ) + "  " );
        printKey( addr );
        addKey( addr );

        delay( 500 );
    }

    // Check if master key is present
    if ( authenticateKey( ds, addr, false, false, false, true ) )
    {
        // Check if we have a 'delete index' set: remove the key
        if ( keyIndex > 0 )
        {
          deleteKey( addr );
        //writeMessage( "acksess granted!" );
          writeMessage( "Deleted @ " + String( keyIndex ) );

          //writeMessage( "adding new key\n" );
          tone( buzzerPin, 700, 1000 );

          // Reset delete modus
          keyIndex = -1;
          delay( 1000 );
        }
        else
        {
          g_nMode = 0;
          writeMessage( "Normal mode  " );
          programBeep( false );
          return;
        }
    }
  }

  delay( 10 );
}

void openDoorSound( )
{
  // Blink the led fast for about 3 seconds
  for ( byte n = 0; n < 2; n++ )  // 250+250*6 500+500*3
  {
    digitalWrite( ledPin, HIGH );
    tone( buzzerPin, 1000, 250 );
    delay( 250 );
    digitalWrite( ledPin, LOW );
    delay( 250 );
  }

  digitalWrite( ledPin, HIGH );
  delay( 4000 );

  tone( buzzerPin, 1000, 1000 );
  delay( 1000 );
}

void programBeep( bool _bOn )
{
  if ( _bOn)
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
  }
  else
  {
    tone( buzzerPin, 1600, 300 );
    delay( 600 );
    tone( buzzerPin, 1600, 300 );
    delay( 300 );
    tone( buzzerPin, 1300, 300 );
    delay( 300 );
    tone( buzzerPin, 1000, 300 );
    delay( 300 );
    tone( buzzerPin, 700, 1000 );
  }
}

void unlockDoor( )
{
  resetDisplay();
  writeMessage( "ACKsess granted!" );
  digitalWrite( relayPin, HIGH );
}

void lockDoor()
{
  digitalWrite( ledPin, LOW );

  // Relay off
  digitalWrite( relayPin, LOW );
}
