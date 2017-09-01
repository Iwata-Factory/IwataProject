/*********************************************************************
サンプルアプリ０

下記のサイトに情報が掲載されています。

http://www.geocities.jp/bokunimowakaru/diy/xbee/xbee-arduino_3.html

本ソースリストおよびソフトウェアは、ライセンスフリーです。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

                               Copyright (c) 2010-2013 Wataru KUNINO
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
	(3) Arduino IDE を起動します。(既に起動していた場合、一度、終了して再起動)
	(4) ソースにXBeeモジュールのIEEEアドレスを追加
	　　本ソースの「byte dev_gpio」の宣言部分に記載しているZigBee IEEEアドレス
	　　を、お手持ちの子機のアドレス(XBeeモジュールの裏面に記載)に変更します。
	(5) 液晶の種類の設定
	　　Adafruit LCD Sheildを使用しない場合は本ソースの「#define ADAFRUIT」を
	　　削除します。ライブラリ内の「xbee.cpp」内のdefineも外せば、ライブラリの
	　　動作も液晶に表示できるようになります。
	(6) Arduinoマイコンボードに書き込みます。ATmega 328で動作確認しています。
*/
/*
液晶表示に関する注意点

	Adafruit LCD Sheildを使用した場合：
	そのまま使用してください。液晶にリモート先のスイッチ状態が
	表示されます。

	Adafruit LCD Sheildをお持ちでない場合：
	コンパイル前に#define ADAFRUITを消してください。
	通常のキャラクタ液晶へ出力するようになります。
*/
/*
ハードウェア(XBee子機)

	port:	port指定	IO名	ピン番号	
			port= 0		DIO0	XBee_pin 20		SW1 Commision
			port= 1		DIO1	XBee_pin 19		デジタル入力
			port= 2		DIO2	XBee_pin 18		アナログ入力
			port= 4		DIO4	XBee_pin 16		LED3
			port=11		DIO11	XBee_pin  7		LED2
			port=12		DIO12	XBee_pin  4		LED1
*/

/*********************************************************************
インポート処理						※ライブラリ XBeeに必要な処理です
*********************************************************************/

#include <xbee.h>

/*********************************************************************
アプリケーション					※以下を編集してアプリを作成します
*********************************************************************/

#define ADAFRUIT	// 使用する液晶が Adafruit LCD Sheild の時に定義する

#ifndef ADAFRUIT
	// 通常のキャラクタLCDの場合
	#include <LiquidCrystal.h>
	LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#else
	// Adafruit I2C接続LCDの場合(Adafruit_RGBLCDShieldライブラリが必要)
	#include <Wire.h>
	#include <Adafruit_MCP23017.h>
	#include <Adafruit_RGBLCDShield.h>
	Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#endif

//　↓お手持ちのXBeeモジュール子機のIEEEアドレスに変更する
byte dev_gpio[]   = {0x00,0x13,0xA2,0x00,0x40,0x30,0xC1,0x6F};

void setup(){
	lcd.begin(16, 2);					// キャラクタ液晶の初期化
	lcd.clear();						// 液晶表示をクリア（消去）
	lcd.print("Sample 0 TEST");			// タイトル表示
	xbee_init( 0x00 );					// XBee用COMポートの初期化(引数はポート番号)
	xbee_gpio_init(dev_gpio);			// デバイスdev_gpioにIO設定を行うための送信
	delay(1000);						// 1秒間の待ち
}

void loop(){
	int gpi = xbee_gpi(dev_gpio,1);		// GPIポート1からデジタル入力
	int adc = xbee_adc(dev_gpio,2);		// ADCポート2からアナログ入力
	xbee_gpo(dev_gpio,11,gpi);			// GPOutポート11をin値へ
	xbee_gpo(dev_gpio,12,1);			// GPOutポート12をHレベルへ
	
	lcd.clear();						// 液晶表示をクリア（消去）
	lcd.print("GPI(1)  = ");
	lcd.print( gpi );					// inの値を表示
	lcd.setCursor(0,1);					// 液晶の２行目にカーソルを移動
	lcd.print("ADC(2)  = ");
	lcd.print( adc  );					// inの値を表示
	delay(1000);						// 1秒間の待ち
	
	xbee_gpo(dev_gpio,12,0);			// GPOutポート12をLレベルへ
	delay(1000);						// 1秒間の待ち
}
