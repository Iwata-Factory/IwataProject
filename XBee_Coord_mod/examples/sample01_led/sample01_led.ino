/*********************************************************************
サンプルアプリ１　リモート先のLEDをON/OFFする。
起動後５秒くらい待ってから、Arduino親機のDigital 14 (Analog 0)をHighや
Low にすると、リモート先XBee子機のDIO11(XBee_pin 7)に接続したLED1が
点灯/消灯します。
Adafruit LCD Sheildを使用すれば、４方向ボタンでLED1とLED2(DIO12 XBee_
pin 4)も制御できます。

解説：本ソース内の「使い方」の欄を参照ください。
下記のサイトにも情報が掲載されています。
http://www.geocities.jp/bokunimowakaru/diy/xbee/xbee-tutorial_2.html

本ソースリストおよびソフトウェアは、ライセンスフリーです。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

                               Copyright (c) 2010-2012 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/
/*
使い方：
	(1) ライブラリのダウンロード
	　　当方のサイトから「PC/H8/Arduino用 管理lib. ZB Coordinator API 」を
	　　ダウンロードします。ファイル名＝xbeeCoord_XXX.zip (XXXは数字)
	　　　　http://www.geocities.jp/bokunimowakaru/diy/xbee/xbee-download.html
	(2) ライブラリのインストール
	　　ZIPファイルの中の「xbee_arduino」フォルダ内の「XBee」ファルダを	
	　　 arduinoをインストールしたフォルダ(例 C:\arduino)の中の「libraries」
	　　 フォルダにインポートします。
	　　　　C:\arduino\libraries\XBee\xbee.h
	　　　　C:\arduino\libraries\XBee\xbee.cpp
	　　　　C:\arduino\libraries\XBee\examples\...
	　　Adafruit LCDをお持ちの方は、そちらのライブラリもインストールします。
	　　　　C:\arduino\libraries\Adafruit_RGBLCDShield
	　　DFROBOT LCD Keypad Shieldをお持ちの方は「xbee_arduino」フォルダ内の
	　　「LiquidCrystalDFR」ファルダをインストールします
	(3) Arduino IDE を起動します。(既に起動していた場合、一度、終了して再起動)
	(4) ソースにXBeeモジュールのIEEEアドレスを追加
	　　本ソースの「byte dev_gpio」の宣言部分に記載しているZigBee IEEEアドレス
	　　を、お手持ちの子機のアドレス(XBeeモジュールの裏面に記載)に変更します。
	(5) 液晶の種類の設定
	　　Adafruit LCD Sheildを使用しない場合は本ソースの「#define ADAFRUIT」を
	　　削除します。ライブラリ内の「xbee.cpp」内のdefineも外せば、ライブラリの
	　　動作も液晶に表示できるようになります。
	(6) Arduinoマイコンボードに書き込みます。ATmega 328で動作確認しています。
	(7) 動作確認
	　　ArduinoのDigital 14 (Analog 0)をHigh や Low にすると リモート先のLED1が
	　　点灯/消灯します。(LED1はXBeeの4番ピン、Port12です。)
	　　Adafruit LCD Sheildを使用した場合は上下左右のキー操作でリモート先の
	　　LEDの点灯状態が変わります。
*/
/*
液晶表示に関する注意点

	Adafruit LCD Sheildを使用した場合：
	そのまま使用してください。液晶にリモート先のスイッチ状態が
	表示されます。

	DF ROBOTの場合：
	コンパイル前に#define ADAFRUITを消してください。
	また、Adafruit互換のKeypadライブラリをインストールしてください。
	http://www.geocities.jp/bokunimowakaru/diy/arduino/lcds.html
	
	Adafruit LCD Sheildをお持ちでない場合：
	コンパイル前に#define ADAFRUITを消してください。
	通常のキャラクタ液晶へ出力するようになります。
	
*/
/*
ハードウェア(XBee子機)

	port:	port指定	IO名	ピン番号			USB評価ボード(XBIB-U-Dev)
			port=11		DIO11	XBee_pin  7			    LED2	※port11がLED2
			port=12		DIO12	XBee_pin  4			    LED1	※port12がLED1
*/

/*********************************************************************
インポート処理						※ライブラリ XBeeに必要な処理です
*********************************************************************/

#include <xbee.h>

/*********************************************************************
アプリケーション					※以下を編集してアプリを作成します
*********************************************************************/

#define ADAFRUIT	// 使用する液晶が Adafruit LCD Sheild の時に定義する
#define KEYPAD		// 使用する液晶が Adafruit もしくは DF ROBOTの時

#ifndef ADAFRUIT
	// 通常のキャラクタLCDの場合
	#include <LiquidCrystalDFR.h>
	LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#else
	// Adafruit I2C接続LCDの場合(Adafruit_RGBLCDShieldライブラリが必要)
	#include <Wire.h>
	#include <Adafruit_MCP23017.h>
	#include <Adafruit_RGBLCDShield.h>
	Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#endif

//　↓お手持ちのXBeeモジュール子機のIEEEアドレスに変更する
byte dev_gpio[]   = {0x00,0x13,0xA2,0x00,0x40,0x30,0xC1,0x6F};	// PIO デバイス

#define BUTTON_PIN	14		// Digital 14 ピン(Analog 0 ピン)にボタンを接続

void setup(){
	pinMode(BUTTON_PIN, INPUT);   		// Digital 14 ピン (Analog 0 ピン)を入力に
	lcd.begin(16, 2);
	lcd.clear(); lcd.print("Sample 1 LED");
	xbee_init( 0x00 );					// XBee用COMポートの初期化(引数はポート番号)
	lcd.clear(); lcd.print("ATNJ");
	xbee_atnj( 0xFF );					// デバイスを常に参加受け入れ(テスト用)
	lcd.clear(); lcd.print("gpio init");
	xbee_gpio_init(dev_gpio);			// デバイスdev_gpioにIO設定を行うための送信
	lcd.clear(); lcd.print("DONE");
}

void loop(){
	byte c = 0;
	
	while(1){
		#ifndef KEYPAD								// 通常のキャラクタLCDの場合
			c = digitalRead(BUTTON_PIN);
			switch( c ){
				case 0:
					xbee_gpo(dev_gpio,12,1);		// GPOポート12をHレベル(消灯)へ
					xbee_gpo(dev_gpio,11,1);		// GPOポート11をHレベル(消灯)へ
					break;
				case 1:
					xbee_gpo(dev_gpio,12,0);		// GPOポート12をLレベル(点灯)へ
					xbee_gpo(dev_gpio,11,0);		// GPOポート11をLレベル(点灯)へ
					break;
				default:
					break;
			}
			lcd.clear(); lcd.print( c , BIN );
			while( c == digitalRead(BUTTON_PIN) );
		#else										// Adafruit I2C接続LCDの場合
			c = lcd.readButtons();
			switch( c ){
				case BUTTON_DOWN:
					xbee_gpo(dev_gpio,12,1);		// GPOポート12をHレベル(消灯)へ
					xbee_gpo(dev_gpio,11,1);		// GPOポート11をHレベル(消灯)へ
					break;
				case BUTTON_LEFT:
					xbee_gpo(dev_gpio,12,0);		// GPOポート12をLレベル(点灯)へ
					break;
				case BUTTON_RIGHT:
					xbee_gpo(dev_gpio,11,0);		// GPOポート11をLレベル(点灯)へ
					break;
				case BUTTON_UP:
					xbee_gpo(dev_gpio,12,0);		// GPOポート12をHレベル(消灯)へ
					xbee_gpo(dev_gpio,11,0);		// GPOポート11をHレベル(消灯)へ
					break;
				default:
					break;
			}
			lcd.clear();
			lcd.print( c , BIN );
			while( c == lcd.readButtons() );
		#endif
	}
}
