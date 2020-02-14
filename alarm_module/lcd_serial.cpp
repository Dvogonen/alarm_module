/*
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

#include "lcd_serial.h"

lcd_serial::lcd_serial(void) : lcd(0)
{
  // Initialize serial communication
  Serial.begin(115200);
  delay(10);
  Serial.println();

  // Initialize LCD
  lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3C (May need to be 0x3C)
  lcd.setTextSize(1);
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, 7);
  lcd.clearDisplay();
  lcd.display();

  // Initialize LCD buffer
  column = 0;
  for (uint8_t i = 0; i < ROWS; i++)
  {
    for (uint8_t j = 0; j < COLUMNS; j++)
    {
      term_buff[i][j] = ' ';
    }
    term_buff[i][COLUMNS] = 0;
  }
}

void lcd_serial::rewrite(void)
{
  lcd.setCursor(0, 0);
  lcd.clearDisplay();
  for (uint8_t i = 0; i < ROWS; i++)
  {
    lcd.println(term_buff[i]);
  }
  lcd.display();
}

void lcd_serial::output(char ch)
{
  if (ch == 10)
  {
    for (uint8_t i = 0; i < ROWS - 1; i++)
    {
      for (uint8_t j = 0; j < COLUMNS; j++)
      {
        term_buff[i][j] = term_buff[i + 1][j];
      }
    }
    for (uint8_t i = 0; i < COLUMNS; i++)
      term_buff[ROWS - 1][i] = ' ';
    this->rewrite();
    column = 0;
    Serial.println("");
  }
  else
  {
    Serial.write(ch);
    if (column < COLUMNS)
    {
      term_buff[ROWS - 1][column] = ch;
      column++;
    }
  }
}

void lcd_serial::print(const char *ch)
{
  for (uint8_t i = 0; ch[i]; i++)
  {
    this->output(ch[i]);
  }
}

void lcd_serial::print(char ch)
{
  this->output(ch);
}

void lcd_serial::print(unsigned char ch)
{
  char intbuff[5];
  itoa(ch, intbuff, 10);
  this->print(intbuff);
}

void lcd_serial::print(unsigned int i)
{
  char intbuff[5];
  itoa(i, intbuff, 10);
  this->print(intbuff);
}

void lcd_serial::print(int i)
{
  char intbuff[6];
  itoa(i, intbuff, 10);
  this->print(intbuff);
}

void lcd_serial::println(void)
{
  this->print("\x0a"); // Line Feed
}

void lcd_serial::println(const char *str)
{
  this->print(str);
  this->print("\x0a"); // Line Feed
}

void lcd_serial::println(char ch)
{
  this->print(ch);
  this->print("\x0a"); // Line Feed
}

void lcd_serial::println(unsigned char ch)
{
  this->print(ch);
  this->print("\x0a"); // Line Feed
}

void lcd_serial::println(unsigned int i)
{
  this->print(i);
  this->print("\x0a"); // Line Feed
}

void lcd_serial::println(int i)
{
  this->print(i);
  this->print("\x0a"); // Line Feed
}
