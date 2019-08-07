/*
    @file host.h
    @brief Modified Arduino Basic to work with cardKeyboard and I2C OLED 128x32 or 128x64.
    Reference source:https://github.com/robinhedwards/ArduinoBASIC

    @author Kei Takagi
    @date 2019.7.15

    Copyright (c) 2019 Kei Takagi
*/

#ifndef _HOST_H
#define _HOST_H

#include <stdint.h>

// BUZZER 0...BUZZER NONE PinNo...USE BUZZER PinNo
#define BUZZER                  0

#define EXTERNAL_EEPROM         1
#define EXTERNAL_EEPROM_ADDR    0x50    // I2C address (7 bits)
#define EXTERNAL_EEPROM_SIZE    32768   // only <=32k tested (64k might work?)

#define MAGIC_AUTORUN_NUMBER    0xFC

void host_init(void);
void host_sleep(long ms);
void host_digitalWrite(int pin, int state);
int host_digitalRead(int pin);
int host_analogRead(int pin);
void host_pinMode(int pin, int mode);
void host_click();
void host_startupTone();
void host_cls();
void host_showBuffer();
void host_moveCursor(uint8_t x, uint8_t y);
void host_outputString(char *str);
void host_outputProgMemString(const char *str);
void host_outputChar(char c);
void host_outputFloat(float f);
char *host_floatToStr(float f, char *buf);
int host_outputInt(long val);
void host_newLine();
char *host_readLine();
char host_getKey();
bool host_ESCPressed();
void host_outputFreeMem(unsigned int val);
void host_saveProgram(bool autoexec);
void host_loadProgram();


#if EXTERNAL_EEPROM
void writeExtEEPROM(uint16_t address, uint8_t data);
void host_directoryExtEEPROM();
bool host_saveExtEEPROM(char *fileName);
bool host_loadExtEEPROM(char *fileName);
bool host_removeExtEEPROM(char *fileName);
#endif

#endif
