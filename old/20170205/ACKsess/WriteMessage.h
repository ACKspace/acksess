#define RX_PIN 0
#define TX_PIN 1

void writeMessage( const String& _strMessage, byte _nLine = 0 );
void writeMessage( const int& _nMessage, const int& _nType = DEC, byte _nLine = 0 );
void writeSerialMessage( const String& _strMessage );
void writeI2CMessage( const String& _strMessage, byte _nLine );
void resetDisplay( );
