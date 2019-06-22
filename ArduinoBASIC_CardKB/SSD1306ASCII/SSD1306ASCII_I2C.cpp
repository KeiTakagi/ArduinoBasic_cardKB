/*

    @file SSD1306ASCII_I2C.cpp
    @brief SSD1306ASCII OLED 128x32 
    Reference source:https://github.com/robinhedwards/ArduinoBASIC

    @author Kei Takagi
    @date 2019.6.8

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
    0xAE,                   // Display OFF (sleep mode) 
    0x00,                   // 0Set Lower Column Start Address for Page Addressing Mode 
    0x10,                   // Set Higher Column Start Address for Page Addressing Mode 
    0x40,                   // Set Display Start Line
    0xB0,                   // 
    0x81,                   // set Contrast Control for BANK0
    0x60,                   // [contrast]
    0xA0,                   // segment remap  0XA1:mirrored writing 
    0xA8,                   // multiplex ratio
    0x1F,                   // duty = 1/32
    0x32,                   // 0X30---0X33  set VPP   8V
    0xC0,                   // Com scan direction   0XC8
    0xD3,                   // Display offset(D3h, 00h)
    0x00,                   //
    0xD9,                   // Set Pre-charge Period 
    0x22,                   // 0xf1
    0xDA,                   // COM pins
    0x02,                   // 0x2 ada x12
    0xdb,                   // Set VCOMH Deselect Level
    0x40,                   //
    0x8d,                   // Enable charge pump regulator
    0x14,
    0xAF,                   // Display ON
    0xA6                    // Normal display (RESET)
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
size_t SSD1306ASCII::write(uint8_t c) {
  uint8_t i;
  if ( c < 0x20 || 0x7F < c)return 0;
  if (col_ >= OLED_COLMAX) return 0;
  setCursor(col_, row_);
	for (i = 0; i < 5; i++) {
    Wire.endTransmission();
    Wire.beginTransmission(OLED_ADDR);
    Wire.write((uint8_t)0x40);
    Wire.write(pgm_read_byte(&font[(c - 0x20) * 5 + i]));
  }
  Wire.endTransmission();
  col_++;

	return 1;
}
//------------------------------------------------------------------------------
size_t SSD1306ASCII::write(const char* s) {
  size_t i ;
  for (i = 0; i < strlen(s); i++)
    write(s[i]);
  return i;
}

