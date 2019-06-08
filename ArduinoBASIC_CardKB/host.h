/*
    @file host.h
    @brief Modified Arduino Basic to work with cardKeyboard and OLED 128x32.
    Reference source:https://github.com/robinhedwards/ArduinoBASIC

    @author Kei Takagi
    @date 2019.6.8

    Copyright (c) 2019 Kei Takagi
*/

#include <stdint.h>
#include <Adafruit_NeoPixel.h>

#define SCREEN_WIDTH            21
#define SCREEN_HEIGHT           4

#define EXTERNAL_EEPROM         0
#define EXTERNAL_EEPROM_ADDR    0x50    // I2C address (7 bits)
#define EXTERNAL_EEPROM_SIZE    32768   // only <=32k tested (64k might work?)

#define MAGIC_AUTORUN_NUMBER    0xFC

void host_init(int buzzerPin);
void host_sleep(long ms);
void host_digitalWrite(int pin, int state);
int host_digitalRead(int pin);
int host_analogRead(int pin);
void host_pinMode(int pin, int mode);
void host_click();
void host_startupTone();
void host_cls();
void host_showBuffer();
void host_moveCursor(int x, int y);
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

#define NUMPIXELS      1
#define PIN           13

#define Set_Bit(val, bitn)    (val |=(1<<(bitn)))
#define Clr_Bit(val, bitn)    (val&=~(1<<(bitn)))
#define Get_Bit(val, bitn)    (val &(1<<(bitn)) )

//       d0   d1     d2  d3 d4 d5 d6 d7 d8 d9 d10 d11
//A3�F   esc   1      2  3  4  5  6  7  8  9  0   del
//A2:    tab   q      w  e  r  t  y  u  i  o  p
//A1:   left   up     a  s  d  f  g  h  j  k  l   enter
//A0:   down   right  z  x  c  v  b  n  m  ,  .   space
//sym: d15
//shift: d12
//fn: d14

const uint8_t PROGMEM KeyMap[48][7] =
{ //nor, shift,long_shift, sym,long_sym,fn,long_fn,
  {  27,  27,  27,  27,  27, 128, 128},//esc
  { '1', '1', '1', '!', '!', 129, 129},//1
  { '2', '2', '2', '@', '@', 130, 130},//2
  { '3', '3', '3', '#', '#', 131, 131},//3
  { '4', '4', '4', '$', '$', 132, 132},//4
  { '5', '5', '5', '%', '%', 133, 133},//5
  { '6', '6', '6', '^', '^', 134, 134},//6
  { '7', '7', '7', '&', '&', 135, 135},//7
  { '8', '8', '8', '*', '*', 136, 136},//8
  { '9', '9', '9', '(', '(', 137, 137},//9
  { '0', '0', '0', ')', ')', 138, 138},//0
  {  8 , 127 , 127, 8 ,  8 , 139, 139},//del
  {  9 ,  9 ,  9 ,  9 ,  9 , 140, 140},//tab
  { 'q', 'Q', 'Q', '{', '{', 141, 141},//q
  { 'w', 'W', 'W', '}', '}', 142, 142},//w
  { 'e', 'E', 'E', '[', '[', 143, 143},//e
  { 'r', 'R', 'R', ']', ']', 144, 144},//r
  { 't', 'T', 'T', '/', '/', 145, 145},//t
  { 'y', 'Y', 'Y', '\\', '\\', 146, 146}, //y
  { 'u', 'U', 'U', '|', '|', 147, 147},//u
  { 'i', 'I', 'I', '~', '~', 148, 148},//i
  { 'o', 'O', 'O', '\'', '\'', 149, 149}, //o
  { 'p', 'P', 'P', '"', '"', 150, 150},//p
  {  0 ,  0 ,  0 ,  0 ,  0 , 0, 0},    //  no key
  {  180,  180,  180,  180,  180, 152, 152},//LEFT
  {  181,  181,  181,  181,  181, 153, 153},//UP
  { 'a', 'A', 'A', ';', ';', 154, 154},//a
  { 's', 'S', 'S', ':', ':', 155, 155},//s
  { 'd', 'D', 'D', '`', '`', 156, 156},//d
  { 'f', 'F', 'F', '+', '+', 157, 157},//f
  { 'g', 'G', 'G', '-', '-', 158, 158},//g
  { 'h', 'H', 'H', '_', '_', 159, 159},//h
  { 'j', 'J', 'J', '=', '=', 160, 160},//j
  { 'k', 'K', 'K', '?', '?', 161, 161},//k
  { 'l', 'L', 'L',   0,   0, 162, 162},//l
  { 13 , 13 ,  13,  13,  13, 163, 163},//enter
  {  182, 182 ,  182,  182,  182, 164, 164},//DOWN
  {  183, 183 ,  183,  183,  183, 165, 165},//RIGHT
  { 'z', 'Z', 'Z',  0 ,  0 , 166, 166},//z
  { 'x', 'X', 'X',  0 ,  0 , 167, 167},//x
  { 'c', 'C', 'C',  0 ,  0 , 168, 168},//c
  { 'v', 'V', 'V',  0 ,  0 , 169, 169},//v
  { 'b', 'B', 'B',  0 ,  0 , 170, 170},//b
  { 'n', 'N', 'N',  0 ,  0 , 171, 171},//n
  { 'm', 'M', 'M',  0 ,  0 , 172, 172},//m
  { ',', ',', ',', '<', '<', 173, 173},//,
  { '.', '.', '.', '<', '>', 174, 174},//.
  { ' ', ' ', ' ', ' ', ' ', 175, 175} //space
};
const uint8_t PROGMEM pinDmap[] = {254, 253, 251, 247, 239, 223, 191, 127};
const uint8_t PROGMEM pinBmap[] = {222, 221, 219, 215};

#define shiftPressed (PINB & 0x10 ) != 0x10
#define symPressed (PINB & 0x80 ) != 0x80
#define fnPressed (PINB & 0x40 ) != 0x40

void flashOn(void);
void flashOff(void);
void keybordSetup(void);
uint8_t GetInput(void);
uint8_t getChar(void);

#if EXTERNAL_EEPROM
#include <I2cMaster.h>
void writeExtEEPROM(unsigned int address, byte data);
void host_directoryExtEEPROM();
bool host_saveExtEEPROM(char *fileName);
bool host_loadExtEEPROM(char *fileName);
bool host_removeExtEEPROM(char *fileName);
#endif
