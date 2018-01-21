#include <stdint.h>

#define ENTROPY_BUFFER_SIZE 350

uint16_t entropyBufferAvailable( );
uint8_t  entropyBufferFill( );
uint8_t  entropyBufferGet( uint8_t *entropyBuffer, uint16_t length );
