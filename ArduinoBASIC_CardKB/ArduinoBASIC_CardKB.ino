/*
    @file ArduinoBASIC_CardKB.ino
    @brief Modified Arduino Basic to work with cardKeyboard and OLED 128x32.
    Reference source:https://github.com/robinhedwards/ArduinoBASIC

    @author Kei Takagi
    @date 2019.11.26

    Copyright (c) 2019 Kei Takagi
*/

#include <font.h>
#include <SSD1306ASCII_I2C.h>

#include <EEPROM.h>

#include "basic.h"
#include "host.h"
#include "cardkb.h"

// Define in host.h if using an external EEPROM e.g. 24LC256
// Should be connected to the I2C pins
// SDA -> Analog Pin 4, SCL -> Analog Pin 5
// See e.g. http://www.hobbytronics.co.uk/arduino-external-eeprom

// If using an external EEPROM, you'll also have to initialise it by
// running once with the appropriate lines enabled in setup() - see below

// buzzer pin, 0 = disabled/not present
#define BUZZER_PIN    0

// OLED
SSD1306ASCII oled;

uint8_t mem[MEMORY_SIZE];
#define TOKEN_BUF_SIZE    64
uint8_t tokenBuf[TOKEN_BUF_SIZE];

const char welcomeStr[] PROGMEM = "Arduino BASIC";
uint8_t autorun = 0;

void setup() {
  keybordSetup();
  oled.init();
  oled.clear();

  reset();
  host_init();
  host_cls();
  host_outputProgMemString(welcomeStr);
  // show memory size
  host_outputFreeMem(sysVARSTART - sysPROGEND);
  host_showBuffer();

  // IF USING EXTERNAL EEPROM
  // The following line 'wipes' the external EEPROM and prepares
  // it for use. Uncomment it, upload the sketch, then comment it back
  // in again and upload again, if you use a new EEPROM.
  // writeExtEEPROM(0,0); writeExtEEPROM(1,0);

  if (EEPROM.read(0) == MAGIC_AUTORUN_NUMBER)
    autorun = 1;
  else
    host_startupTone();
}

void loop() {
  int ret = ERROR_NONE;

  if (!autorun) {
    // get a line from the user
    char *input = host_readLine();
    // special editor commands
    if (input[0] == '?' && input[1] == 0) {
      host_outputFreeMem(sysVARSTART - sysPROGEND);
      host_showBuffer();
      return;
    }
    // otherwise tokenize
    ret = tokenize((unsigned char*)input, tokenBuf, TOKEN_BUF_SIZE);
  }
  else {
    host_loadProgram();
    tokenBuf[0] = TOKEN_RUN;
    tokenBuf[1] = 0;
    autorun = 0;
  }
  // execute the token buffer
  if (ret == ERROR_NONE) {
    host_newLine();
    ret = processInput(tokenBuf);
  }
  if (ret != ERROR_NONE) {
    host_newLine();
    if (lineNumber != 0) {
      host_outputInt(lineNumber);
      host_outputChar('-');
    }
    host_outputProgMemString((char *)pgm_read_word(&(errorTable[ret])));
  }
}
