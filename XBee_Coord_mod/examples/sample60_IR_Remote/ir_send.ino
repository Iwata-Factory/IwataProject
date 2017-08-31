/*********************************************************************

赤外線リモコン送信部 for Arduino

本ソースリストおよびソフトウェアは、ライセンスフリーです。
個人での利用は自由に行えます。著作権表示の改変は禁止します。

                               Copyright (c) 2012-2014 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/
/*
赤外線リモコン信号を送信します。

制約事項
		リモコン信号は8ビット単位で扱っています。
		SIRC方式などで8ビット単位にならない場合は送信できません。

参考文献

本ソフトの開発のために下記の文献を参考にしました。

・ボクにもわかる地上デジタル 地デジ方式編 - 赤外線リモコン方式
	http://www.geocities.jp/bokunimowakaru/std-commander.html
・MICROCHIP PIC12F683 Data Sheet (DS41211D) 2007
	Microchip Technology Inc.
・C言語ではじめるPICマイコン（オーム社）
	中尾真治
・プリント基板で作るPIC応用装置（ラトルズ）
	鈴木哲哉
・Arduinoをはじめよう (オライリージャパン)
	Massimo Banzi著 船田功訳
・Arduino 日本語リファレンス
	http://www.musashinodenpa.com/arduino/ref/index.php
*/

#define IR_OUT		12				// 赤外線LEDの接続ポート
#define IR_OUT_OFF	LOW				// 赤外線LED非発光時の出力値
#define IR_OUT_ON	HIGH			// 赤外線LED発光時の出力値
#define DATA_SIZE	16				// データ長(byte),4の倍数、16以上

#define FLASH_AEHA_TIMES	16	// シンボルの搬送波点滅回数（ＡＥＨＡ）
#define FLASH_NEC_TIMES		22	// シンボルの搬送波点滅回数（ＮＥＣ）
#define FLASH_SIRC_TIMES	24	// シンボルの搬送波点滅回数（ＳＩＲＣ）

// enum IR_TYPE{ AEHA=0, NEC=1, SIRC=2 };		// 家製協AEHA、NEC、SONY SIRC切り換え
#define AEHA		0
#define NEC			1
#define SIRC		2

void ir_init(void){
	pinMode(IR_OUT, OUTPUT);
	digitalWrite(IR_OUT, IR_OUT_OFF);
}

/* 赤外線ＬＥＤ点滅用 */
void ir_flash(byte times){
	while(times){
		times--;
		delayMicroseconds(12);				// 13 uS
		digitalWrite(IR_OUT, IR_OUT_ON);
		delayMicroseconds(7);				// 13 uS
		digitalWrite(IR_OUT, IR_OUT_OFF);
	}
}
void ir_wait(byte times){
	while(times){
		times--;
		delayMicroseconds(12);				// 13 uS
		digitalWrite(IR_OUT, IR_OUT_OFF);
		delayMicroseconds(7);				// 13 uS
		digitalWrite(IR_OUT, IR_OUT_OFF);
	}
}

/* 赤外線ＬＥＤ信号送出 */
void ir_send(byte *data, const byte data_len, const byte ir_type ){
	byte i,j,t;
	byte b;
	byte out;
	
	switch( ir_type ){
		case NEC:
			ir_flash( 8 * FLASH_NEC_TIMES );	// send 'SYNC_H'
			ir_flash( 8 * FLASH_NEC_TIMES );
			ir_wait(  8 * FLASH_NEC_TIMES );	// send 'SYNC_L'
			for( i = 0 ; i < data_len ; i++){
				for( b = 0 ; b < 8 ; b++ ){
					ir_flash( FLASH_NEC_TIMES );
					if( data[i] & (0x01 << b) ){
						ir_wait( 3 * FLASH_NEC_TIMES );
					}else{
						ir_wait( FLASH_NEC_TIMES );
					}
				}
			}
			ir_flash( FLASH_NEC_TIMES );		// send 'stop'
			break;
		case SIRC:
			for(j=0;j<3;j++){
				t=5;						// 送信済シンボル基本単位(SYNCで送信)
				ir_flash( 4 * FLASH_SIRC_TIMES );	// send 'SYNC_H'
				ir_wait(      FLASH_SIRC_TIMES );	// send 'SYNC_L'

				for( i = 0 ; i < data_len ; i++){
					for( b = 0 ; b < 8 ; b++ ){
						if( data[i] & (0x01 << b) ){
							ir_flash( 2 * FLASH_SIRC_TIMES );
							t +=3 ;
						}else{
							ir_flash( FLASH_SIRC_TIMES );
							t +=2 ;
						}
						ir_wait( FLASH_SIRC_TIMES );
					}
				}
				while( t <= 75 ){
					t++;
					ir_wait( FLASH_SIRC_TIMES );
				}
			}
			break;
		case AEHA:
		default:
			ir_flash( 8 * FLASH_AEHA_TIMES );	// send 'SYNC_H'
			ir_wait(  4 * FLASH_AEHA_TIMES);	// send 'SYNC_L'
			for( i = 0 ; i < data_len ; i++){
				for( b = 0 ; b < 8 ; b++ ){
					ir_flash( FLASH_AEHA_TIMES );
					if( data[i] & (0x01 << b) ){
						ir_wait( 3 * FLASH_AEHA_TIMES );
					}else{
						ir_wait( FLASH_AEHA_TIMES );
					}
				}
			}
			ir_flash( FLASH_AEHA_TIMES );		// send 'stop'
			break;
	}
}
