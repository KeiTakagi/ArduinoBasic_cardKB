# CardKeyBoardでArduinoBasicが動作する自作ポケコン
[M5stak](https://m5stack.com/)用のCardKeyBoardとI2CのOLED(128x32)で、Robin Edwardsさんの[ArduinoBasic](https://github.com/robinhedwards/ArduinoBASIC)が動く自作ポケコンです。80年代の家庭用コンピューターで使われていたBasic言語を実行することができます。I2CのOLED(128x64)をそのまま差し替えると縦に２倍拡大して表示されます。<br>
[![pocket computer](./img/img001.jpg)](https://youtu.be/O71nmI2AG5k)

## ToDo
- コード最適化
- グラフィック関数(点,画面消去)

## GPIO関連命令を廃止しました
cardKBのキーボードで使用するGPIOと競合するため、PIN,PINMODE,PINREAD,ANALOGRDの使用を廃止しました。理解した上で使用する場合は、basic.hの下記の箇所を修正してください。<br>
```
//GPIO 1...USE GPIO   0...GPIO NONE
#define GPIO                    0

  ↓↓

#define GPIO                    1
```

## 三角関数に対応しました
RADですがSIN、COS、TAN、それと、EXPとSQRTも追加しました。

## EEPROMに対応しました
EEPROM(24LC256など)を使用する場合、host.hを以下のように0から1へ書き換えてください。<br>
```
#define EXTERNAL_EEPROM         0

  ↓↓

#define EXTERNAL_EEPROM         1
```
初回、EEPROMを使用する場合コメントを外して実行するとEEPROMが初期化されます。初期化後は元に戻してください。コメントを戻さない場合、毎回初期化されます。
```
  // IF USING EXTERNAL EEPROM
  // The following line 'wipes' the external EEPROM and prepares
  // it for use. Uncomment it, upload the sketch, then comment it back
  // in again and upload again, if you use a new EEPROM.
  // writeExtEEPROM(0,0); writeExtEEPROM(1,0);

  ↓↓

     writeExtEEPROM(0,0); writeExtEEPROM(1,0);
```

## コンパイルの際の注意事項
- Adafruit NeoPixelライブラリが必要です。
- SSD1306ASCIIは「\Arduino\libraries」フォルダー配下に入れてください。
- コンパイルの際は下図のようにATmega328p(3.3V,8Mhz)を選択してください。選択しない場合、NeoPixel LEDが白色で点灯したままになります。<br>![image](./img/img002.PNG)<br>
- バイナリは下図から作成できます<br>![image](./img/img003.PNG)<br>
- avrdudeやavrdude-GUI等のツールを使ってコンパイルしたバイナリを書き込んでください。

## ヒューズビット
初期値のまま書き換えないでください。<br>
![image](./img/img004.PNG)<br>

## 1.3インチ I2C OLED 128×64に対応しました
SSD1306ASCII_I2C.hを以下のように修正してください。<br>
注意：0.96インチ I2C OLED 128×64は128×32として動作を想定しています。想定した動作をしないかもしれません。<br>
```
#define OLED_WIDTH  128
//#define OLED_HEIGHT 32
#define OLED_HEIGHT 64

#define OLED_COLMAX 21
//#define OLED_ROWMAX 4
#define OLED_ROWMAX 8
```
![image](./img/img011.jpg)