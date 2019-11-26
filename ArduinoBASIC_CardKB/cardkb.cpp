/*
    @file cardkb.cpp
    @brief cardKeyboard

    @author Kei Takagi
    @date 2019.11.26

    Copyright (c) 2019 Kei Takagi
*/

#include <Adafruit_NeoPixel.h>
#include "cardkb.h"

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

bool    idle = false;
uint8_t _shift = 0, _fn = 0, _sym = 0;
uint8_t KEY = 0, hadPressed = 0;
uint8_t Mode = 0; //0->normal.1->shift 2->long_shift, 3->sym, 4->long_shift 5->fn,6->long_fn

void flashOn(byte r, byte g, byte b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b)); pixels.show();
}

void keybordSetup(void) {
  uint8_t i, j;
  pinMode(A3, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A0, OUTPUT);

  DDRB = 0x00;
  PORTB = 0xff;
  DDRD = 0x00;
  PORTD = 0xff;

  pixels.begin();
  for ( j = 0; j < 3; j++) {
    for ( i = 0; i < 5; i++) {
      flashOn(i, i, i);
      delay(10);
    }
    for ( i = 5; i > 0; i--) {
      flashOn(i, i, i);
      delay(10);
    }
  }
  flashOn(0, 0, 0);
}

byte getInput(uint8_t delay_time) {
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
        flashOn(3, 3, 3);
        delay(delay_time);
        flashOn(0, 0, 0);
        return ret;
      }
    }
    for (j = 0; j < 4; j++) {
      ret++;
      if (PINB == pgm_read_byte(&pinBmap[j])) {
        flashOn(3, 3, 3);
        delay(delay_time);
        flashOn(0, 0, 0);
        return ret;
      }
    }
  }
  hadPressed = 0;
  return 0;
}

byte getChar(uint8_t delay_time)
{
  byte c = 0;
  if (shiftPressed) {
    if (Mode > 0) {
      _shift = 0;
      Mode = 0;
      delay(200);
    } else {
      if (_shift < LONGPRESSEDTIME)_shift++;
      _sym = 0; _fn = 0; idle = false;
      return 0;
    }
  }
  if ( _shift == LONGPRESSEDTIME) {
    //long shift
    Mode = 2;
  } else if ( 0 < _shift) {
    // shift
    Mode = 1;
  }

  if (symPressed) {
    if (Mode > 0) {
      _sym = 0;
      Mode = 0;
      delay(200);
    } else {
      if (_sym < LONGPRESSEDTIME)_sym++;
      _shift = 0; _fn = 0; idle = false;
      return 0;
    }
  }
  if ( _sym == LONGPRESSEDTIME) {
    //long sym
    Mode = 4;
  } else if ( 0 < _sym) {
    // sym
    Mode = 3;
  }

  if (fnPressed) {
    if (Mode > 0) {
      _fn = 0;
      Mode = 0;
      delay(200);
    } else {
      if (_fn < LONGPRESSEDTIME)_fn++;
      _shift = 0; _sym = 0; idle = false;
      return 0;
    }
  }
  if ( _fn == LONGPRESSEDTIME) {
    //long fn
    Mode = 6;
  } else if ( 0 < _fn) {
    // fn
    Mode = 5;
  }
  
  switch (Mode) {
    case 0://normal
      flashOn(0, 0, 0); break;
    case 1://shift
      if (idle) {
        flashOn(0, 0, 0);
      } else {
        flashOn(5, 0, 0);
      }
      break;
    case 2://long_shift
      flashOn(5, 0, 0); break;
    case 3://sym
      if (idle) {
        flashOn(0, 0, 0);
      } else {
        flashOn(0, 5, 0);
      }
      break;
    case 4://long_sym
      flashOn(0, 5, 0); break;
    case 5://fn
      if (idle) {
        flashOn(0, 0, 0);
      } else {
        flashOn(0, 0, 5);
      }
      break;
    case 6://long_fn
      flashOn(0, 0, 5); break;
  }

  if (hadPressed == 0) {
    KEY = getInput(delay_time);
    if (hadPressed == 1) {
      c = pgm_read_byte(&KeyMap[KEY - 1][Mode]);
      if ((Mode == 1) || (Mode == 3) || (Mode == 5)) {
        Mode = false;
        _shift = 0;
        _sym = 0;
        _fn = 0;
      }
      hadPressed = 0;
    }
  }
  idle=!idle;
  return c;
}
