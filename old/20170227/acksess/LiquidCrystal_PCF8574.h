#include <stdint.h>
#include <SoftwareWire.h>
//#include <LiquidCrystal_PCF8574.h> // copied over this lib since we made a change to it (added SoftwareWire pointer)

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class LiquidCrystal_PCF8574 : public Print {
public:
  LiquidCrystal_PCF8574(uint8_t adr);

  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);
  void begin2a(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);
  void begin2b();

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void setBacklight(uint8_t brightness);
  void setWire( const SoftwareWire& _wire );
  
  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t col, uint8_t row); 

  virtual size_t write(uint8_t);
  using Print::write;

private:
  // low level functions
  void _command(uint8_t);
  void _send(uint8_t value, uint8_t mode);
  void _sendNibble(uint8_t halfByte, uint8_t mode);
  void _write2Wire(uint8_t halfByte, uint8_t mode, uint8_t enable);

// NEW:
  const SoftwareWire* m_wire;
  uint8_t _Addr;        ///< Wire Address of the LCD
  uint8_t _backlight;   ///< the backlight intensity 

  uint8_t _displayfunction; ///< lines and dots mode
  uint8_t _displaycontrol;  ///< cursor, display, blink flags
  uint8_t _displaymode;     ///< left2right, autoscroll

  uint8_t _numlines;        ///< The number of rows the display supports.
};



/// Definitions on how the PCF8574 is connected to the LCD

/// These are Bit-Masks for the special signals and background light
#define PCF_RS  0x01
#define PCF_RW  0x02
#define PCF_EN  0x04
#define PCF_BACKLIGHT 0x08

// Definitions on how the PCF8574 is connected to the LCD
// These are Bit-Masks for the special signals and Background
#define RSMODE_CMD  0
#define RSMODE_DATA 1


// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

// modification:
// don't use ports from Arduino, but use ports from Wire

// a nibble is a half Byte

// NEW: http://playground.arduino.cc//Code/LCDAPI
// NEW: setBacklight
