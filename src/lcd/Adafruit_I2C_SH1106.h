#ifndef __ADAFRUIT_I2C_SH1106_H__
#define __ADAFRUIT_I2C_SH1106_H__

//#include "Adafruit_GFX_ext.h"
#include <Adafruit_GFX.h>
// #include "../menu/Adafruit_GFX_Renderer.h"

//#define offset 0x00    // SDD1306                      // offset=0 for SSD1306 controller
#define offset 0x02    // SH1106                       // offset=2 for SH1106 controller
#define OLED_address  0x3c                             // all the OLED's I have seen have this address

#define BLACK  0
#define WHITE  1
#define INVERSE 3

#define SH1106_LCDHEIGHT 64
#define SH1106_LCDWIDTH 128


#define SH1106_PAGES   (SH1106_LCDHEIGHT / 8)
#define SH1106_COLUMNS (SH1106_LCDWIDTH / 8)


/************************************************
 *
 ***********************************************/
class Adafruit_I2C_SH1106 : public Adafruit_GFX
{

	public:
		Adafruit_I2C_SH1106();

		void drawPixel(int16_t x, int16_t y, uint16_t color);
		void flushDisplay();
		void init(void);
		void flush(bool clear);

//==========================================================//
// Resets display depending on the actual mode.
void resetDisplay(void);

//==========================================================//
// Turns display on.
void displayOn(void);

//==========================================================//
// Turns display off.
void displayOff(void);

//==========================================================//
// Clears the display by sendind 0 to all the screen map.
void clearDisplay(void);

//==========================================================//
// Actually this sends a byte, not a char to draw in the display.
// Display's chars uses 8 byte font the small ones and 96 bytes
// for the big number font.
void sendChar(unsigned char data);


//==========================================================//
// Used to send commands to the display.
void sendcommand(unsigned char com);

//==========================================================//
// Set the cursor position in a 16 COL * 8 ROW map.
void setSegment(unsigned char page, unsigned char col);



};

#endif
