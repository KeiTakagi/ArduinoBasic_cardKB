# CardKeyboardでArduinoBasicが動作する自作ポケコン
[M5stak](https://m5stack.com/)用のCardKeyBoardとI2CのOLED(128x32)で、Robin Edwardsさんの[ArduinoBasic](https://github.com/robinhedwards/ArduinoBASIC)が動く自作ポケコンです。８０年代の家庭用コンピューターで使われていたBasic言語を実行することができます。I2CのOLED(128x64)をそのまま差し替えると縦に２倍拡大して表示されます。<br>

![image](./img/img001.jpg)


## ToDo
- コード最適化
- グラフィック関数(点,画面消去)
- 三角関数

## EEPROMに対応しました
EEPROM(24LC256など)を使用する場合、host.hの該当箇所を以下のように書き換えてください。
```
#define EXTERNAL_EEPROM         1
```

## コンパイルの際の注意事項
- Adafruit NeoPixelライブラリが必要です。
- SSD1306ASCIIは「\Arduino\libraries」フォルダー配下に入れてください。
- コンパイルの際は下図のようにATmega328p(3.3V,8Mhz)を選択してください。選択しない場合、NeoPixel LEDが白色で点灯したままになります。<br>![image](./img/img002.PNG)<br>
- バイナリは下図から作成できます<br>![image](./img/img003.PNG)<br>
- avrdudeやavrdude-GUI等のツールを使ってコンパイルしたバイナリを書き込んでください。

## ヒューズビット
ヒューズビットを書き換える場合、誤った値を書き込むと文鎮化する可能性があります。自己責任で書き換えてください。<br>
- HIGH = 0xC2
- LOW = 0x07
- EXTENDED = 0xD8
- LOCKBIT = 0x3F
