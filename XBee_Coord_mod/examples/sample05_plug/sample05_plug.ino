/*********************************************************************
サンプルアプリ５　スマートプラグ
XBee Smart Plug(Digi 純正品)を発見すると、そのデバイスから照度と消費
電力を得ます。暗くなるとSmart Plugに接続した機器の電源が切れます。

新しいXBee子機のコミッションボタンDIO0(XBee_pin 20)を押下する(信号レベ
ルがH→L→Hに推移する)と、子機から参加要求信号が発行されて、Arduino親機
が子機を発見し、「found a device」と表示します。

ただし、複数のZigBeeネットワークに対応するために、ペアリングは起動後の
30秒間しか受け付けません。Arduino親機を起動して、コミッションボタンを
押してペアリングを実施しておき、再度、コミッションボタンでデバイス発見
します。デバイス発見は30秒のペアリング期間後でも行えます。

本ソースリストおよびソフトウェアは、ライセンスフリーです。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

※注意：パソコンやファンヒータなどAC電源の途絶えると故障や事故の発生する
　機器をSmart Plugに接続しないこと。
　
                               Copyright (c) 2010-2012 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/
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
Arduinoへの書き込み時の注意点

書き込み時にXBeeからの受信があると、書き込みに失敗する場合があり案す。
全ての子機をオフにする、もしくはArduinoに取り付けた親機を外してから
書き込んでください。
*/
/*
ハードウェア(XBee子機) Digi純正 Smart Plug

	port:	port指定	IO名	ピン番号			センサーデバイス
			port= 0		DIO0	XBee_pin 20 (Cms)	 SW1 Commision
			port= 1		DIO1	XBee_pin 19 (AD1)	 照度センサー
			port= 2		DIO2	XBee_pin 18 (AD2)	 温度センサー
			port= 3		DIO3	XBee_pin 17 (AD3)	 AC電流センサー
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

byte dev_sens[]   = {0x00,0x13,0xA2,0x00,0x40,0x00,0x00,0x00};	// 仮設定
byte dev_en = 0;	// センサー発見の有無(0:未発見)

void setup(){
	lcd.begin(16, 2);
	lcd.clear(); lcd.print("Sample 5 PLUG");
	xbee_init( 0x00 );					// XBee用COMポートの初期化(引数はポート番号)
}

void loop(){
	byte i;
	byte trig=0;
	float value;

	XBEE_RESULT xbee_result;			// 受信データ用の構造体

	// デバイス探索
	lcd.clear(); lcd.print("Searching:SmPlug");
	if( xbee_atnj(30) ){				// 30秒間の参加受け入れ
		lcd.clear(); lcd.print("Found a device");
		xbee_from( dev_sens );			// 受信したアドレスをdev_sensへ格納
		dev_en = 1;						// sensor発見済
		xbee_gpio_config( dev_sens, 1 , AIN ); // 接続相手のポート1をAIN入力へ
		xbee_gpio_config( dev_sens, 2 , AIN ); // 接続相手のポート2をAIN入力へ
		xbee_gpio_config( dev_sens, 3 , AIN ); // 接続相手のポート3をAIN入力へ
	}else{
		lcd.clear(); lcd.print("Failed:no dev.");
	}
	delay(500);
	lcd.clear(); lcd.print("DONE");

	// メイン処理
	lcd.clear(); lcd.print("Receiving");
	while(1){							// 永久に受信する
		if( dev_en && trig == 0){
			xbee_force( dev_sens );		// デバイスdev_sensへデータ要求(100回に1回の頻度で)
			trig = 100;
		}
		trig--;
		/* データ受信(待ち受けて受信する) */
		xbee_rx_call( &xbee_result );	// データを受信します。
		switch( xbee_result.MODE ){		// 受信したデータの内容(MODE値)に応じて
			case MODE_RESP:				// sensorからの応答の場合に照度と温度を表示
			case MODE_GPIN:				// 周期方式でデータ受信した場合も想定
				if( bytecmp( dev_sens , xbee_result.FROM ,8 ) == 0 &&
				xbee_result.STATUS == STATUS_OK ){
					// 照度測定結果
					value = xbee_sensor_result( &xbee_result, LIGHT);	
					lcd.clear(); lcd.print( (unsigned int)value ); lcd.print("Lux ");
					// 暗いときは電源を切り、明るいときは電源を入れる
					if( value < 1.0 ){
						xbee_gpo(dev_sens , 4 , 0 );	// Turns outlet off (Port 4 = 0)
					}else{
						xbee_gpo(dev_sens , 4 , 1 );	// Turns outlet on (Port 4 = 1)
					}
					// 温度測定結果 (当方で確認したところ7.12℃の高めが表示されますが、Digi社の仕様です。)
					value = xbee_sensor_result( &xbee_result, TEMP);
					lcd.clear(); lcd.print( value , 1 ); lcd.print("C");
					// 電力測定結果
					value = xbee_sensor_result( &xbee_result, WATT);
					lcd.setCursor(0,1); lcd.print( value , 1 ); lcd.print("Watts");
				}
				break;
			case MODE_IDNT:				// 新しいデバイスを発見
				lcd.clear(); lcd.print("found a new dev");
				for( i=0;i<8;i++ ) dev_sens[i]=xbee_result.FROM[i];
										// 発見したアドレスをdev_sensに読み込み
				dev_en = 1;				// sensor発見済
				xbee_gpio_config( dev_sens, 1 , AIN ); // 接続相手のポート1をAIN入力へ
				xbee_gpio_config( dev_sens, 2 , AIN ); // 接続相手のポート2をAIN入力へ
				xbee_gpio_config( dev_sens, 3 , AIN ); // 接続相手のポート3をAIN入力へ
				break;
			default:
				break;
		}
	}
}
