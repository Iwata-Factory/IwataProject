/***************************************************************************************
ガスセンサの値を読み取ってキャラクタ液晶に表示する
DFROBOT LCD Keypad Shield用
メタンガスセンサ	MQ-4    RL=22kΩ
一酸化炭素センサ	MQ-7

                                                       Copyright (c) 2013 Wataru KUNINO
***************************************************************************************/
#include <xbee.h>
#include <LiquidCrystal.h>
#include "pitches.h"

#define		ALERT_CO		300					// 警報音を鳴らすCOガス量
#define		BEEP_CO			200					// 検出音を鳴らすCOガス量
#define		ALERT_CH4		500					// 警報音を鳴らすCH4ガス量
#define		BEEP_CH4		350					// 検出音を鳴らすCH4ガス量

#define		PIN_LED			13					// Digital 13に動作確認用LEDを接続
#define		PIN_BUZZER		2					// Digital 2にブザーを接続
#define		PIN_CO			0					// Analog 0に一酸化炭素ガスセンサ
#define		PIN_CH4			1					// Analog 1にメタンガスセンサ
#define		PIN_CO_VSS		11					// Digital 11にCOガスのVSS側
#define		PIN_CO_PWM		75					// COガスセンサVSS側のPWM値

#define		CO_Ro			2					// COガス100ppm時のセンサ抵抗値[kΩ]
#define		CH4_Ro			10					// メタン1000ppm時のセンサ抵抗値[kΩ]

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
byte trig=0;									// COガスセンサの測定状態
												// 00:冷却時間終了後に測定を実行してtrig=01を設定、CO電源5V(加熱)
												// 01:データを送信してtrig=02を設定
												// 02:加熱時間終了後にCO電源を1.4Vへ(冷却)trig=00へ
byte buzzer=0x00;								// ブザー状態
unsigned long time;								// 現在の時間
unsigned long time_next;						// 次回の測定状態変更時間
boolean time_ovf = false;						// 桁上がり状況のフラグ
int val;										// CH4,COガスの読み取り値
byte dev[]={0,0,0,0,0,0,0,0};                       // XBee親機アドレス（全てゼロ）

void setup(){
	byte i,j=0;
	pinMode(PIN_CO_VSS,OUTPUT);						// COガス用の電源ポートを出力に設定
	digitalWrite(PIN_CO_VSS,LOW);					// 出力を 0V に設定
	lcd.begin(16, 2);								// LCDのサイズを16文字×2桁に設定
	lcd.clear();									// 画面消去
	lcd.print("Gas Sensors");						// 文字を表示
	
	xbee_init( 0x00 );								// XBee用COMポートの初期化
	lcd.setCursor(0, 1);							// 液晶の２行目に移動
	lcd.print("Commissioning");						// 文字を表示
	while( xbee_ping(dev) == 0xFF ){				// ping応答を確認
		while( xbee_atai() > 0x01 ){				// ネットワーク参加状況を確認
			for(i=0;i<4;i++){
            	tone(PIN_BUZZER,NOTE_A7,20);        // ペアリング中を表す音を鳴らす
            	delay(500);
			}
			xbee_atcb(1);							// ネットワーク参加ボタンを押下
			j++;
			if(j>10){
				j=0;
				xbee_atcb(4);						// ネットワーク初期化
			}
		}
	}
	xbee_from(dev);									// ping応答のあったアドレスを保存
	time_next = millis() + 1000;					// 1秒後にCOガス測定状態を変更する
}

void loop(){
    byte i;
	float co=0;										// COガスの測定値
	float ch4=0;									// CH4ガスの測定値
    char s[40];										// 送信用の文字列
	XBEE_RESULT xbee_result;						// 受信データ(詳細)用の変数の定義
	
	time= millis();

	digitalWrite(PIN_LED,HIGH);						// 動作LEDの点灯
	if( time_ovf && time < 1000 ) time_ovf=false;	// timerリセット(50日に一度)処理
	if( time > time_next && !time_ovf){				// 状態変更時刻になった時
		switch(trig){
			case 0:									// 測定の実行
				val = analogRead(PIN_CO);  			// 一酸化炭素ガスセンサの値を取得
				co = pow(10,2.0-1.7*log10(10*(1023/(float)val-1)/CO_Ro));
				digitalWrite(PIN_CO_VSS,HIGH);		// COガスの電源を 5V に設定
				time_next += 60000;					// 60秒後に設定
				if( time_next < 60000 ) time_ovf = true;
				trig=1;								// 測定後に６０秒の過熱を実行
				break;
			case 2:									// 測定待ち
				analogWrite(PIN_CO_VSS,PIN_CO_PWM);	// COガスの電源を 1.4V に設定
				time_next += 90000;					// 90秒後に設定
				if( time_next < 90000 ) time_ovf = true;
				trig=0;								// 加熱後に９０秒の冷却を実行
				break;
		}
	}
	val = analogRead(PIN_CH4);						// メタンガスセンサ読み取り
	ch4 = pow(10,3.0-2.7*log10(22*(1023/(float)val-1)/CH4_Ro));
	
	/* ＣＨ４の表示 */
	lcd.setCursor(0, 1);							// 液晶の２行目に移動
	lcd.print("CH4=");								// 液晶に文字を表示
	if( ch4 < 10 ) lcd.print( ch4 , 2);				// 変数ch4の値を表示
	else lcd.print( ch4 , 0);
	lcd.print("ppm   ");							// 変数ch4の単位ppmを表示
	lcd.setCursor(12, 1);							// 液晶の２行目の後方へ移動
	lcd.print( (time/1000)%3600 );					// 経過時間を表示
	lcd.setCursor(12, 0);							// ＣＯセンサ加熱状態表示
	if( trig == 0 ) lcd.print("COOL");				// ※CO値はtrig=1の時のみの表示
	else lcd.print("HEAT");							//   につき、XBee通信部内で表示
	
	/* XBee通信 */
	if( co  >= 10000. )  co = 9999.;
	if( ch4 >= 10000. ) ch4 = 9999.;
	sprintf(s, "%04d,%04d", (int)co, (int)ch4 );	// XBee送信用データ作成
	if(trig==1){									// COの測定結果がある時(trig=1)
		xbee_uart(dev,s);                           // データを送信
		trig=2;
		buzzer=0x00;
		if(co > ALERT_CO) buzzer=0x01;					// COブザーを設定
		/* 表示 */
		lcd.clear();								// 液晶の画面消去
		lcd.print("CO =");
		if( co < 10 ) lcd.print( co , 2);			// 変数coの値を表示
		else lcd.print( co , 0);
		lcd.print("ppm");							// 変数coの単位ppmを表示
	}
	digitalWrite(PIN_LED,LOW);						// 動作LEDの消灯
	
	/* ブザー処理 */
	if(buzzer){										// ブザー値が存在する場合
		if(buzzer&0x01){							// トグル動作用
			if(buzzer&0x10)	tone(PIN_BUZZER,NOTE_B7,100);	// CH4用ブザー
			else			tone(PIN_BUZZER,NOTE_B7,200);	// CO用ブザー
			buzzer++;
		}else{
			if(buzzer&0x10)	tone(PIN_BUZZER,NOTE_G7,100);
			else			tone(PIN_BUZZER,NOTE_G7,200);
			buzzer--;
		}
	}else if(ch4 > ALERT_CH4) buzzer=0x11;				// CH4ブザーを設定
	else{
		if(ch4 > BEEP_CH4) tone(PIN_BUZZER,NOTE_A7,100);
		if( co > BEEP_CO ) tone(PIN_BUZZER,NOTE_A7,300);
	}
	
	for(i=0;i<50;i++){								// 約500ms間、繰り返し実行する
		xbee_rx_call( &xbee_result );				// データをxbeeから受信した時
		if(xbee_result.MODE==MODE_UART){
	        if(xbee_result.DATA[0] == 0x1B){        // ESCコードが先頭だったとき
	            if(xbee_result.DATA[1]=='B'){       // コマンドが'B'の時（Beep）
	                buzzer=1;                       // 警報音を鳴らす
	                if(	xbee_result.DATA[2]==0x00 ||
	                	xbee_result.DATA[2]==(byte)'0'){  // 値が0x00の時
	                    buzzer=0;                   // 警報音を止める
	                }
	            }else if(xbee_result.DATA[1]=='G'){ // コマンドが'G'の時（Get）
		            trig=0;                         // 次回のloopで測定して送信
		        }
	        }
		}
	}
}

/*
参考文献
	TECHNICAL DATA MQ-4 GAS SENSOR HANWAEI ELECTRONICS
	TECHNICAL DATA MQ-7 GAS SENSOR HANWAEI ELECTRONICS
*/
