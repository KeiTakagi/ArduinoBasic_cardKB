/*
    @file host.cpp
    @brief Modified Arduino Basic to work with cardKeyboard and I2C OLED 128x32 or 128x64.
    Reference source:https://github.com/robinhedwards/ArduinoBASIC

    @author Kei Takagi
    @date 2019.11.26

    Copyright (c) 2019 Kei Takagi
*/

#include <SSD1306ASCII_I2C.h>
#include <EEPROM.h>
#include "host.h"
#include "basic.h"
#include "cardkb.h"

extern SSD1306ASCII oled;
extern EEPROMClass EEPROM;
int timer1_counter;

byte screenBuffer[OLED_COLMAX * OLED_ROWMAX];
byte lineDirty[OLED_ROWMAX];
uint8_t curX = 0, curY = 0;
volatile boolean flash = false;
byte inputMode = 0;
byte inkeyChar = 0;

const char bytesFreeStr[] PROGMEM = "bytes free";


void initTimer() {
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  timer1_counter = 49911;   // preload timer 65536-8MHz/256/2Hz
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

ISR(TIMER1_OVF_vect) {      // interrupt service routine
  TCNT1 = timer1_counter;   // preload timer
  flash = !flash;
}

void host_init() {
#if BUZZER
  pinMode(BUZZER, OUTPUT);
#endif
  initTimer();
}

void host_sleep(long ms) {
  delay(ms);
}

void host_digitalWrite(int pin, int state) {
  digitalWrite(pin, state ? HIGH : LOW);
}

int host_digitalRead(int pin) {
  return digitalRead(pin);
}

int host_analogRead(int pin) {
  return analogRead(pin);
}

void host_pinMode(int pin, int mode) {
  pinMode(pin, mode);
}

void host_click() {
#if BUZZER
  digitalWrite(BUZZER, HIGH);
  delay(1);
  digitalWrite(BUZZER, LOW);
#endif
}

void host_cls() {
  memset(screenBuffer, 0x20, OLED_COLMAX * OLED_ROWMAX);
  memset(lineDirty, 1, OLED_ROWMAX);
  curX = 0;
  curY = 0;
}

void host_startupTone() {
#if BUZZER
  uint8_t i, j;
  for (i = 1; i <= 2; i++) {
    for (j = 0; j < 50 * i; j++) {
      digitalWrite(BUZZER, HIGH);
      delay(3 - i);
      digitalWrite(BUZZER, LOW);
      delay(3 - i);
    }
    delay(100);
  }
#endif
}

void host_moveCursor(uint8_t x, uint8_t y) {
  if (x < 0) x = 0;
  if (x >= OLED_COLMAX) x = OLED_COLMAX - 1;
  if (y < 0) y = 0;
  if (y >= OLED_ROWMAX) y = OLED_ROWMAX - 1;
  curX = x;
  curY = y;
}

void host_showBuffer() {
  uint8_t x, y;
  for ( y = 0; y < OLED_ROWMAX; y++) {
    if (lineDirty[y] || (inputMode && y == curY)) {
      oled.setCursor(0, y);
      for ( x = 0; x < OLED_COLMAX; x++) {
        char c = screenBuffer[y * OLED_COLMAX + x];
        if (c < 0x20) c = ' ';
        if (x == curX && y == curY && inputMode && flash) c = 0x7f; //Cursor blink
        oled.print(c);
      }
      lineDirty[y] = 0;
    }
  }
}

void scrollBuffer() {
  memcpy(screenBuffer, screenBuffer + OLED_COLMAX, OLED_COLMAX * (OLED_ROWMAX - 1));
  memset(screenBuffer + OLED_COLMAX * (OLED_ROWMAX - 1), 0x20, OLED_COLMAX);
  memset(lineDirty, 1, OLED_ROWMAX);
  curY--;
}

void host_outputString(char *str) {
  uint8_t pos = curY * OLED_COLMAX + curX;
  while (*str) {
    lineDirty[pos / OLED_COLMAX] = 1;
    if (pos >= OLED_COLMAX * OLED_ROWMAX) {
      scrollBuffer();
      pos -= OLED_COLMAX;
    }
    screenBuffer[pos++] = *str++;
  }
  curX = pos % OLED_COLMAX;
  curY = pos / OLED_COLMAX;
}

void host_outputProgMemString(const char *p) {
  while (1) {
    uint8_t c = pgm_read_byte(p++);
    if (c == 0) break;
    host_outputChar(c);
  }
}

void host_outputChar(char c) {
  host_outputChar(c, false);
}
void host_outputChar(char c, bool pause) {
  uint8_t pos = curY * OLED_COLMAX + curX;
  lineDirty[pos / OLED_COLMAX] = 1;
  screenBuffer[pos++] = c;
  if (pos >= OLED_COLMAX * OLED_ROWMAX) {
    host_showBuffer();
    if (pause) {
      while (1) {
        if (getChar(0) != 0x00)break;
      }
    }
    scrollBuffer();
    pos -= OLED_COLMAX;
  }
  curX = pos % OLED_COLMAX;
  curY = pos / OLED_COLMAX;
}

int host_outputInt(long num) {
  // returns len
  long i = num, xx = 1;
  int c = 0;
  do {
    c++;
    xx *= 10;
    i /= 10;
  }
  while (i);

  for ( i = 0; i < c; i++) {
    xx /= 10;
    host_outputChar(((num / xx) % 10) + '0');
  }
  return c;
}

char *host_floatToStr(float f, char *buf) {
  // floats have approx 7 sig figs
  float a = fabs(f);
  if (f == 0.0f) {
    buf[0] = '0';
    buf[1] = 0;
  }
  else if (a < 0.0001 || a > 1000000) {
    // this will output -1.123456E99 = 13 characters max including trailing nul
    dtostre(f, buf, 6, 0);
  }
  else {
    int decPos = 7 - (int)(floor(log10(a)) + 1.0f);
    dtostrf(f, 1, decPos, buf);
    if (decPos) {
      // remove trailing 0s
      char *p = buf;
      while (*p) p++;
      p--;
      while (*p == '0') {
        *p-- = 0;
      }
      if (*p == '.') *p = 0;
    }
  }
  return buf;
}

void host_outputFloat(float f) {
  char buf[16];
  host_outputString(host_floatToStr(f, buf));
}

void host_newLine() {
  host_newLine(false);
}
void host_newLine(bool pause) {
  curX = 0;
  curY++;
  if (curY == OLED_ROWMAX) {
    host_showBuffer();
    if (pause) {
      while (1) {
        if (getChar(0) != 0x00)break;
      }
    }
    scrollBuffer();
  }
  memset(screenBuffer + OLED_COLMAX * (curY), 0x20, OLED_COLMAX);
  lineDirty[curY] = 1;
}

char *host_readLine() {
  inputMode = 1;

  if (curX == 0) memset(screenBuffer + OLED_COLMAX * (curY), 0x20, OLED_COLMAX);
  else host_newLine();

  uint8_t startPos = curY * OLED_COLMAX + curX;
  uint8_t pos = startPos;

  bool done = false;
  char c;
  while (!done) {
    while (c = getChar(200)) {
      host_click();
      // read the next key
      lineDirty[pos / OLED_COLMAX] = 1;
      if ( 0x20 <= c && c < 0x7f)
        screenBuffer[pos++] = c;
      else if (c == 0x08 && pos > startPos) //DELETE
        screenBuffer[--pos] = 0;
      else if (c == 0x0D) // ENTER
        done = true;
      curX = pos % OLED_COLMAX;
      curY = pos / OLED_COLMAX;
      // scroll if we need to
      if (curY == OLED_ROWMAX) {
        if (startPos >= OLED_COLMAX) {
          startPos -= OLED_COLMAX;
          pos -= OLED_COLMAX;
          scrollBuffer();
        }
        else
        {
          screenBuffer[--pos] = 0;
          curX = pos % OLED_COLMAX;
          curY = pos / OLED_COLMAX;
        }
      }
      host_showBuffer();
    }
    host_showBuffer();  //Cursor blink
  }
  screenBuffer[pos] = 0;
  inputMode = 0;
  // remove the cursor
  lineDirty[curY] = 1;
  host_showBuffer();
  return &screenBuffer[startPos];
}

char host_getKey() {
  char c = inkeyChar;
  inkeyChar = 0;
  if (0x20 <= c && c < 0x7f)
    return c;
  else return 0;
}

bool host_ESCPressed() {
  inkeyChar = getChar(0);
  if (inkeyChar == 0x1B)//ESC
    return true;
  else
    return false;
}

void host_outputFreeMem(uint16_t val)
{
  host_newLine(true);
  host_outputInt(val);
  host_outputChar(' ');
  host_outputProgMemString(bytesFreeStr);
}

void host_saveProgram(bool autoexec) {
  EEPROM.write(0, autoexec ? MAGIC_AUTORUN_NUMBER : 0x00);
  EEPROM.write(1, sysPROGEND & 0xFF);
  EEPROM.write(2, (sysPROGEND >> 8) & 0xFF);
  for (int i = 0; i < sysPROGEND; i++)
    EEPROM.write(3 + i, mem[i]);
}

void host_loadProgram() {
  // skip the autorun byte
  sysPROGEND = EEPROM.read(1) | (EEPROM.read(2) << 8);
  for (int i = 0; i < sysPROGEND; i++)
    mem[i] = EEPROM.read(i + 3);
}

void host_LED(uint8_t r,uint8_t g,uint8_t b){
  flashOn(r,g,b);
}
void host_Img(uint8_t x,uint8_t y,uint8_t *imgBuff){
  oled.setCursor(x,y);
  oled.setimg(imgBuff);
}

//-----------------------------------------------------------------------------

#if EXTERNAL_EEPROM
#include <Wire.h>

void writeExtEEPROM(uint16_t address, uint8_t data) {
  uint8_t i2caddr = (uint8_t)EXTERNAL_EEPROM_ADDR | (uint8_t)(address >> 16);
  Wire.beginTransmission(i2caddr);
  Wire.write((byte)(address >> 8));   // MSB
  Wire.write((byte)(address & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();
  delay(5);
}

byte readExtEEPROM(uint16_t address)
{
  uint8_t i2caddr = (uint8_t)EXTERNAL_EEPROM_ADDR | (uint8_t)(address >> 16);
  Wire.beginTransmission(i2caddr);
  Wire.write((byte)(address >> 8));   // MSB
  Wire.write((byte)(address & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(i2caddr, (uint8_t)1);
  byte b = Wire.read();
  return b;
}

// get the EEPROM address of a file, or the end if fileName is null
uint16_t getExtEEPROMAddr(char *fileName) {
  uint16_t addr = 0;
  while (1) {
    uint16_t len = readExtEEPROM(addr) | (readExtEEPROM(addr + 1) << 8);
    if (len == 0) break;
    if (fileName) {
      bool found = true;
      for (int i = 0; i <= strlen(fileName); i++) {
        if (fileName[i] != readExtEEPROM(addr + 2 + i)) {
          found = false;
          break;
        }
      }
      if (found) return addr;
    }
    addr += len;
  }
  return fileName ? EXTERNAL_EEPROM_SIZE : addr;
}

void host_directoryExtEEPROM() {
  uint16_t addr = 0;
  while (1) {
    uint16_t len = readExtEEPROM(addr) | (readExtEEPROM(addr + 1) << 8);
    if (len == 0) break;
    int i = 0;
    while (1) {
      char ch = readExtEEPROM(addr + 2 + i);
      if (!ch) break;
      host_outputChar(readExtEEPROM(addr + 2 + i), true);
      i++;
    }
    addr += len;
    host_outputChar(' ', true);
  }
  host_outputFreeMem(EXTERNAL_EEPROM_SIZE - addr - 2);
}

bool host_removeExtEEPROM(char *fileName) {
  uint16_t addr = getExtEEPROMAddr(fileName);
  if (addr == EXTERNAL_EEPROM_SIZE) return false;
  uint16_t len = readExtEEPROM(addr) | (readExtEEPROM(addr + 1) << 8);
  uint16_t last = getExtEEPROMAddr(NULL);
  uint16_t count = 2 + last - (addr + len);
  while (count--) {
    byte b = readExtEEPROM(addr + len);
    writeExtEEPROM(addr, b);
    addr++;
  }
  return true;
}

bool host_loadExtEEPROM(char *fileName) {
  uint16_t addr = getExtEEPROMAddr(fileName);
  if (addr == EXTERNAL_EEPROM_SIZE) return false;

  // skip filename
  addr += 2;
  while (readExtEEPROM(addr++)) ;
  sysPROGEND = readExtEEPROM(addr) | (readExtEEPROM(addr + 1) << 8);
  for (uint16_t i = 0; i < sysPROGEND; i++) {
    mem[i] = readExtEEPROM(addr + 2 + i);
  }
  return true;
}

bool host_saveExtEEPROM(char *fileName) {
  uint16_t i;
  uint16_t addr = getExtEEPROMAddr(fileName);
  if (addr != EXTERNAL_EEPROM_SIZE)
    host_removeExtEEPROM(fileName);
  addr = getExtEEPROMAddr(NULL);
  uint8_t fileNameLen = strlen(fileName);
  uint16_t len = 2 + fileNameLen + 1 + 2 + sysPROGEND;
  if ((uint16_t)EXTERNAL_EEPROM_SIZE - addr - len - 2 < 0)
    return false;

  // write overall length
  writeExtEEPROM(addr++, len & 0xFF);
  writeExtEEPROM(addr++, (len >> 8) & 0xFF);

  // write filename
  for ( i = 0; i < strlen(fileName); i++)
    writeExtEEPROM(addr++, fileName[i]);
  writeExtEEPROM(addr++, 0);

  // write length & program
  writeExtEEPROM(addr++, sysPROGEND & 0xFF);
  writeExtEEPROM(addr++, (sysPROGEND >> 8) & 0xFF);
  for ( i = 0; i < sysPROGEND; i++)
    writeExtEEPROM(addr++, mem[i]);

  // 0 length marks end
  writeExtEEPROM(addr++, 0);
  writeExtEEPROM(addr++, 0);
  return true;
}

#endif
