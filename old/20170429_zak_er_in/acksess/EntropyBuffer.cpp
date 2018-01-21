#include "EntropyBuffer.h"
#include "Entropy.h"

static uint8_t  buffer[ ENTROPY_BUFFER_SIZE ];
static uint16_t available = 0;
static uint16_t  position  = 0;

uint16_t entropyBufferAvailable( )
{
	return available;
}

uint8_t entropyBufferFill( )
{
	uint8_t filled = 0;

	if ( ( ENTROPY_BUFFER_SIZE - available ) > 0 )
	{
		buffer[ position ] = Entropy.randomByte( );
		position = ( position + 1 ) % ENTROPY_BUFFER_SIZE;
		available++;
		filled = 1;
	}

	return filled;
}

uint8_t entropyBufferGet( uint8_t *entropyBuffer, uint16_t length )
{
	while( length > available )
	{
		entropyBufferFill( );
	}

	for( uint16_t p = 0; p < length; p++ )
	{
		entropyBuffer[ p ] = buffer[ position ];
		position = ( position - 1 ) % ENTROPY_BUFFER_SIZE;
	}
	available -= length;

	return 1;
}
