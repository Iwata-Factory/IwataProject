/***************************************************************************************
サンプルアプリ60 XBee子機 リモート赤外線リモコン for Arduino

本ソースリストおよびソフトウェアは、ライセンスフリーです。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

赤外線リモコン受信モジュール	Digilal 2	コールバック：ardIsrIr
XBee 13 (On/Sleep)				Digital 3	コールバック：ardIsrXbee
赤外線リモコン送信LED			Digital 12
XBee 9 (Sleep_RQ)				Digital 4

XBee ファームウェアは「ZIGBEE END DEVICE API」を使用します。

別途、親機が必要です。
親機をPC用Cygwnで作成する場合はCygwin用サンプルsample60_ir.cを使用します。

既知の不具合：
01:起動直後(初回スリープに入って5秒間)は赤外線リモコン受信の結果を親機に送信しない。

                                                 Copyright (c) 2014 Wataru KUNINO
                                                 http://www.geocities.jp/bokunimowakaru/
***************************************************************************************/

#include <xbee.h>
#include <avr/sleep.h>
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define DATA_LEN	16+5					// リモコンコードのデータ長(byte)+5
#define STRING_LEN	8						// 表示用の文字列最大値
#define LED_PIN		13						// 動作確認用LEDのDigitalピン番号
#define SLEEP_PIN	4						// XBeeのスリープ解除信号
#define XBEE_WAKE	16						// XBee起動後の待ち時間(受信繰り返し回数)

// enum IR_TYPE{ AEHA=0, NEC=1, SIRC=2 };	// 家製協AEHA、NEC、SONY SIRC切り換え
#define AEHA		0
#define NEC 		1
#define SIRC		2
byte type = AEHA;								// 赤外線方式
byte irdata[14] ={0xAA,0x5A,0xCF,0x10,0x07,0x21,0x23,0x15,0x18,0xA0,0x00,0xF4,0x51,0x66};

/* IEEEアドレス */
byte dev_coord[8] ={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

/* コールバック状態 */
volatile byte ardIsrSrc=1;

void xbee_on(){									// XBee スリープ解除
	pinMode(SLEEP_PIN, OUTPUT); 
	digitalWrite(SLEEP_PIN,0);
	digitalWrite(LED_PIN,1);
}
void xbee_off(){
	pinMode(SLEEP_PIN, INPUT);					// XBee スリープ実行 ※XBee 3.3V入力考慮
	xbee_delay(200);							// OFF待ち 100ms以上が必要
	digitalWrite(LED_PIN,0);
}

void gpio_init(){								// ArduinoのIO初期設定
	pinMode(2, INPUT_PULLUP);					// 赤外線リモコン受信モジュール割込み
	pinMode(3, INPUT);							// XBee 割込み
	pinMode(LED_PIN,OUTPUT);
	digitalWrite(LED_PIN,1);
	xbee_off(); delay(500); xbee_on();
}

void led_flash(byte i){							// LEDの高速点滅
	for(;i>0;i--){
		if(i<100) digitalWrite(LED_PIN,i%2);
		else digitalWrite(LED_PIN,i%6);
		delay(30);
	}
}

void setup() {
	byte i,j=0;
	gpio_init();
	ir_init();
	ir_read_init();
	xbee_init( 0x00 );
	while( xbee_ping(dev_coord) == 0xFF ){		// ping応答を確認
		while( xbee_atai() > 0x01 ){			// ネットワーク参加状況を確認
			led_flash(60); xbee_atcb(1); j++;	// ネットワーク参加ボタンを押下
			if(j>10){
				j=0; xbee_atcb(4); led_flash(200);	// ネットワーク初期化
			}
		}
	}
	xbee_delay(1000);
	xbee_atcb(1);
	digitalWrite(LED_PIN,1);
	if( xbee_atvr() == ZB_TYPE_ENDDEV){
		xbee_at("ATST00FA");					// ST = 250 ms
		xbee_at("ATSP01F4");					// SP = 5 sec
		xbee_at("ATSN24");						// SN = 36回 180秒(3分)毎に起動
		xbee_at("ATSM05");						// スリープモードへ
		xbee_at("ATWH14");						// WH = 20 ms
		xbee_at("ATAC");						// 設定実行
		xbee_at("ATSI");						// スリープ実行
		xbee_delay(3000);
	}else{
		xbee_uart(dev_coord,"Please Set me to End Dev.");
		while(1);								// End device以外の場合は動かさない
	}
	xbee_off();									// XBeeをOFF
}

void loop(){
	byte i,len8;
	int len=0;									// 信号長
	byte data[DATA_LEN];
	char s[STRING_LEN];
	char c = 0;
	XBEE_RESULT xbee_result;

	/* 赤外線リモコンコードの読み取り */
	attachInterrupt(0, ardIsrIr,   CHANGE);		// Digital 2を割込み入力に設定
	attachInterrupt(1, ardIsrXbee, CHANGE);		// Digital 3を割込み入力に設定
	interrupts();
	system_sleep();								// Arduino パワーダウン
	if(ardIsrSrc==0){
		len= ir_read(&data[5],DATA_LEN-5,type);	// ★起動直後に実行★IRデータを読み込む
		if(len>0){
			len8 = (byte)(len/8);
			if(len%8 != 0) len8++;
			/* 結果送信 */
			xbee_on();
			xbee_delay(10);						// XBee ON待ち 10ms程度
			data[0]=0x1B; data[1]=(byte)'I'; data[2]=(byte)'R';
			data[3]=type; data[4]=len8;
			xbee_bytes(dev_coord,data,len8+5);	// XBee 送信
			xbee_delay(150);					// 送信完了待ち 100ms程度
			ardIsrSrc=1;
		}
	}
	/* XBee受信処理 */
	xbee_on();									// ON時間の延長
	do for(i = 0; i < XBEE_WAKE; i++){
		len = xbee_rx_call( &xbee_result );		// XBeeデータを読み込む
		if(	xbee_result.MODE == MODE_UART &&	// シリアルデータを受信
			xbee_result.DATA[0] == 0x1B &&		// ESCコードが先頭だったとき
			xbee_result.DATA[1] == (byte)'I' &&
			xbee_result.DATA[2] == (byte)'R'	// コマンドが「IR」の時
		){
			type = xbee_result.DATA[3];
			len8 = xbee_result.DATA[4];
			if(len8+5 <= len){
			//	xbee_uart(dev_coord,"OK");		// ★受信直後に実行★
				if(len8 > 0) ir_send( &xbee_result.DATA[5] , len8 , type );
			}else xbee_uart(dev_coord,"ERROR");
			xbee_delay(100);					// 送信完了待ち 100ms程度
			break;								// for i;
		}
	}while(xbee_result.MODE);
	xbee_off();									// XBeeをOFF
}

void ardIsrIr(){
	noInterrupts();
	ardIsrSrc=0;
}

void ardIsrXbee(){
	noInterrupts();
	ardIsrSrc=1;
}

void system_sleep() {					// システム停止 
	// set system into the sleep state 
	// system wakes up when wtchdog is timed out
	cbi(ADCSRA,ADEN);					// ADC 回路電源をOFF (ADC使って無くても120μA消費するため）
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);// パワーダウンモードをSLEEP_MODE_PWR_DOWNに設定(sleep mode is set here)
	sleep_enable();
	sleep_mode();						// ここでスリープに入る (System sleeps here)
	sleep_disable();					// WDTがタイムアップしたらここから動作再開 
	sbi(ADCSRA,ADEN);					// ADC ON
}

/* データ例
 ESC  'I'  'R'	type len  data....
{0x1B,0x49,0x52,0x00,0x06,0xAA,0x5A,0x8F,0x12,0x15,0xE1}	// AQUOS 音量を下げる
{0x1B,0x49,0x52,0x00,0x05,0x2C,0x52,0x09,0x2B,0x22};		// 照明の輝度を下げる
*/
