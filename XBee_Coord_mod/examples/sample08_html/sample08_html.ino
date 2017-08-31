/*********************************************************************
XBeeセンサーから受信した内容をhttp表示する。
ハードは以下の組み合わせで動作確認しています。

	Arduino Ethernet + Wireless SD (XBee) + Adafruit LCD

※EthernetのMACアドレスとIPアドレスの設定が必要です。

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
Arduino Ethernet 使用上の注意点

ソース内でEthernetのMACアドレスとIPアドレスの設定が必要です。
設定しなくても動作しますが、ハードウェアごとに異なるアドレスを設定
しなければ、干渉などの不具合が発生しますので、必ず、設定してください。
また、Ethernet部に異常が発生した場合、リセットボタンでは解消されない
場合があります。(電源の入れ直しで解消できます。)
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

#include <SPI.h>
#include <Ethernet.h>

/*********************************************************************
Arduino Ethernet の MACアドレス(裏面に記載)を記入します。       */
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x00, 0x00 };

/*********************************************************************
Arduino Ethernet のIPアドレスとHTTPサーバのポート番号(通常80)を設定します。*/
IPAddress ip(192,168,0, 118);
EthernetServer server(80);

byte dev_sens[]   = {0x00,0x13,0xA2,0x00,0x40,0x00,0x00,0x00};	// 仮設定
byte dev_en = 0;	// センサー発見の有無(0:未発見)

void setup(){
	lcd.begin(16, 2);
	lcd.clear(); lcd.print("Sample 8 HTML");
	xbee_init( 0x00 );					// XBee用COMポートの初期化(引数はポート番号)

	// デバイス探索
	lcd.clear(); lcd.print("Searching:SENSOR");
	if( xbee_atnj(10) ){				// 10秒間の参加受け入れ
		lcd.clear(); lcd.print("Found a device");
		xbee_from( dev_sens );			// 受信したアドレスをdev_sensへ格納
		dev_en = 1;						// sensor発見済
		xbee_gpio_config( dev_sens, 1 , AIN ); // 接続相手のポート1をAIN入力へ
		xbee_gpio_config( dev_sens, 2 , AIN ); // 接続相手のポート2をAIN入力へ
		// xbee_gpio_config( dev_sens, 3 , AIN ); // 接続相手のポート3をAIN入力へ
	}else{
		lcd.clear(); lcd.print("Failed:no dev.");
	}
	delay(500);
	lcd.clear(); lcd.print("DONE");
	
	// start the Ethernet connection and the server:
	Ethernet.begin(mac, ip);
	server.begin();
	lcd.clear();
	lcd.print(Ethernet.localIP());
}

void loop(){
	byte i;
	byte trig=0;
	float value_lux;		// 照度の受信値
	float value_temp;		// 温度の受信値

	XBEE_RESULT xbee_result;			// 受信データ用の構造体
	
	EthernetClient client;
	boolean currentLineIsBlank;	// an http request ends with a blank line
	char c;
	
	// メイン処理
	while(1){							// 永久に受信する
		if( dev_en && trig == 0){
			xbee_force( dev_sens );		// デバイスdev_sensへデータ要求(100回に1回の頻度で)
			trig = 100;
		}
		trig--;
		/* データ受信(待ち受けて受信する) */
		xbee_rx_call( &xbee_result );
										// データを受信します。

		switch( xbee_result.MODE ){		// 受信したデータの内容(MODE値)に応じて
			case MODE_RESP:				// sensorからの応答の場合に照度と温度を表示
			case MODE_GPIN:				// 周期方式でデータ受信した場合も想定
				if( bytecmp( dev_sens , xbee_result.FROM ,8 ) == 0 &&
					xbee_result.STATUS == STATUS_OK ){
					// 照度測定結果
					value_lux = xbee_sensor_result( &xbee_result, LIGHT);
					lcd.clear(); lcd.print( value_lux , 1 );	lcd.print(" Lux ");
					// 温度測定結果
					value_temp = xbee_sensor_result( &xbee_result, TEMP);
					lcd.setCursor(0,1); lcd.print( value_temp , 1 ); lcd.print(" C");
				}
				break;
			case MODE_IDNT:				// 新しいデバイスを発見
				lcd.clear(); lcd.print("found a new dev");
				for( i=0;i<8;i++ ) dev_sens[i]=xbee_result.FROM[i];
										// 発見したアドレスをdev_sensに読み込み
				dev_en = 1;				// sensor発見済
				xbee_gpio_config( dev_sens, 1 , AIN ); // 接続相手のポート1をAIN入力へ
				xbee_gpio_config( dev_sens, 2 , AIN ); // 接続相手のポート2をAIN入力へ
				// xbee_gpio_config( dev_sens, 3 , AIN ); // 接続相手のポート3をAIN入力へ
				break;
			default:
				break;
		}

		client = server.available();
		if (client) {
			lcd.clear();
			lcd.print("new client");
			currentLineIsBlank = true;
			while (client.connected()) {
				if (client.available()) {
					c = client.read();
					if (c == '\n' && currentLineIsBlank) {
						// send a standard http response header
						client.println("HTTP/1.1 200 OK");
						client.println("Content-Type: text/html");
						client.println("Connnection: close");
						client.println();
						client.println("<!DOCTYPE HTML>");
						client.println("<html><head>");
						client.println("<meta charset=\"UTF-8\">");
						// add a meta refresh tag, so the browser pulls again every 10 seconds:
						client.println("<meta http-equiv=\"refresh\" content=\"10\">");
						client.println("</head><body>");
						// output the value
						client.println("<h3>XBee Sensors</h3><table border=1>");
						client.print("<tr><td>照度センサ</td><td>");
						client.print( value_lux , 2);
						client.println("lux</td>");
						client.print("<tr><td>温度センサ</td><td>");
						client.print( value_temp , 2);
						client.println("℃</td>");
						client.println("</table>");
						client.println("</body></html>");
						break;
					}
					if (c == '\n') {				// you're starting a new line
						currentLineIsBlank = true;
					} else if (c != '\r') {			// you've gotten a character on the current line
						currentLineIsBlank = false;
					}
				}
			}
			delay(1);			// give the web browser time to receive the data
			client.stop();		// close the connection:
		}
	}
}

