# ArduinoBasic for cardKeyBoard Beta Version
- Robin Edwardsさんの[”ArduinoBasic”](https://github.com/robinhedwards/ArduinoBASIC)を、M5stak用のCardKeyBoardとI2CのOLED(128x32)で動かしています。<br>

![image](./img/img001.jpg)

## ToDo
- 三角関数

## コンパイルの際の注意事項
- Adafruit NeoPixelライブラリが必要です。
- SSD1306ASCIIは「\Arduino\libraries」フォルダー配下に入れてください。
- コンパイルの際は下図のようにATmega328p(3.3V,8Mhz)を選択してください。<br>![image](./img/img002.PNG)<br>
- バイナリは下図から作成できます<br>![image](./img/img003.PNG)<br>
- avrdudeやavrdude-GUI等のツールを使ってコンパイルしたバイナリを書き込んでください。

