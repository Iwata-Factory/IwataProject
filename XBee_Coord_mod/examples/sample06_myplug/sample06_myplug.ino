/*********************************************************************
サンプルアプリ６　Myスマートプラグ
自作 XBee Smart Plugを発見すると、デバイスから照度と温度、消費電力を
得ます。

ご注意：測定値は目安です。環境によって変動します。

使用方法：
http://www.geocities.jp/bokunimowakaru/diy/xbee/xbee-plug.html

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
ハードウェア(XBee子機)

	port:	port指定	IO名	ピン番号			センサーデバイス
			port= 0		DIO0	XBee_pin 20 (Cms)	 SW1 Commision
			port= 1		DIO1	XBee_pin 19 (AD1)	 照度センサー 
			port= 2		DIO2	XBee_pin 18 (AD2)	 温度センサー 
			port= 3		DIO3	XBee_pin 17 (AD3)	 電流センサー 
			
センサーデバイスの型番と情報

	照度センサー
			照度センサ（フォトトランジスタ）新日本無線ＮＪＬ７５０２Ｌ
			http://akizukidenshi.com/catalog/g/gI-02325/
			
			光電流３３μＡ　（白色LED, 100Lux） 但しA 光源時４６μＡ

				AIN[mV] = LUX * 0.33e-3[mA] * RL[Ω]
			∴ LUX[lux] = AIN[mV]/3.3		(RL = 10kΩ)の時

	温度センサー
			高精度ＩＣ温度センサ National ＬＭ６１ＣＩＺ
			http://akizukidenshi.com/catalog/g/gI-02726/
			
				AIN[mV] ＝（＋１０ｍＶ／℃×Ｔ℃）＋６００ｍＶ
			∴ T[deg C] = AIN[mV]/10-60

	電流センサー
			30Aクランプ型電流センサ SEN-11005 (sparkfun)
			ECHUN Electronic ECS1030-L72
			http://www.sparkfun.com/products/11005
			http://strawberry-linux.com/catalog/items?code=18214
			
			Turnn ratio Np:Ns=1:2000
			Current Ratio 30A/15mA
			AIN入力の分割抵抗比DIV = 47k/(47k+47k)
			
				AIN[mV] ＝ AC[mA]/2000*RL*DIV*sqrt(2)[Ohm]
			∴	AC[A] = AIN[mV]/(RL*DIV/2*sqrt(2))
				AC[W] = AIN[mV]/(RL*DIV/200*sqrt(2))
				
			(RL = 1kΩ DIV=0.5)の時
				AC[A] = AIN[mV]/353.6
				AC[W] = AIN[mV]/3.536		但し 最大339Wまで
											OPAのVohで実際には300Wくらいまで
			(RL = 330Ω DIV=0.5)の時
				AC[A] = AIN[mV]/116.7
				AC[W] = AIN[mV]/1.167		但し 最大1028Wまで
											OPAのVohで実際には900Wくらいまで
			(RL = 100Ω DIV=0.5)の時
				AC[A] = AIN[mV]/35.36
				AC[W] = AIN[mV]/0.3536		但し 最大3390Wまで
											OPAのVohで実際には3kWくらいまで
											
			*/
			#define CTSENSER_MLTPL 1.17302 / 3.536		// 負荷抵抗1kΩ 最大300W
			/*
			#define CTSENSER_MLTPL 1.17302 / 3.536		// 負荷抵抗1kΩ 最大300W
			#define CTSENSER_MLTPL 1.17302 / 1.167		// 負荷抵抗330Ω 最大900W
			#define CTSENSER_MLTPL 1.17302 / 0.3536		// 負荷抵抗100Ω 最大3kW
			
	AIN[mV] = (float)AIN /1023.0 * 1200.0
			= (float)AIN*1.17302;
			
	確認
			測定値 電力メータのN出力 835W
			電力メータ(400 Rev/kWh)1回転時間 42.9/4 =10.7秒
			消費電力 = 3600(秒)÷1回転時間(秒)÷0.400(Rev/Wh)
			＝ 841 W
			ほぼ一致 OK
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
	lcd.clear(); lcd.print("Sample 6 MyPLUG");
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
		xbee_gpio_config( dev_sens, 1 , AIN );	// 接続相手のポート1をAIN入力へ
		xbee_gpio_config( dev_sens, 2 , AIN );	// 接続相手のポート2をAIN入力へ
		xbee_gpio_config( dev_sens, 3 , AIN );	// 接続相手のポート3をAIN入力へ
		delay(1000);
		xbee_rat( dev_sens , "ATIR03E8");		// 測定間隔を1000ms(1秒)に設定
	}else{
		lcd.clear(); lcd.print("Failed:no dev.");
	}
	delay(500);
	lcd.clear(); lcd.print("DONE");
	
	// メイン処理
	lcd.clear(); lcd.print("Receiving");
	while(1){							// 永久に受信する

		/* 親機からスマートプラグへデータを要求する場合は以下5行を使用する */
/*		if( dev_en && trig == 0){
			xbee_force( dev_sens );		// デバイスdev_sensへデータ要求(100回に1回の頻度で)
			trig = 100;
		}
		trig--;
*/
		// 参考情報
		// 電池駆動するスマートプラグの省電力化の方法として「サンプリング要求方式」と
		// 「周期サンプリング方式」があり、前者の場合は上記の５行を有効にします。
		// この場合のxbee_rx_callの応答は「MODE_RESP」になります。
		// 後者の場合はこのままで動作します。この場合の応答は「MODE_GPIN」になります。
		// http://www.geocities.jp/bokunimowakaru/diy/xbee/xbee-sleep_cyclic.html

		/* データ受信(待ち受けて受信する) */
		xbee_rx_call( &xbee_result );
		switch( xbee_result.MODE ){		// 受信したデータの内容(MODE値)に応じて
			case MODE_RESP:				// スマートプラグへのデータ要求に対する応答
			case MODE_GPIN:				// スマートプラグから周期方式でデータ受信する
				if( bytecmp( dev_sens , xbee_result.FROM ,8 ) == 0 ){
					// 照度測定結果
					value = (float)xbee_result.ADCIN[1] * 1.17302 / 3.3;
					lcd.clear(); lcd.print( (unsigned int)value ); lcd.print("Lux ");
					// 温度測定結果
					value = (float)xbee_result.ADCIN[2] * 1.17302 / 10. - 60.;
					lcd.print( (unsigned int)value ); lcd.print("C");
					// 電力測定結果
					value = (float)xbee_result.ADCIN[3] * CTSENSER_MLTPL;
					lcd.setCursor(0,1); lcd.print( value , 1 ); lcd.print(" Watts");
				}
				break;
			default:
				break;
		}
	}
}
