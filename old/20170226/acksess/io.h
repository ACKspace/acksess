#include <stdint.h>
#include <Arduino.h>

#define relayPin  5     // the number of the relay pin
#define ledPin    3     // the number of the LED pin (change to 13 to see the onboard led)
#define buttonPin 4     // the number of the pushbutton pin
#define buzzerPin 9     // the number of the buzzer pin

void setupIo( );

void openDoorSound( );
void filteredSound( );

void unlockDoor( );
void lockDoor( );
void unlockSoundLockDoor( );

uint8_t isFireButtonPushed( );
