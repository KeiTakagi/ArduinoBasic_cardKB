// Simple Hello world demo

#include <SSD1306ASCII_I2C.h>

SSD1306ASCII oled;
//------------------------------------------------------------------------------
void setup() {
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  oled.init();

  oled.clear();
  oled.print("Hello world!");

  oled.setCursor(0, 1);
  oled.write("A long line is truncated");

  oled.setCursor(5, 3);
  double x = 123.45;
  oled.write("x = ");
  oled.print(x);
}
//------------------------------------------------------------------------------
void loop() {}