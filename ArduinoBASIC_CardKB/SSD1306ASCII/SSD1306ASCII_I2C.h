/*

    @file SSD1306ASCII_I2C.h
    @brief SSD1306ASCII OLED 128x32 
    Reference source:https://github.com/robinhedwards/ArduinoBASIC

    @author Kei Takagi
    @date 2019.6.8

    Copyright (c) 2019 Kei Takagi

*/

#ifndef SSD1306ASCII_I2C_h
#define SSD1306ASCII_I2C_h

#include "Arduino.h"

//#define OLED_DISPLAY

#define WIRE_BUFMAX 32
#define OLED_ADDR   0x3C
#define OLED_WIDTH  128
#define OLED_HEIGHT 32
#define OLED_COLMAX 21
#define OLED_ROWMAX 4

#define BLACK       0 ///< Draw 'off' pixels
#define WHITE       1 ///< Draw 'on' pixels
#define INVERSE     2 ///< Invert pixels

class SSD1306ASCII : public Print {
 public:
  void clear();
  void init();
  void home() {setCursor(0, 0);}
  uint8_t col() {return col_;}
  uint8_t row() {return row_;}
  void setCursor(uint8_t col, uint8_t row);
  void commandList(const uint8_t *c, uint8_t n);
  size_t write(uint8_t c);
  size_t write(const char* s);

 private:
  // cursor position
  int8_t col_, row_;
};
#endif
