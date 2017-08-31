/*********************************************************************
XBeeからシリアルで受信した内容をファイルへ保存する。(UART.csv)

子機XBeeのUARTからのデータをArduino(親機)で受信してSDカードに保存します。

	液晶はAdafruit LCD Sheildのライブラリを使用しています。
	お持ちでない場合は、#define ADAFRUITを無効にしてください。
	
	SDカードは Ardino 純正 Wireless SD Shield (XBee Shield)用です。

※メモリの制約から#define LITEを有効にしなければ動作しない場合があります。

本ソースリストおよびソフトウェアは、ライセンスフリーです。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

                               Copyright (c) 2010-2012 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/
/*
Arduinoへの書き込み時の注意点

書き込み時にXBeeからの受信があると、書き込みに失敗する場合があり案す。
全ての子機をオフにする、もしくはArduinoに取り付けた親機を外してから
書き込んでください。
*/
/*
メモリ制約の注意点

	標準ライブラリの液晶へはメモリ不足で表示が出来ない場合があります。
	この場合は表示をライブラリで#define LITEを定義してください。
	但し、Arduino 1.0.1とXBeeライブラリ Ver 1.5の組み合わせでは修正しな
	くても動作しています。
*/
/*
SDカードのCS端子(チップセレクト)の設定について

	void setup()内のSD.begin()でCS端子を設定しています。Arduino Wireless
	SD SheildではDigital 4番ピンなので、SD.begin(4)と設定します。
	デフォルト(ADAFRUIT非定義時)はDigital 10番ピンになります。
	複数のSPI機器を使う場合は、それぞれ異なるCSピンを使用します。
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

#include <SD.h>

void setup(){
	File file;

	lcd.begin(16, 2);
	lcd.clear(); lcd.print("Sample 7 UART-SD");
	#ifdef ADAFRUIT
		while( SD.begin(4) == false ) delay(3000);
	#else
		while( SD.begin() == false ) delay(3000);
	#endif
	file = SD.open( "LOG.TXT" , FILE_WRITE);
	file.println( "started" );
	if( file != false ) {
		file.println( "hello!" );
		xbee_init( 0x00 );				// XBee用COMポートの初期化
		xbee_atnj( 0xFF );				// ジョイン許可
		file.println( "started xbee" );
	}
	file.close();
	lcd.clear(); lcd.print( "started xbee" );
}

void loop(){
	byte i;
	byte data;
	unsigned int value_i;
	
	File file;
	
	XBEE_RESULT xbee_result;			// 受信データ用の構造体
	
	// メイン処理
	while(1){							// 永久に受信する
		/* データ受信(待ち受けて受信する) */
		xbee_rx_call( &xbee_result );

		switch( xbee_result.MODE ){		// 受信したデータの内容(MODE値)に応じて
			case MODE_UART:
				/* シリアルデータを受信 */
				file = SD.open( "UART.TXT" , FILE_WRITE);
				if( file != false ) {
					for(i=0;i<data;i++){
						if( (xbee_result.DATA[i] >=' ' && xbee_result.DATA[i] <= 126)
						 || (xbee_result.DATA[i] == (byte)'\n') )
							file.print( (char)xbee_result.DATA[i] );
							// 文字の時に書き出し
					}
					file.close();
				}
				for(i=0;i<data;i++){
					if( xbee_result.DATA[i] >= (byte)' ' && xbee_result.DATA[i] <= 126)
						lcd.print( (char)xbee_result.DATA[i] );
					if( xbee_result.DATA[i] == (byte)'\r' || xbee_result.DATA[i] == (byte)'\n' )
						lcd.clear();
				}
				break;
			case MODE_GPIN:
				/* GPIOからのデータを受信 */
				file = SD.open( "GPIN.TXT" , FILE_WRITE);
				if( file != false ) {
					for(i=0;i<2;i++) file.print( xbee_result.DATA[i] , BIN );
					for(i=1;i<3;i++) file.print( (int)xbee_result.ADCIN[i] );
					file.close();
					file.print( '\n' );
				}
				lcd.clear();
				lcd.print( "GPIN" );
				break;
			case MODE_IDNT:				// 新しいデバイスを発見
				file = SD.open( "LOG.TXT" , FILE_WRITE);
				if( file != false ) {
					file.print( "IDNT from:" );
					for(i=0;i<8;i++) file.print( xbee_result.FROM[i] , HEX );
					file.print( '\n' );
					file.close();
				}
				lcd.clear();
				lcd.print( "IDNT" );
				break;
			default:
				break;
		}
	}
}
