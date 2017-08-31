/*********************************************************************
サンプルアプリ３　スイッチ
子機のスイッチ2～4(Port1～3)を押した時に、その状態を液晶に表示します。
新しいデバイスを発見すると、そのデバイスのGPIOの設定を変更します。
（同時に２つ以上のデバイスのスイッチ状態を検出できます）
新しいXBee子機のコミッションボタンDIO0(XBee_pin 20)を押下する(信号レベ
ルがH→L→Hに推移する)と、子機から参加要求信号が発行されて、Arduino親機
が子機を発見し、「found a device」と表示します。
始めは、一度、ペアリングをしてから、再度コミッションボタンを押す必要が
あります。

解説：本ソース内の「使い方」の欄を参照ください。

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
	　　XBee子機のDIO1～3(XBee_pin 19～17)をHigh や Low にすると Arduino親機に
	　　スイッチの状態が表示されます。
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

	port:	port指定	IO名	ピン番号			USB評価ボード(XBIB-U-Dev)
			port= 0		DIO0	XBee_pin 20 (Cms)	    SW1 Commision
			port= 1		DIO1	XBee_pin 19 (AD1)	    SW2
			port= 2		DIO2	XBee_pin 18 (AD2)	    SW3
			port= 3		DIO3	XBee_pin 17 (AD3)	    SW4
*/

/*********************************************************************
インポート処理						※ライブラリ XBeeに必要な宣言です
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
byte dev_gpio[]   = {0x00,0x13,0xA2,0x00,0x40,0x30,0xC1,0x6F};	// PIO デバイス

void setup(){
	lcd.begin(16, 2);
	lcd.clear(); lcd.print("Sample 3 ADD DEV");
	xbee_init( 0x00 );					// XBee用COMポートの初期化(引数はポート番号)
	lcd.clear(); lcd.print("gpio init");
	xbee_gpio_init(dev_gpio);			// デバイスdev_gpioにIO設定を行うための送信
	lcd.clear(); lcd.print("ATNJ");
	xbee_atnj( 0xFF );					// デバイスを常に参加受け入れ(テスト用)
	lcd.clear(); lcd.print("DONE");
}

void loop(){
	char i;
	byte data;

	XBEE_RESULT xbee_result;			// 受信データ用の構造体

	// メイン処理
	lcd.clear(); lcd.print("Receiving");
	while(1){							// 永久に受信する
		/* データ受信(待ち受けて受信する) */
		data = xbee_rx_call( &xbee_result );
										// データを受信します。
		switch( xbee_result.MODE ){		// 受信したデータの内容(MODE値)に応じて
			case MODE_GPIN:				// GPIO入力を検出(リモート機のボタンが押された)
				/* スイッチの状態を表示 */
				lcd.clear(); lcd.print("D ");	// GPIO入力値(デジタル)を表示
				for( i=7; i>=0 ; i--) lcd.print( (char)( '0' + ((data>>i) & 0x01) ) );
				/* ADC入力値の表示 */
				lcd.setCursor(0,1); lcd.print("A ");	// ADC入力値(アナログ)を表示
				for( i=1;i<3;i++ ){ 
					lcd.print( xbee_result.ADCIN[i] );
					lcd.print(' ');
				}
				break;
			case MODE_IDNT:				// 新しいデバイスを発見
				for( i=0;i<8;i++ ) dev_gpio[i]=xbee_result.FROM[i];
										// 発見したアドレスをdev_gpioに読み込み
				xbee_gpio_init( dev_gpio );
				lcd.clear(); lcd.print("found a new dev");
				break;
			default:
				break;
		}
	}
}
