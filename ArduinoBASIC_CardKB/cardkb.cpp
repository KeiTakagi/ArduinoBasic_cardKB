/*
    @file cardkb.cpp
    @brief cardKeyboard
    
    @author Kei Takagi
    @date 2019.8.8

    Copyright (c) 2019 Kei Takagi
*/

#include <Adafruit_NeoPixel.h>
#include "cardkb.h"

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

uint16_t idle = 0;
uint8_t _shift = 0, _fn = 0, _sym = 0;
uint8_t KEY = 0, hadPressed = 0;
uint8_t Mode = 0; //0->normal.1->shift 2->long_shift, 3->sym, 4->long_shift 5->fn,6->long_fn

void flashOn(byte r, byte g, byte b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b)); pixels.show();
}

void keybordSetup(void) {
  pinMode(A3, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A0, OUTPUT);

  DDRB = 0x00;
  PORTB = 0xff;
  DDRD = 0x00;
  PORTD = 0xff;

  pixels.begin();
  for (byte j = 0; j < 3; j++) {
    for (byte i = 0; i < 5; i++) {
      flashOn(i, i, i);
      delay(10);
    }
    for (byte i = 5; i > 0; i--) {
      flashOn(i, i, i);
      delay(10);
    }
  }
  flashOn(0, 0, 0);
}

byte getInput(void) {
  byte ret = 0;
  byte i, j;
  hadPressed = 1;
  for (i = 0; i < 4; i++) {
    digitalWrite(A3, (0b00001110 >> i) & 0b00000001);
    digitalWrite(A2, (0b00001101 >> i) & 0b00000001);
    digitalWrite(A1, (0b00001011 >> i) & 0b00000001);
    digitalWrite(A0, (0b00000111 >> i) & 0b00000001);
    delay(2);
    for (j = 0; j < 8; j++) {
      ret++;
      if (PIND == pgm_read_byte(&pinDmap[j])) {
        while (PIND != 0xff) {
          flashOn(3, 3, 3);
        }
        flashOn(0, 0, 0);
        return ret;
      }
    }
    for (j = 0; j < 4; j++) {
      ret++;
      if (PINB == pgm_read_byte(&pinBmap[j])) {
        while (PINB != 223) {
          flashOn(3, 3, 3);
        }
        flashOn(0, 0, 0);
        return ret;
      }
    }
  }
  hadPressed = 0;
  return 0;
}

byte getChar(void)
{
  byte c = 0;

  if (shiftPressed) {
    _sym = 0; _fn = 0; idle = 0;
    while (shiftPressed)delay(1);
    if (_shift == 0) {
      delay(200);
      if (shiftPressed) {
        while (shiftPressed)delay(1);
        _shift = 2;
        Mode = 2;
      } else {
        _shift = 1;
        Mode = 1;
      }
    } else {
      delay(200);
      if (shiftPressed) {
        while (shiftPressed)delay(1);
        if (_shift == 2) {
          Mode = 0;
          _shift = 0;
        } else {
          Mode = 2;
          _shift = 2;
        }
      } else {
        Mode = 0;
        _shift = 0;
      }
    }
  }

  if (symPressed) {
    _shift = 0; _fn = 0; idle = 0;
    while (symPressed)delay(1);
    if (_sym == 0) {
      delay(200);
      if (symPressed) {
        while (symPressed)delay(1);
        _sym = 2;
        Mode = 4;
      } else {
        _sym = 1;
        Mode = 3;
      }
    } else {
      delay(200);
      if (symPressed) {
        while (symPressed)delay(1);
        if (_sym == 2) {
          Mode = 0;
          _sym = 0;
        } else {
          Mode = 4;
          _sym = 2;
        }
      } else {
        Mode = 0;
        _sym = 0;
      }
    }
  }

  if (fnPressed) {
    _sym = 0; _shift = 0; idle = 0;
    while (fnPressed)delay(1);
    if (_fn == 0) {
      delay(200);
      if (fnPressed) {
        while (fnPressed)delay(1);
        _fn = 2;
        Mode = 6;
      } else {
        _fn = 1;
        Mode = 5;
      }
    } else {
      delay(200);
      if (fnPressed) {
        while (fnPressed)delay(1);
        if (_fn == 2) {
          Mode = 0;
          _fn = 0;
        } else {
          Mode = 6;
          _fn = 2;
        }
      } else {
        Mode = 0;
        _fn = 0;
      }
    }
  }
  switch (Mode) {
    case 0://normal
      flashOn(0, 0, 0); break;
    case 1://shift
      if ((idle / 6) % 2 == 1) {
        flashOn(0, 0, 0);
      } else {
        flashOn(5, 0, 0);
      }
      break;
    case 2://long_shift
      flashOn(5, 0, 0); break;
    case 3://sym
      if ((idle / 6) % 2 == 1) {
        flashOn(0, 0, 0);
      } else {
        flashOn(5, 0, 0);
      }
      break;
    case 4://long_sym
      flashOn(0, 5, 0); break;
    case 5://fn
      if ((idle / 6) % 2 == 1) {
        flashOn(0, 0, 0);
      } else {
        flashOn(0, 0, 5);
      }
      break;
    case 6://long_fn
      flashOn(0, 0, 5); break;
  }
  if (hadPressed == 0) {
    KEY = getInput();
    if (hadPressed == 1) {
      c = pgm_read_byte(&KeyMap[KEY - 1][Mode]);
      if ((Mode == 1) || (Mode == 3) || (Mode == 5)) {
        Mode = 0;
        _shift = 0;
        _sym = 0;
        _fn = 0;
      }
      hadPressed = 0;
    }
  }
  idle++;
  return c;
}
