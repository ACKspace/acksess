#include <stdint.h>

#define RX_PIN 12
#define TX_PIN 13

void unsetDisplayInitialized( );
void writeSerialMessage( const char* message );
void writeSerialMessage( const char* message, const uint8_t line );
void hexdump( const uint8_t* a, const uint8_t length );
static void writeI2CMessage( const char* message, const uint8_t line );
