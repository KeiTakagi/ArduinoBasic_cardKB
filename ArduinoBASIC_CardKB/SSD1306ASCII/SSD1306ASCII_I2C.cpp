/*

    @file SSD1306ASCII_I2C.cpp
    @brief SSD1306ASCII I2C OLED 128x32 or 128x64.
    Reference source:https://github.com/robinhedwards/ArduinoBASIC

    @author Kei Takagi
    @date 2020.02.27

    Copyright (c) 2019 Kei Takagi

*/

#include <SSD1306ASCII_I2C.h>
#include <Wire.h>
#include "font5x7.c"

//------------------------------------------------------------------------------
// clear the screen
void SSD1306ASCII::clear() {
  uint8_t wkCnt;
  for (row_ = 0; row_ < OLED_ROWMAX; row_++) {
    setCursor(0, row_);
    col_ = OLED_WIDTH;
    wkCnt = 1;
    while (col_--) {
      if (wkCnt == 1) {
        Wire.endTransmission();
        Wire.beginTransmission(OLED_ADDR);
        Wire.write((uint8_t)0x40);
        wkCnt = WIRE_BUFMAX;
      }
      Wire.write(0x00);
      wkCnt--;
    }
    Wire.endTransmission();
  }
  col_ = 0;
  row_ = 0;
}
//------------------------------------------------------------------------------
// send a command
void SSD1306ASCII::commandList(const uint8_t *c, uint8_t n) {
  uint8_t wkCnt = 1;
  while (n--) {
    if (wkCnt <= 1) {
      Wire.endTransmission();
      Wire.beginTransmission(OLED_ADDR);
      Wire.write((uint8_t)0X80);
      wkCnt = WIRE_BUFMAX;
    }
    Wire.write(pgm_read_byte(c++));
    wkCnt--;
  }
  Wire.endTransmission();
}
//------------------------------------------------------------------------------
void SSD1306ASCII::init() {
  Wire.begin();
  Wire.setClock(400000);
  // Init sequence
  static const uint8_t PROGMEM init_seq[] = {

    0xAE, // Display OFF (sleep mode)
    0x00,
    0x81, // set Contrast Control for BANK0
    0x60, //  -> [contrast]
    0x40, // Set Display Start Line 0x40-0x7F

#if OLED_DISPLAY == 0
    0xA0, // Display mode right/left - Nomal:0xA0 / mirrored writing:0xA1
    0xC0, // Display mode up/down - Nomal:0xC0 / reverse:0xC8
#else
    0xA1, // Display mode right/left - Nomal:0xA0 / mirrored writing:0xA1
    0xC8, // Display mode up/down - Nomal:0xC0 / reverse:0xC8
#endif

#if OLED_ROWMAX == 4
    0xA8, // Set Multiplex Ratio  0xA8, 0x1F
    0x1F, //  -> 0x1F - 0x3F
    0x22, // Set page address
    0x00, //  -> Page start address 0
    0x04, //  -> 0x04:Page end address 4  0x07:Page end address 7
#else
    0x22, // Set page address
    0x00, //  -> Page start address 0
    0x07, //  -> 0x04:Page end address 4  0x07:Page end address 7
    0xA8, // Set Multiplex Ratio  0xA8, 0x3F
    0x3F, //  -> 0x1F - 0x3F
#endif
    0xD3, // Display offset(D3h, 00h)
    0x00, //  -> Set vertical shift by COM from 0-63 The value is reset to 0x00 after RESET.
    0xDA, // COM pins
    0x00, //  -> 0x2 ada 0x12
    0x10, // Page addressing mode
    0x20, // Set memory addressing mode
    0x00, // Horizontal addressing mode
    0x21, // Set column address
    0x00, //  -> Column start address 0
    0x7F, //  -> Column end address 127
    0x8D, // Enable charge pump regulator
    0x14, //  -> ON
    0xD5, // Set Display Clock Divide Ratio/Oscillator Frequency 0xD5, 0x80
    0x80, //  -> B10000000
    0x2E, // Deactivate scroll
    0xA4, // Disable Entire Display ON
    0xAF, // Display On 0xAF
    0xA6  // Normal Display 0xA6, Inverse display 0xA7

  };
  commandList(init_seq, sizeof(init_seq));
  clear();
}

//------------------------------------------------------------------------------
void SSD1306ASCII::setCursor(uint8_t col, uint8_t row) {
  row > OLED_ROWMAX ? row = OLED_ROWMAX : row_ = row;
  col > OLED_COLMAX ? col = OLED_COLMAX : col_ = col;

  // convert character column to display column
  col *= 6;
  col += 2;

  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);
  Wire.write(0xB0 + row);
  Wire.write(0x00);
  Wire.write((col >> 4) | 0x10);
  Wire.write(0x00);
  Wire.write(col & 0x0f);
  Wire.endTransmission();
}
//------------------------------------------------------------------------------
void SSD1306ASCII::setimg(const uint8_t* c) {
  uint8_t i;
  for (i = 0; i < 6; i++) {
    Wire.endTransmission();
    Wire.beginTransmission(OLED_ADDR);
    Wire.write((uint8_t)0x40);
    Wire.write(*(c + i));
  }
  Wire.endTransmission();
}
//------------------------------------------------------------------------------
size_t SSD1306ASCII::write(const uint8_t c) {
  uint8_t i;
  uint8_t buf[6];
  if ( c < 0x20 || 0x7F < c)return 0;
  if (col_ >= OLED_COLMAX) return 0;
  setCursor(col_, row_);
  for (i = 0; i < 5; i++)
    buf[i] = pgm_read_byte(&font[(c - 0x20) * 5 + i]);
  buf[i] = 0x00;
  setimg(buf);
  Wire.endTransmission();
  col_++;
  return 1;
}
//------------------------------------------------------------------------------
size_t SSD1306ASCII::write(const char* s) {
  size_t i=0;
  while(s[i])write(s[i++]);
  return i;
}
