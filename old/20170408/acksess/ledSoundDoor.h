#include <stdint.h>

#define relayPin  5     // the number of the relay pin
#define ledPin    3     // the number of the LED pin (change to 13 to see the onboard led)
#define buttonPin 2     // the number of the pushbutton pin
#define buzzerPin 9     // the number of the buzzer pin

void setupPins( );
void writeLed( uint8_t value );
void initNormalMode( );
void exitNormalMode( );
void initAdminMode( );
void exitAdminMode( );
void filteredSound( );
uint8_t isFireButtonPushedBlocking( );
uint8_t isFireButtonPushed( );
void unlockDoor( );
void lockDoor( );
void unlockLockDoor( );
uint8_t hasDoorBeenUnlocked( );
void resetHasDoorBeenUnlocked( );
void unlockDoorInterrupt( );
