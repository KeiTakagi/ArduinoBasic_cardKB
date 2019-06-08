// Display all characters in the font

#define VIEW_DELAY 10000

#include <SSD1306ASCII_I2C.h>

SSD1306ASCII oled;
//------------------------------------------------------------------------------
void setup() {
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  oled.init();
}
//------------------------------------------------------------------------------
void loop() {
  uint8_t row = 0;
  uint8_t c = 0;
  oled.clear();
  while (1) {
    for ( uint8_t i = 0; i < 16; i++) {
      oled.write(c++);
    }
    if (++row == OLED_HEIGHT / 8) {
      delay(VIEW_DELAY);
      row = 0;
      oled.clear();
    }
    oled.setCursor(0, row);
  }
  delay(VIEW_DELAY);
}