#include <stdint.h>

#define buttonPin  2 // the number of the pushbutton pin
#define ledPin     3 // the number of the LED pin (change to 13 to see the onboard led)
#define relayPin   4 // the number of the relay pin
#define buzzerPin 10 // the number of the buzzer pin

void setupPins( );
void writeLed( uint8_t value );
void initNormalMode( );
void exitNormalMode( );
void initAdminMode( );
void exitAdminMode( );
void filteredSound( );
uint8_t isFireButtonPushedBlocking( );
uint8_t isFireButtonPushed( );
void unlockDoor( uint8_t sound = 1 );
void lockDoor( uint8_t sound = 1 );
void unlockLockDoor( uint8_t sound = 1);
uint8_t hasDoorBeenUnlocked( );
void resetHasDoorBeenUnlocked( );
void unlockDoorInterrupt( );
