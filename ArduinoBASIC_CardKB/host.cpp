/*
    @file host.cpp
    @brief Modified Arduino Basic to work with cardKeyboard and OLED 128x32.
    Reference source:https://github.com/robinhedwards/ArduinoBASIC

    @author Kei Takagi
    @date 2019.6.8

    Copyright (c) 2019 Kei Takagi
*/

#include <Adafruit_NeoPixel.h>
#include <SSD1306ASCII_I2C.h>
#include <EEPROM.h>
#include "host.h"
#include "basic.h"

extern SSD1306ASCII oled;
extern EEPROMClass EEPROM;
int timer1_counter;

char screenBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
char lineDirty[SCREEN_HEIGHT];
int curX = 0, curY = 0;
volatile char flash = 0, redraw = 0;
char inputMode = 0;
char inkeyChar = 0;
char buzPin = 0;

const char bytesFreeStr[] PROGMEM = "bytes free";

uint16_t idle = 0;
uint8_t _shift = 0, _fn = 0, _sym = 0;
uint8_t KEY = 0, hadPressed = 0;
uint8_t Mode = 0; //0->normal.1->shift 2->long_shift, 3->sym, 4->long_shift 5->fn,6->long_fn
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

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

ISR(TIMER1_OVF_vect)        // interrupt service routine
{
  TCNT1 = timer1_counter;   // preload timer
  flash = !flash;
  redraw = 1;
}

void host_init(int buzzerPin) {
  buzPin = buzzerPin;
  oled.clear();
  if (buzPin)
    pinMode(buzPin, OUTPUT);
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
  if (!buzPin) return;
  digitalWrite(buzPin, HIGH);
  delay(1);
  digitalWrite(buzPin, LOW);
}

void host_cls() {
  memset(screenBuffer, 32, SCREEN_WIDTH * SCREEN_HEIGHT);
  memset(lineDirty, 1, SCREEN_HEIGHT);
  curX = 0;
  curY = 0;
}

void host_startupTone() {
  if (!buzPin) return;
  for (int i = 1; i <= 2; i++) {
    for (int j = 0; j < 50 * i; j++) {
      digitalWrite(buzPin, HIGH);
      delay(3 - i);
      digitalWrite(buzPin, LOW);
      delay(3 - i);
    }
    delay(100);
  }
}

void host_moveCursor(int x, int y) {
  if (x < 0) x = 0;
  if (x >= SCREEN_WIDTH) x = SCREEN_WIDTH - 1;
  if (y < 0) y = 0;
  if (y >= SCREEN_HEIGHT) y = SCREEN_HEIGHT - 1;
  curX = x;
  curY = y;
}

void host_showBuffer() {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    if (lineDirty[y] || (inputMode && y == curY)) {
      oled.setCursor(0, y);
      for (int x = 0; x < SCREEN_WIDTH; x++) {
        char c = screenBuffer[y * SCREEN_WIDTH + x];
        if (c < 32) c = ' ';
        if (x == curX && y == curY && inputMode && flash) c = 127;
        oled.print(c);
      }
      lineDirty[y] = 0;
    }
  }
}

void scrollBuffer() {
  memcpy(screenBuffer, screenBuffer + SCREEN_WIDTH, SCREEN_WIDTH * (SCREEN_HEIGHT - 1));
  memset(screenBuffer + SCREEN_WIDTH * (SCREEN_HEIGHT - 1), 32, SCREEN_WIDTH);
  memset(lineDirty, 1, SCREEN_HEIGHT);
  curY--;
}

void host_outputString(char *str) {
  int pos = curY * SCREEN_WIDTH + curX;
  while (*str) {
    lineDirty[pos / SCREEN_WIDTH] = 1;
    screenBuffer[pos++] = *str++;
    if (pos >= SCREEN_WIDTH * SCREEN_HEIGHT) {
      scrollBuffer();
      pos -= SCREEN_WIDTH;
    }
  }
  curX = pos % SCREEN_WIDTH;
  curY = pos / SCREEN_WIDTH;
}

void host_outputProgMemString(const char *p) {
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    host_outputChar(c);
  }
}

void host_outputChar(char c) {
  int pos = curY * SCREEN_WIDTH + curX;
  lineDirty[pos / SCREEN_WIDTH] = 1;
  screenBuffer[pos++] = c;
  if (pos >= SCREEN_WIDTH * SCREEN_HEIGHT) {
    scrollBuffer();
    pos -= SCREEN_WIDTH;
  }
  curX = pos % SCREEN_WIDTH;
  curY = pos / SCREEN_WIDTH;
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

  for (int i = 0; i < c; i++) {
    xx /= 10;
    char digit = ((num / xx) % 10) + '0';
    host_outputChar(digit);
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
  curX = 0;
  curY++;
  if (curY == SCREEN_HEIGHT)
    scrollBuffer();
  memset(screenBuffer + SCREEN_WIDTH * (curY), 32, SCREEN_WIDTH);
  lineDirty[curY] = 1;
}

char *host_readLine() {
  inputMode = 1;

  if (curX == 0) memset(screenBuffer + SCREEN_WIDTH * (curY), 32, SCREEN_WIDTH);
  else host_newLine();

  int startPos = curY * SCREEN_WIDTH + curX;
  int pos = startPos;

  bool done = false;
  char c;
  while (!done) {
    while (c = getChar()) {
      host_click();
      // read the next key
      lineDirty[pos / SCREEN_WIDTH] = 1;
      if (c >= 32 && c <= 126)
        screenBuffer[pos++] = c;
      else if (c == 0x08 && pos > startPos) //DELETE
        screenBuffer[--pos] = 0;
      else if (c == 0x0D) // ENTER
        done = true;
      curX = pos % SCREEN_WIDTH;
      curY = pos / SCREEN_WIDTH;
      // scroll if we need to
      if (curY == SCREEN_HEIGHT) {
        if (startPos >= SCREEN_WIDTH) {
          startPos -= SCREEN_WIDTH;
          pos -= SCREEN_WIDTH;
          scrollBuffer();
        }
        else
        {
          screenBuffer[--pos] = 0;
          curX = pos % SCREEN_WIDTH;
          curY = pos / SCREEN_WIDTH;
        }
      }
      redraw = 1;
    }
    if (redraw)
      host_showBuffer();
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
  if (c >= 32 && c <= 126)
    return c;
  else return 0;
}

bool host_ESCPressed() {

  while (inkeyChar = getChar()) {
    // read the next key
    //        inkeyChar = keyboard.read();
    if (inkeyChar == 0x1B)//ESC
      return true;
  }
  return false;
}

void host_outputFreeMem(unsigned int val)
{
  host_newLine();
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

void flashOn(void) {
  pixels.setPixelColor(0, pixels.Color(3, 3, 3)); pixels.show();
}
void flashOff(void) {
  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); pixels.show();
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
  for (uint8_t j = 0; j < 3; j++) {
    for (uint8_t i = 0; i < 5; i++) {
      pixels.setPixelColor(0, pixels.Color(i, i, i)); pixels.show();
      delay(10);
    }
    for (uint8_t i = 5; i > 0; i--) {
      pixels.setPixelColor(0, pixels.Color(i, i, i)); pixels.show();
      delay(10);
    }
  }
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
}

uint8_t GetInput(void) {
  uint8_t ret = 0;
  uint8_t i, j;
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
          flashOn();
        }
        flashOff();
        return ret;
      }
    }
    for (j = 0; j < 4; j++) {
      ret++;
      if (PINB == pgm_read_byte(&pinBmap[j])) {
        while (PINB != 223) {
          flashOn();
        }
        flashOff();
        return ret;
      }
    }
  }
  hadPressed = 0;
  return 0;
}

uint8_t getChar(void)
{
  uint8_t c = 0;

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
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      break;
    case 1://shift
      if ((idle / 6) % 2 == 1) {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      } else {
        pixels.setPixelColor(0, pixels.Color(5, 0, 0));
      }
      break;
    case 2://long_shift
      pixels.setPixelColor(0, pixels.Color(5, 0, 0));
      break;
    case 3://sym
      if ((idle / 6) % 2 == 1) {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      } else {
        pixels.setPixelColor(0, pixels.Color(0, 5, 0));
      }
      break;
    case 4://long_sym
      pixels.setPixelColor(0, pixels.Color(0, 5, 0)); break;
    case 5://fn
      if ((idle / 6) % 2 == 1) {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      } else {
        pixels.setPixelColor(0, pixels.Color(0, 0, 5));
      }
      break;
    case 6://long_fn
      pixels.setPixelColor(0, pixels.Color(0, 0, 5)); break;
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
  if (hadPressed == 0) {
    KEY = GetInput();
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

#if EXTERNAL_EEPROM
#include <I2cMaster.h>
extern TwiMaster rtc;

void writeExtEEPROM(unsigned int address, byte data)
{
  if (address % 32 == 0) host_click();
  rtc.start((EXTERNAL_EEPROM_ADDR << 1) | I2C_WRITE);
  rtc.write((int)(address >> 8));   // MSB
  rtc.write((int)(address & 0xFF)); // LSB
  rtc.write(data);
  rtc.stop();
  delay(5);
}

byte readExtEEPROM(unsigned int address)
{
  rtc.start((EXTERNAL_EEPROM_ADDR << 1) | I2C_WRITE);
  rtc.write((int)(address >> 8));   // MSB
  rtc.write((int)(address & 0xFF)); // LSB
  rtc.restart((EXTERNAL_EEPROM_ADDR << 1) | I2C_READ);
  byte b = rtc.read(true);
  rtc.stop();
  return b;
}

// get the EEPROM address of a file, or the end if fileName is null
unsigned int getExtEEPROMAddr(char *fileName) {
  unsigned int addr = 0;
  while (1) {
    unsigned int len = readExtEEPROM(addr) | (readExtEEPROM(addr + 1) << 8);
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
  unsigned int addr = 0;
  while (1) {
    unsigned int len = readExtEEPROM(addr) | (readExtEEPROM(addr + 1) << 8);
    if (len == 0) break;
    int i = 0;
    while (1) {
      char ch = readExtEEPROM(addr + 2 + i);
      if (!ch) break;
      host_outputChar(readExtEEPROM(addr + 2 + i));
      i++;
    }
    addr += len;
    host_outputChar(' ');
  }
  host_outputFreeMem(EXTERNAL_EEPROM_SIZE - addr - 2);
}

bool host_removeExtEEPROM(char *fileName) {
  unsigned int addr = getExtEEPROMAddr(fileName);
  if (addr == EXTERNAL_EEPROM_SIZE) return false;
  unsigned int len = readExtEEPROM(addr) | (readExtEEPROM(addr + 1) << 8);
  unsigned int last = getExtEEPROMAddr(NULL);
  unsigned int count = 2 + last - (addr + len);
  while (count--) {
    byte b = readExtEEPROM(addr + len);
    writeExtEEPROM(addr, b);
    addr++;
  }
  return true;
}

bool host_loadExtEEPROM(char *fileName) {
  unsigned int addr = getExtEEPROMAddr(fileName);
  if (addr == EXTERNAL_EEPROM_SIZE) return false;
  // skip filename
  addr += 2;
  while (readExtEEPROM(addr++)) ;
  sysPROGEND = readExtEEPROM(addr) | (readExtEEPROM(addr + 1) << 8);
  for (int i = 0; i < sysPROGEND; i++)
    mem[i] = readExtEEPROM(addr + 2 + i);
}

bool host_saveExtEEPROM(char *fileName) {
  unsigned int addr = getExtEEPROMAddr(fileName);
  if (addr != EXTERNAL_EEPROM_SIZE)
    host_removeExtEEPROM(fileName);
  addr = getExtEEPROMAddr(NULL);
  unsigned int fileNameLen = strlen(fileName);
  unsigned int len = 2 + fileNameLen + 1 + 2 + sysPROGEND;
  if ((long)EXTERNAL_EEPROM_SIZE - addr - len - 2 < 0)
    return false;
  // write overall length
  writeExtEEPROM(addr++, len & 0xFF);
  writeExtEEPROM(addr++, (len >> 8) & 0xFF);
  // write filename
  for (int i = 0; i < strlen(fileName); i++)
    writeExtEEPROM(addr++, fileName[i]);
  writeExtEEPROM(addr++, 0);
  // write length & program
  writeExtEEPROM(addr++, sysPROGEND & 0xFF);
  writeExtEEPROM(addr++, (sysPROGEND >> 8) & 0xFF);
  for (int i = 0; i < sysPROGEND; i++)
    writeExtEEPROM(addr++, mem[i]);
  // 0 length marks end
  writeExtEEPROM(addr++, 0);
  writeExtEEPROM(addr++, 0);
  return true;
}

#endif
