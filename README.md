# CardKeyboardでArduinoBasicが動作する自作ポケコン
[M5stak](https://m5stack.com/)用のCardKeyBoardとI2CのOLED(128x32)で、Robin Edwardsさんの[ArduinoBasic](https://github.com/robinhedwards/ArduinoBASIC)が動く自作ポケコンです。８０年代の家庭用コンピューターで使われていたBasic言語を実行することができます。I2CのOLED(128x64)をそのまま差し替えると縦に２倍拡大して表示されます。<br>
[![pocket computer](./img/img001.jpg)](https://youtu.be/O71nmI2AG5k)

## ToDo
- コード最適化
- GPIO関連命令の廃止
- 三角関数
- グラフィック関数(点,画面消去)

## EEPROMに対応しました
EEPROM(24LC256など)を使用する場合、host.hを以下のように0から1へ書き換えてください。
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
ヒューズビットを書き換える場合、誤った値を書き込むと文鎮化する可能性があります。自己責任で書き換えてください。<br>
- HIGH = 0xC2
- LOW = 0x07
- EXTENDED = 0xD8
- LOCKBIT = 0x3F
