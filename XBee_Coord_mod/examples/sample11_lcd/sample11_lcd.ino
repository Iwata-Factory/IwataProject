/*********************************************************************
他のXBeeから送られてきたテキストデータをキャラクタＬＣＤに表示します。

このスケッチは子機用（M2M子機）です。センサーは、孫機となります。
親機用としても使えます。

本Arduino機に接続するXBeeモジュールは「ROUTER API」のファームウェアを
書き込んでください。
もちろん、「COORDINATOR API」を書き込むことで親機としても動作します。
	
本ソースリストおよびソフトウェアは、ライセンスフリーです。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

							   Copyright (c) 2013- Wataru KUNINO
							   http://www.geocities.jp/bokunimowakaru/
*********************************************************************/
/*
コンパイル時の注意点：

			下記の時計ライブラリTimeを使用します

			http://playground.arduino.cc/Code/Time

機能説明

・テキストを受信すると改行までの文字をＬＣＤへ出力します。
・表示しきれない場合はスクロール表示します。
・エスケープコマンド(0x1B)を受信するとコマンドに応じてレベル表示などを
　行います。
*/
/*
機能一覧

	テキスト表示　：親機から送られてきたテキスト文字を表示する機能
	メータ表示機能：１０段階のレベルメータ（２個）を表示する機能
	アラート機能　：親機からの指示でアラート音、チャイム音を鳴らす機能
	「上」ボタン　：親機へのコミッショニング機能（コミッショニングボタン）
	「下」ボタン　：ジョイン許可機能（本機へ子機を参加させる機能）
	ペアリング　　：ジョイン許可して本機が孫機とペアリングする機能
	データ要求　　：ペアリングした孫機へアナログ値のデータを要求する機能
	連続データ要求：データ要求を繰り返し実行する機能
	温度測定機能　：Arduino内蔵の温度計で温度測定する機能（Arduino UNO用）
	測定結果送信　：温度測定結果を送信する機能
	コマンド送信　：親機へコマンド（UART）を送信する機能（コマンド変更可能）
	テキスト送信　：親機へテキスト文字（UART）を送信する機能（テキスト変更可能）
	時計表示機能　：親機から受け取った時刻補正情報を元に時計表示を行います
	PAN ID表示　　：本機のPAN IDを表示する機能
	初期化　　　　：本機のネットワーク設定などを初期化する機能
*/

/*
画面設計

	起動時					メニュー表示			レベル表示時
	　0123456789012345		　0123456789012345		　0123456789012345
	┏━━━━━━━━┓	┏━━━━━━━━┓	┏━━━━━━━━┓
	┃M2M Display ----┃	┃LED_SW		00┃	┃----- ----- I=00┃
	┃text message....┃	┃Sent GPIO(11)= 1┃	┃26.5C 3000W 1000┃
	┗━━━━━━━━┛	┗━━━━━━━━┛	┗━━━━━━━━┛
	
*/
/*
親機からのUARTコマンド

	レベルメータ表示

		[ESC] [L] [0～9,A]			レベルメータ１の表示レベル(0～10段)
		[ESC] [L] [0～9,A][0～9,A]	レベルメータ１と２の表示レベル(0～10段)
		[ESC] [L] [0x00～0x0A]		レベルメータ１の表示レベル(0～10段)
		[ESC] [L] [0x10～0x1A]		レベルメータ２の表示レベル(0～10段)

	ブザー出力

		[ESC] [A] [0～9,A] 			アラート音を鳴らす(0～9回),0＝OFF
		[ESC] [B] [0～9,A] 			ベル音を鳴らす(0～10回),0＝OFF
		[ESC] [C] [1]				チャイム音(1回)
		
	UART送信データのユーザ定義
		
		[ESC] [M] [1～2] [TEXT]		メニュー名称(10文字まで)
		[ESC] [U] [1～2] [UART]		送信データ(8バイトまで)

	時刻合わせデータ
	
		[ESC] [T] [h][h][:][m][m]	時刻合わせ用のデータ

*/
/*
Arduinoへの書き込み時の注意点

書き込み時にXBeeからの受信があると、書き込みに失敗する場合があります。
全ての子機をオフにする、もしくはArduinoに取り付けた親機を外してから
書き込んでください。
*/
/*********************************************************************
インポート処理						※ライブラリ XBeeに必要な宣言です
*********************************************************************/

#include <avr/wdt.h>
#include <xbee.h>
#include "pitches.h"
#define 	PIN_BUZZER		2							// Digital 2にスピーカを接続
#include <EEPROM.h>
#include <Time.h>

/*********************************************************************
アプリケーション					※以下を編集してアプリを作成します
*********************************************************************/

//define ADAFRUIT	// 使用する液晶が Adafruit LCD Sheild の時に定義する

#ifndef ADAFRUIT
	// DFRobot製キャラクタLCDの場合
	#include <LiquidCrystalDFR.h>
	LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#else
	// Adafruit I2C接続LCDの場合(Adafruit_RGBLCDShieldライブラリが必要)
	#include <Wire.h>
	#include <Adafruit_MCP23017.h>
	#include <Adafruit_RGBLCDShield.h>
	Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#endif

const static byte coord[]
					= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};	// 親機(保持用)
byte dev_gpin[]		= {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF};	// 孫機(保持用)
byte dev_tmp[]		= {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF};	// 一時保管用
byte device_type	= ZB_TYPE_COORD; 				// ZB_TYPE_ROUTER ZB_TYPE_ENDDEV

byte counter;
#define MENU_MAX 15
byte menu_n = 0;
char menu_s[MENU_MAX][11]={	"Pairing   ",			// 0 孫機とペアリング
							"GPIO4  OFF",			// 1 GPIO制御
							"Force Shot",			// 2 データ要求
							"Repeat ON ",			// 3 データ連続要求
							"DispLv OFF",			// 4 メーター表示オフ
							"Temper ON ",			// 5 温度測定オン
							"TxRepo ON ",			// 6 測定結果の親機への送信をオン
							"Send ESC+A",			// [7] ESC+Aを送信	// menu[7-8]処理
							"Send Hello",			// [8] テキスト文字Helloを送信
							"Clock  ON ",			// 9 時計表示オン
							"Remaining ",			// 10 ペアリング可能な孫機数
							"Ope PAN ID",			// 11 PAN ID取得
							"Address   ",			// 12 アドレスリストの表示
							"Reset NetW",			// 13 ZBネットリセット
							"Reset All "};			// 14 EEPROMの初期化
char send_data[2][9]=	{	"\0A\0\0\0\0\0\0",
							"Hello\0\0\0" };

char s[17]	   = "by Wataru Kunino";
char d[5]	   = "----";
byte level_1   = 0;
byte level_2   = 0;
byte alert_value = 0;
byte buzzer=0x00;									// ブザー状態
boolean gpio = 1;									// GPIO4の状態
boolean force_repeat = 0;							// 測定の繰り返し 0=OFF／1=ON
boolean tx_rep = 0; 								// 測定結果の報告 0=OFF／1=ON
boolean disp_lv_chr = 1;							// 結果表示 0=レベル／1=数値表示
boolean dev_gpin_en = 0;							// 孫機とのペアリング状況
boolean temper = 0;									// 温度計モードオフ
boolean clock = 0;									// 時計表示モードオフ
boolean clock10 = 0;								// 時計表示で10～12時の時
#define TEMP_OFFSET     0.0                         // Arduino内部温度上昇の補正用

byte font_lv[5][8]={
	{
		0b00000,
		0b10000,
		0b00000,
		0b10000,
		0b00000,
		0b10000,
		0b00000,
		0b10101
	},{
		0b11000,
		0b11000,
		0b11000,
		0b11000,
		0b11000,
		0b11000,
		0b11000,
		0b10101
	},{
		0b11011,
		0b11011,
		0b11011,
		0b11011,
		0b11011,
		0b11011,
		0b11011,
		0b10101
	},{
		0b00000,
		0b10000,
		0b00000,
		0b10000,
		0b00000,
		0b10000,
		0b00000,
		0b10000
	},{
		0b00010,
		0b10110,
		0b00010,
		0b10010,
		0b00010,
		0b10010,
		0b00111,
		0b10000
	}
};
/*
	　０１２３４　０１２３４　０１２３４　０１２３４　０１２３４
	０●　　　　　●　　　　　●　　　　　●　　　　　●　　　　
	１
	２●　　　　　●　　　　　●　　　　　●　　　　　●　　　　
	３
	４●　　　　　●　　　　　●　　　　　●　　　　　●　　　　
	５
	６●　●　●　●　●　●　●　●　●　●　●　●　●　●　●
	
	　０１２３４　０１２３４　０１２３４　０１２３４　０１２３４
	０●●　　　　●　　　　　●　　　　　●　　　　　●　　　　
	１●●
	２●●　　　　●　　　　　●　　　　　●　　　　　●　　　　
	３●●
	４●●　　　　●　　　　　●　　　　　●　　　　　●　　　　
	５●●
	６●　●　●　●　●　●　●　●　●　●　●　●　●　●　●
	
	　０１２３４　０１２３４　０１２３４　０１２３４　０１２３４
	０●●　●●　●　　　　　●　　　　　●　　　　　●　　　　
	１●●　●●
	２●●　●●　●　　　　　●　　　　　●　　　　　●　　　　
	３●●　●●
	４●●　●●　●　　　　　●　　　　　●　　　　　●　　　　
	５●●　●●
	６●　●　●　●　●　●　●　●　●　●　●　●　●　●　●
	

	　０１２３４　０１２３４　０１２３４　０１２３４　０１２３４
	０●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	１●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	２●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	３●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	４●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	５●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	６●　●　●　●　●　●　●　●　●　●　●　●　●　●　●

*/

void lcd_cls( const byte line ){						// 指定した行を消去する関数
	lcd.setCursor(0,line);
	for(byte i=0;i<16;i++)lcd.print(" ");
	lcd.setCursor(0,line);
}

void lcd_print_hex(const byte in){						// 16進数2桁(1バイト)の表示関数
	lcd.print( in>>4 , HEX );
	lcd.print( in%16 , HEX );
}

void lcd_print_level(const byte lev1, const byte lev2){
	byte i;

	lcd.setCursor( 0 , 0 );
	if(disp_lv_chr){
		for(i=0 ; i < 5 ; i++){
			if( lev1 <= 2 * i )			lcd.write( (byte)0x00 );
			else if( lev1 - 2 * i == 1 )	lcd.write( (byte)0x01 );
			else							lcd.write( (byte)0x02 );
		}
		if(lev1>10) lcd.print("F"); else lcd.write(3);
		for(i=0 ; i < 5 ; i++){
			lcd.setCursor( i+6 , 0 );
			if( lev2 <= 2 * i )			lcd.write( (byte)0x00 );
			else if( lev2 - 2 * i == 1 )	lcd.write( (byte)0x01 );
			else							lcd.write( (byte)0x02 );
		}
		if(lev2>10) lcd.print("F"); else{
			if( clock && clock10 )lcd.write(4); else lcd.write(3);
		}
	}else{
		lcd.print("1:");
		if( lev1 < 10) lcd.print(" ");
		lcd.print(lev1);
		lcd.print(" 2:");
		if( lev2 < 10) lcd.print(" ");
		lcd.print(lev2);
		lcd.print("  ");
		if( clock && clock10 )lcd.write(4); else lcd.write(3);
	}
}

void lcd_print_result(XBEE_RESULT *xbee_result){
	byte i;
	byte ain_n=0;		// アナログ入力数
	
	lcd_cls(1);
	for(i=1;i<=3;i++){
		if( xbee_result->ADCIN[i] < 1024){
			lcd.print( xbee_result->ADCIN[i] , DEC );
			lcd.print( " " );
			ain_n++;
			if(ain_n==1)		level_1=(byte)(xbee_result->ADCIN[i] / 102 );
			else if(ain_n==2)	level_2=(byte)(xbee_result->ADCIN[i] / 102 );
		}
	}
	if(ain_n==0){
		for(i=0;i<8;i++){
			lcd.print( (xbee_result->DATA[0]>>(7-i))&0x01 , BIN );
		}
	}
}

void lcd_print_address(char *str, byte *dev ){			// アドレス表示用の関数
	byte i;
	byte start = 4; 									// アドレスの表示開始バイト
	lcd_cls(1); 										// 2行目を消去
	if( str[0] == '\0' ) start=0;						// strに文字なし時は8バイト表示
	else lcd.print( str );								// 文字あり時は文字+4バイト表示
	for(i=start;i<8;i++) lcd_print_hex(dev[i]); 		// アドレスを液晶に表示する
}

void menu_onoff(byte n, boolean onoff){
	if( onoff == 0){
		menu_s[n][7]='O';
		menu_s[n][8]='N';
		menu_s[n][9]=' ';
	}else{
		menu_s[n][7]='O';
		menu_s[n][8]='F';
		menu_s[n][9]='F';
	}
}

void setup(){
	byte i;
	
	/* 初期化処理 */
	lcd.begin(16, 2);
	lcd.clear(); lcd.print("M2M Display ----"); 		// 起動時の表示
	lcd.setCursor(0,1); lcd.print(s);
	xbee_init( 0x00 );									// XBee用COMポートの初期化
	xbee_atnj( 0 );							// ジョイン禁止
	xbee_at("ATSP0AF0");					// 子機データ保持期間(28秒)を設定
	xbee_at("ATSN0E10");					// SN=3600回 (28秒*3600回=28時間)
	for(i=0 ; i < 5 ; i++ ){
		lcd.createChar( i , &(font_lv[i][0]) );
	}
	
	/* ユーザ設定のEEPROMからの読み出し */
	send_data[0][0] = 0x1B;								// ESCコードを代入
	for(i=0;i<16;i++) if( EEPROM.read(i) != s[i] ) break;
	if( i < 16 ){
		lcd_cls(1); lcd.print("Writing EEPROM");
		for(i=0;i<16;i++)  EEPROM.write(i, s[i] );
		for(i=16;i<24;i++) EEPROM.write(i, send_data[0][i-16] );
		for(i=24;i<34;i++) EEPROM.write(i, menu_s[7][i-24] );	// menu[7-8]処理
		for(i=40;i<48;i++) EEPROM.write(i, send_data[1][i-40] );
		for(i=48;i<58;i++) EEPROM.write(i, menu_s[8][i-48] );	// menu[7-8]処理
		xbee_delay(500);
	}else{
		for(i=16;i<24;i++) send_data[0][i-16]=EEPROM.read(i);
		for(i=24;i<34;i++) menu_s[7][i-24]   =EEPROM.read(i);	// menu[7-8]処理
		for(i=40;i<48;i++) send_data[1][i-40]=EEPROM.read(i);
		for(i=48;i<58;i++) menu_s[8][i-48]   =EEPROM.read(i);	// menu[7-8]処理
	}
	
	/* ZigBeeタイプの確認と表示用変数sへの代入 */
	device_type=xbee_atvr();							// XBeeのZigBee種別を取得
	switch( device_type ){
		case ZB_TYPE_COORD:  sprintf(s,"ZigBee Coordinat"); break;
		case ZB_TYPE_ROUTER: sprintf(s,"ZigBee Router   "); break;
		case ZB_TYPE_ENDDEV: sprintf(s,"ZigBee EndDevice"); break;
	}
}

void loop(){
	byte i,j;
	byte data;
	byte buttons;
	byte str_len;
	char buf[17];
	XBEE_RESULT xbee_result;							// 受信データ用の構造体

	// メイン処理
	buttons = lcd.readButtons();
	if( counter >= 200 ){
		j=xbee_atai();
		lcd.setCursor(12,0);
		if(j>0){
			lcd.print("I=");
			lcd_print_hex(j); 							// XBeeの状態表示
		}else lcd.print( d );
		lcd.setCursor(0,1);								// 画面消去は行わない(点滅防止)
		lcd.print( s );									// sは空白を含めて必ず16文字
		if(force_repeat) xbee_force(dev_gpin);			// 孫機へデータ取得指示の送信
		
		if(alert_value){
			for(i=0;i<4;i++){
				tone(PIN_BUZZER,NOTE_B7,200);
				xbee_delay(250);
				tone(PIN_BUZZER,NOTE_G7,200);
				xbee_delay(250);
			}
			tone(PIN_BUZZER,NOTE_B7,1000);
			alert_value--;
		}
		if(clock){
			byte hh = (byte)hour();
			byte mm = (byte)minute();
			if (hh>12) hh -= 12;
			if (hh>=10) clock10=1; else clock10=0;
			lcd_print_level( level_1, level_2);
		//	d[0] = (char)( hh/10 )+'0';
		//	d[1] = (char)( hh%10 )+'0';
			d[0] = (char)( hh%10 )+'0';
			d[1] = ':';
			d[2] = (char)( mm/10 )+'0';
			d[3] = (char)( mm%10 )+'0';
			lcd.setCursor(12,0);
			lcd.print(d);
		}
		if(temper){
			float t=-1;
			#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328__) || defined(__AVR_ATmega8__)
				while(lcd.readButtons()) xbee_delay(100);
				byte b_ADMUX = ADMUX;
				byte b_ADCSRA = ADCSRA;
				
				ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
				ADCSRA |= _BV(ADEN);  // enable the ADC
				xbee_delay(20);            // wait for voltages to become stable.
				ADCSRA |= _BV(ADSC);  // Start the ADC
				while (bit_is_set(ADCSRA,ADSC));	  // Detect end-of-conversion
				unsigned int wADC = ADCW;
								ADCSRA = b_ADCSRA;
				ADMUX = b_ADMUX;
				
				t =(wADC - 324.31 ) / 1.22;
			#endif
			
			if(t>0){
				if(t>10 && t<100) d[0] = (char)( t/10+'0'); else d[0]=' ';
				d[1]=(char)( (int)t%10 + '0');
				d[2]='.';
				d[3]=(char)( (int)(t*10)%10 + '0');
			}else{
				d[0]='-';
				if(t <= -10 && t > -100) d[1] = (char)( (char)((-t)/10)+'0');
				else{ d[0]=' '; d[1]='-'; }
				d[2]=(char)( (int)(-t)%10 + '0');
				d[3]='.';
			}
			if( tx_rep ) xbee_uart(coord,d);
			lcd.setCursor(12,0);
			lcd.print(d);
			
			analogReference(DEFAULT);
			while(lcd.readButtons()) xbee_delay(100);
		}
		counter = 0;
	}
	counter++;
	if( buttons != 0 ){
		tone(PIN_BUZZER,NOTE_A6,100);
		counter = 0;
		lcd_cls(1); 									// 文字表示行の消去
		switch( buttons ){
			case BUTTON_UP:
				lcd.print("Commisioning");
				xbee_atcb(1);							// 親機へのジョイン開始
				xbee_delay(3000);
				if( xbee_ping( coord ) != 0xFF){		// PAN_IDの取得
					xbee_from( dev_tmp );				// PING応答元のアドレスを取得
					lcd_print_address("COORD. =",dev_tmp);
				}else lcd.print("Fail");
				break;
			case BUTTON_DOWN:
				if(device_type != ZB_TYPE_ENDDEV){
					lcd.print("Enabled Net Join");
					xbee_atcb(2);						// 孫機のジョイン許可
				}
				break;
			case BUTTON_LEFT:
				if( menu_n > 0 ) menu_n--;
				lcd.print( menu_s[menu_n] );
				break;
			case BUTTON_RIGHT:
				if( menu_n < (MENU_MAX-1) ) menu_n++;
				lcd.print( menu_s[menu_n] );
				break;
			case BUTTON_SELECT:
				lcd.print( menu_s[menu_n] ); lcd.print("> DONE");
				switch( menu_n ){
					case 0:
						if(device_type != ZB_TYPE_ENDDEV){
							sprintf(d,"Pair");
							xbee_atcb(2);				// 0 "Pairing   " 孫機とペアリング
							dev_gpin_en=0;				// (Join Netとの違い＝デバイス登録)
						}
						break;
					case 1: gpio=!gpio;					// 1 GPIO制御
						menu_onoff(menu_n,gpio);
						xbee_gpo(dev_gpin,4,gpio);
						break;
					case 2: xbee_force( dev_gpin ); 	// 2 "Force Shot" データ要求
						break;
					case 3: force_repeat=!force_repeat; // 3 "Repeat ON " データ連続要求
						menu_onoff(menu_n,force_repeat);
						break;
					case 4: disp_lv_chr=!disp_lv_chr;	// 4 "Disp=Level" メーター表示オフ
						menu_onoff(menu_n,disp_lv_chr);
						lcd_print_level( level_1, level_2);
						break;
					case 5: temper=!temper;				// 5 "Temper ON " 温度測定
						menu_onoff(menu_n,temper);
						break;
					case 6: tx_rep=!tx_rep;				// 6 "TxRepo ON " 結果情報の送信
						menu_onoff(menu_n,tx_rep);
						break;
					case 7:								// 7 ESC+Aを送信
					case 8:								// 8 テキスト文字Helloを送信
						xbee_uart(coord,send_data[menu_n-7]);	// menu[7-8]処理
						break;
					case 9: clock=!clock;				// 9 "Clock  ON" 時計表示オン
						menu_onoff(menu_n,clock);
						sprintf(d,"-:--");
						lcd_print_level( level_1, level_2);
						break;
					case 10:							// 10 "Remaining" 孫機数
						j=xbee_atnc();
						if		(device_type == ZB_TYPE_COORD)	i = 10 -j;
						else if	(device_type == ZB_TYPE_ROUTER)	i = 12 -j;
						else 	i = 0;
						sprintf(s,"Rem %2d/%2d (%2d)  ",j,i+j,i);
						break;
					case 11: xbee_atop( dev_tmp );		// 11 "Ope PAN ID" PAN ID取得
						lcd_print_address("",dev_tmp);
						tone(PIN_BUZZER,NOTE_A7,100);
						xbee_delay(3000);
						xbee_ping( coord );
						xbee_from(dev_tmp);
						lcd_print_address("Coord  =",dev_tmp);
						break;
					case 12:							// 12 "Address" アドレスリスト
						xbee_myaddress(dev_tmp);
						lcd_print_address("My IEEE=",dev_tmp);
						tone(PIN_BUZZER,NOTE_A7,100);
						xbee_delay(3000);
						xbee_ping( coord );
						xbee_from(dev_tmp);
						lcd_print_address("Coord  =",dev_tmp);
						if(dev_gpin_en){
							tone(PIN_BUZZER,NOTE_A7,100);
							xbee_delay(3000);
							lcd_print_address("Child  =",dev_gpin);
						}
						break;
					case 13: xbee_atcb(4);				// 13 "Reset NetW" ZBネットリセット
						tone(PIN_BUZZER,NOTE_A7,10000);
						xbee_delay(3000);
						wdt_enable(WDTO_15MS);			// setup();
						while(1);
						break;
					case 14:							// 14 EEPROMの初期化
						EEPROM.write(0,0xFF);
						tone(PIN_BUZZER,NOTE_A7,10000);
						xbee_delay(3000);
						wdt_enable(WDTO_15MS);			// setup();
						while(1);
						break;
					default:
						break;
				}
				xbee_delay(200);
				tone(PIN_BUZZER,NOTE_A7,100);
				break;
			default:
				break;
		}
		while( lcd.readButtons() ) xbee_delay(100);	// 最後のキーが残っていた場合
		xbee_delay(100);
	}

	/* データ受信(待ち受けて受信する) */
	data = xbee_rx_call( &xbee_result );

	if( xbee_result.MODE ){						// 何かを受信した
		counter = 0;
		switch( xbee_result.MODE ){				// 受信したデータの内容(MODE値)に応じて
			case MODE_UART:						// シリアルデータを受信
				if( data >= 3 && xbee_result.DATA[0] == 0x1B ){	// ESCコードが先頭だったとき
					switch( xbee_result.DATA[1]  ){	
						case (byte)'L':
							if( xbee_result.DATA[2] >= 0x00 && 
								xbee_result.DATA[2] <= 0x0A )
										level_1 = xbee_result.DATA[2];
							else if( xbee_result.DATA[2] >= (byte)'0' && 
									 xbee_result.DATA[2] <= (byte)'9' )
										level_1 = xbee_result.DATA[2] - (byte)'0';
							else if( xbee_result.DATA[2] == (byte)'A' )
										level_1 = 10;
							else if( xbee_result.DATA[2] >= 0x10 && 
									 xbee_result.DATA[2] <= 0x1A )
										level_2 = xbee_result.DATA[2] - 16;
							if( data >= 4 ){
								if( xbee_result.DATA[3] >= 0x00 && 
									xbee_result.DATA[3] <= 0x0A )
										level_2 = xbee_result.DATA[3];
								else
								if( xbee_result.DATA[3] >= (byte)'0' && 
									xbee_result.DATA[3] <= (byte)'9' )
										level_2 = xbee_result.DATA[3] - (byte)'0';
								else
								if ( xbee_result.DATA[3] == (byte)'A' )
										level_2 = 10;
							}
							lcd_print_level( level_1, level_2);
							break;
						case (byte)'A':
							sprintf(d,"Alrt");
							alert_value = xbee_result.DATA[2];
							if( alert_value >= (byte)'0' && alert_value <= (byte)'9' ) alert_value -= (byte)'0';
							if( alert_value > 10 ) alert_value = 10 ;
							alert_value *= 10;
							break;
						case (byte)'B':
							sprintf(d,"Beep");
							if( xbee_result.DATA[2] <= 0x0A ) buzzer = xbee_result.DATA[2]; // ブザー値を設定
							else if( xbee_result.DATA[2] >= (byte)'0' && xbee_result.DATA[2] <= (byte)'9'){
								buzzer = xbee_result.DATA[2] - (byte)'0';
							}
							break;
						case (byte)'C':	// コマンドは3文字以上なので[ESC][C][1]などを入力する
							sprintf(d,"Chim");
							tone(PIN_BUZZER,NOTE_CS6,800);
							xbee_delay(1000);
							tone(PIN_BUZZER,NOTE_A5,1000);
							break;
						case (byte)'U':	// [ESC]+[U]+[メニュー番号]+[送信UART情報(8バイトまで)]
							i=0;
							if( xbee_result.DATA[2] == 0x01 || xbee_result.DATA[2] == (byte)'1' ) i=1;
							if( xbee_result.DATA[2] == 0x02 || xbee_result.DATA[2] == (byte)'2' ) i=2;
							if(i>0){
								if(data <= 11){
									sprintf(d,"DefU");
									for(j=0;j<8;j++){
										send_data[i-1][j]=xbee_result.DATA[j+3];
										EEPROM.write(i*24-8+j,xbee_result.DATA[j+3]);
										if( xbee_result.DATA[j+3] == '\0' ) break;
									}
								}
							}
							break;
						case (byte)'M':	// [ESC]+[M]+[メニュー番号]+[メニュー名称(10文字まで)]
							i=0;
							if( xbee_result.DATA[2] == 0x01 || xbee_result.DATA[2] == (byte)'1' ) i=1;
							if( xbee_result.DATA[2] == 0x02 || xbee_result.DATA[2] == (byte)'2' ) i=2;
							if(i>0){
								if(data <= 13){
									sprintf(d,"DefM");
									for(j=0;j<10;j++){
										menu_s[6+i][j]=xbee_result.DATA[j+3];	// menu[7-8]処理
										EEPROM.write(i*24+j,xbee_result.DATA[j+3]);
										if( xbee_result.DATA[j+3] == '\0' ) break;
									}
								}
							}
							break;
						case (byte)'T':	// [ESC]+[T]+[hh:mm]
							if( xbee_result.DATA[2] >= '0' && xbee_result.DATA[2] <= '9' &&
								xbee_result.DATA[3] >= '0' && xbee_result.DATA[3] <= '9' &&
								xbee_result.DATA[4] == ':' &&
								xbee_result.DATA[5] >= '0' && xbee_result.DATA[5] <= '9' &&
								xbee_result.DATA[6] >= '0' && xbee_result.DATA[6] <= '9'){
								sprintf(d,"DefT");
								setTime(	(int)(	 (xbee_result.DATA[2]-'0')*10
													+(xbee_result.DATA[3]-'0')		),
											(int)(	 (xbee_result.DATA[5]-'0')*10
													+(xbee_result.DATA[6]-'0')		),0,NULL,NULL,NULL);
								/*
								tmElements_t tm;
								tm.Hour  = (int)(	 (xbee_result.DATA[2]-'0')*10
													+(xbee_result.DATA[3]-'0')		);
								tm.Minute= (int)(	 (xbee_result.DATA[5]-'0')*10
													+(xbee_result.DATA[6]-'0')		);
								time_t timer = makeTime(tm);
								setTime(timer);
								*/
							}
							break;
						default:
							sprintf(d,"ESC ");
							lcd.setCursor(12,0);
							lcd.print( d );
							lcd_cls(1);
							lcd.print("ESC(0x");
							lcd_print_hex(xbee_result.DATA[1]);
							lcd.print(")=0x");
							lcd_print_hex(xbee_result.DATA[2] );
							break;
					}
				}else{
					for(i=0;i<=16;i++){
						if( i < data ){
							if(	isprint((char)xbee_result.DATA[i]) ||
									(	xbee_result.DATA[i] >= 0xA1 && 
										xbee_result.DATA[i] <= 0xFC	)	 )
							{	// 表示可能文字の時(A1～FCはカタカナ対応)
								s[i] = (char)xbee_result.DATA[i];
							}else s[i]=' ';
						}else s[i]=' ';
					}
					s[16]='\0';
					lcd_cls(1);                             // 文字表示行の消去
					lcd.print( s );
					if( data > 16){
						for(i=0;i<16;i++)buf[i]=s[i];
						for(i=16;i<data;i++){
							xbee_delay(500);
							for(j=0;j<15;j++)buf[j]=buf[j+1];
							if( isprint((char)xbee_result.DATA[i]) ){
								buf[15] = (char)xbee_result.DATA[i];
							}else buf[15]=' ';
							buf[16]='\0';
							lcd.setCursor(0,1);
							lcd.print( buf );
						}
					}
				}
				counter = 0;
				break;
			case MODE_GPIN:		// GPIOからのデータを受信
				sprintf(d,"GPIN");
				lcd.setCursor(12,0);
				lcd.print( d );
				lcd_print_result(&xbee_result);
				counter = 0;
				break;
			case MODE_RESP:		// リモートATコマンドの応答を受信
				sprintf(d,"RESP");
				lcd.setCursor(12,0);
				lcd.print( d );
				lcd_print_result(&xbee_result);
				counter = 0;
				break;
			case MODE_IDNT:		// 新しいデバイスを発見。dev_gpinへ登録
				sprintf(d,"IDNT");
				lcd.setCursor(12,0);
				lcd.print( d );
				lcd_cls(1);
				if(dev_gpin_en==0){
					for(i=0;i<8;i++) dev_gpin[i]=xbee_result.FROM[i];
					lcd_print_address("Child  =",dev_gpin);
					dev_gpin_en=1;
					buzzer=2;
				}else{
					lcd_print_address("Error  =",&(xbee_result.FROM[0]));
														// 孫機登録済みなのでエラー
					buzzer=4;
				}
				counter = 0;
				tone(PIN_BUZZER,NOTE_A6,100);
				break;
			default:
				lcd_cls(1); 	   lcd.print("MODE=");lcd_print_hex(xbee_result.MODE);
				lcd.setCursor(8,1);lcd.print("STAT=");lcd_print_hex(xbee_result.STATUS);
				counter = 0;
				tone(PIN_BUZZER,NOTE_A6,100);
				break;
		}
	}
	if(buzzer){
		tone(PIN_BUZZER,NOTE_A7,100);					// ブザーに音階A7を出力
		xbee_delay(200);
		buzzer--;
	}
}
