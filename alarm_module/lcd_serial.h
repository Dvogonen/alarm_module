/*
 *  This class is used to dispatch serial data to BOTH the normal Serial interface 
 *  and to a 128*64 LCD. The LCD acts as a tele terminal with 8 lines of 20 characters.
 *  Each new line is written to the last row.
 *  Any character beyond 20 is truncated. No automatic newline.
 *  The idea is to be able view debug output without being connected to a computer.
 *  Only a subset of the Serial class is defined.
 *  
 *  Copyright 2020 Kjell Kernen
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy 
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights 
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 *  copies of the Software, and to permit persons to whom the Software is 
 *  furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all 
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 *  SOFTWARE.
 */

#ifndef _LCD_TERMINAL
#define _LCD_TERMINAL

#include <SPI.h>              // Verified with SPI 1.0 by Arduino
#include <Wire.h>             // Verified with Wire 1.0 by Arduino
#include <Adafruit_GFX.h>     // Verified with Adafruit_GFX_Library 1.1.9 by Adafruit
#include <Adafruit_SSD1306.h> // Verified with Adafruit_SSD1306 1.1.2 by Adafruit

// type definitions
typedef unsigned char uint8_t;

class lcd_serial
{
private:
  Adafruit_SSD1306 lcd;

  int const static COLUMNS = 20;
  int const static ROWS = 9;
  char term_buff[ROWS][COLUMNS + 1];
  uint8_t column;

  void rewrite(void);
  void output(char);

public:
  lcd_serial(void);

  void print(const char *);
  void print(char);
  void print(int);
  void print(unsigned int);
  void print(unsigned char);

  void println(void);
  void println(const char *);
  void println(char);
  void println(int);
  void println(unsigned int);
  void println(unsigned char);
};

#endif /* _LCD_TERMINAL */
