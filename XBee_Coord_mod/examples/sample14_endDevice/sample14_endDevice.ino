/***************************************************************************************
当ライブラリをエンドデバイス上で実行する場合のサンプルです。

スリープ信号	Arduino Analog 6 -> XBee pin 9
SDのCS信号		Arduino Digital 4 (Wireless Shield)
動作確認用LED	Arduino Digital 7
												 Copyright (c) 2012-2013 Wataru KUNINO
												 http://www.geocities.jp/bokunimowakaru/
***************************************************************************************/
#include <xbee.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include <SD.h>
#define 	PIN_SDCS			4				// SDのチップセレクトCS(SS)を接続したポート
#define 	PIN_XB_SLEEP		17				// Analag 3 XBeeのスリープ端子
#define		TEMP_OFFSET 		0.0 			// Arduino内部温度上昇の補正用
#define 	SLEEP_DURATION		180 			// 1分 休止間隔(秒)
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
File file;										// SDファイルの定義

byte dev[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; // オール0はCoordinator
char tx_s[8];									// 送信用の文字列
char time_s[13];								// 文字列生成用のバッファ

void xbee_wake(){
	digitalWrite(PIN_XB_SLEEP, HIGH);			// XBeeのスリープ設定
	delay(10);
	digitalWrite(PIN_XB_SLEEP, LOW);			// XBeeのスリープ解除
	delay(10);
}

void xbee_setup(){
	xbee_at("ATST03E8");						// Time before Sleep を1秒に設定
	xbee_at("ATSP07D0");						// 受信間隔を20秒に設定
	xbee_at("ATSM05");							// Cyclic Sleep + PIN wakeモードに設定
}

void get_time(){
	unsigned long time = millis() / 1000;
	sprintf(time_s,"%02d,%02d:%02d:%02d,",
		(int)(time/86400),						// 経過日数
		(int)(time/3600)%24,					// 経過時数
		(int)((time/60)%60),					// 経過分数
		(int)(time%60));						// 経過秒数
}

void sd_log(char *in){
	byte i,b;
	lcd.clear();
	get_time();
	file=SD.open("ENDDEV.CSV", FILE_WRITE);		// 書込みファイル「test.csv」のオープン
	if(file == true){							// オープンが成功した場合
		file.print(time_s); 					// 文字列を記録
		lcd.print( time_s );
		for(i=6;i<8;i++){					// 宛先アドレスの下2バイトをきする
			b = dev[i]/16;
			file.print(b,HEX);
			lcd.print(b,HEX);
			b = dev[i]%16;
			file.print(b,HEX);
			lcd.print(b,HEX);
		}
		file.print(',');
		file.println(in);						// 文字列を記録
		file.close();							// ファイルを閉じる(実際にSDへ書き込む)
		lcd.setCursor(0, 1);
		lcd.print( in );
	}else{
		lcd.print("SD Err");
	}
}

void setup(){
	pinMode(PIN_XB_SLEEP, OUTPUT);				// XBeeのスリープ端子
	lcd.begin(16, 2);								// LCDのサイズを16文字×2桁に設定
	lcd.print("Sample EndDevice");					// 文字を表示
	while(SD.begin(PIN_SDCS)==false){			// SDカードの開始
		delay(2000);							// 失敗時は5秒ごとに繰り返し実施
	}
	sd_log("Reboot");
	xbee_wake();								// XBee ZBモジュールのスリープ解除
	xbee_init( 0x00 );							// XBee用COMポートの初期化
	xbee_setup();								// XBeeの省電力設定
}

void loop(){
	int i;
	byte j=0;
	float temp; 								// 温度

	sd_log("Hello");
	xbee_wake();								// XBeeのスリープを解除する
	while( xbee_ping(dev) == 0xFF ){			// ping応答を確認
		sd_log("CB1");
		while( xbee_atai() > 0x01 ){			// ネットワーク参加状況を確認
			delay(2000);
			xbee_wake();
			xbee_atcb(1);						// ネットワーク参加ボタンを押下
			j++;
			if(j>10){
				j=0;
				xbee_atcb(4);					// ネットワーク初期化
				sd_log("CB4");
			}
		}
		xbee_setup();							// XBeeの省電力設定
		delay(1000);
	}
	xbee_from(dev); 							// ping応答のあったアドレスを保存

	/* 測定 */
	get_time();
	temp=getTemp()-TEMP_OFFSET;
	sprintf(tx_s,"%d.%02d",(int)temp,(int)(temp*100)%100);
	sd_log(tx_s);

	xbee_wake();								// XBee ZBモジュールのスリープ(一時)解除
	xbee_uart( dev , time_s);					// 測定結果（文字列）の送信
	xbee_uart( dev , tx_s); 					// 測定結果（文字列）の送信
	delay(100); 								// 送信完了待ち
	digitalWrite(PIN_XB_SLEEP, LOW);			// XBeeのスリープ解除

	/* 待機 */
	for(i=0;i<SLEEP_DURATION;i++) delay(1000);
}
