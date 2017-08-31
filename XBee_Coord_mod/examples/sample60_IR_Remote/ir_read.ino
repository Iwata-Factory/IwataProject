/*********************************************************************

赤外線リモコン受信部 for Arduino

本ソースリストおよびソフトウェアは、ライセンスフリーです。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

                               Copyright (c) 2009-2014 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/
/*参考文献

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

#define IR_IN		2				// 赤外線センサの接続ポート(Digital 2)
#define IR_IN_OFF	1				// 赤外線センサ非受光時の入力値
#define IR_IN_ON	0				// 赤外線センサ受光時の入力値
#define SYNC_WAIT	2*16*470		// 待ち時間[us] (15ms) ※intの範囲

//enum IR_TYPE{AEHA=0,NEC=1,SIRC=2};// 家製協AEHA、NEC、SONY SIRC切り換え
#define AEHA		0
#define NEC			1
#define SIRC		2

void ir_read_init(void){
	pinMode(IR_IN, INPUT);
}

/* シンボル読取り*/
int ir_sens(byte det){
	unsigned long counter;
	byte in,det_wait,det_count;
	
	if( det == IR_IN_OFF ){		/* AEHA, NEC */
		det_wait = IR_IN_ON;		// 消灯待ち
		det_count= IR_IN_OFF;		// 消灯カウント
	}else{						/* SIRC */
		det_wait = IR_IN_OFF;		// 点灯待ち
		det_count= IR_IN_ON;		// 点灯カウント
	}
	/* 待ち */
	counter = micros()+SYNC_WAIT;
	do in = digitalRead(IR_IN);
	while( micros()<counter && (in == det_wait) );
	/* カウント */
	counter = micros()+SYNC_WAIT;
	if(in == det_wait) return( -1 );	/* 待ちタイムアウト */
	do in = digitalRead(IR_IN);
	while( micros()<counter && (in == det_count));
	counter = micros() - counter + SYNC_WAIT;
	if( in == det_count ) return( -2 );		/* 変化せず */
	if( counter > SYNC_WAIT ) return( -3 );	/* 長すぎ */
	return( (int)counter );
}

/* 赤外線信号読み取りシンプル */
int ir_read(byte *data, const byte data_num, const byte mode){
	int i,bit;
	int data_len= -1;				// Irデータのビット数
	int len, data_wait;
	int	len_on=0,len_off=0;			// 信号長(ループカウント)
	int symbol_len, noise;			// 判定用シンボル長
	byte det = IR_IN_OFF;			// 判定時の入力信号レベル(SIRC対応)
	byte in;

	/* SYNC_ON検出 */
	len_on = ir_sens(IR_IN_ON);		// 受光待ち
	if( len_on < 0 ) return( -1 );			/* タイムアウト */
	/* SYNC_OFF検出 */
	len_off = ir_sens(IR_IN_OFF);
	if( len_off < 0 ) return( -2 );			/* エラー */
	/* モード設定*/
	symbol_len = len_off/2;
	switch( mode ){
		case NEC:					// H(16T) + L(8T)	2:1
			symbol_len = len_off/4;
			det=IR_IN_OFF;
			break;
		case SIRC: 					// H(4T) + L(1T)	4:1
			det=IR_IN_ON;
			symbol_len = (3*len_off)/2;
			break;
		case AEHA:					// H(8T) + L(4T)	2:1
		default:
			symbol_len = len_off/2;
			det=IR_IN_OFF;
			break;
		
	}
	
	/* データー読取り*/
	data_wait = 2 * symbol_len;		// 終了検出するシンボル長
	noise = symbol_len /4;			// ノイズと判定するシンボル長
	for(i=0;i<data_num;i++){
		in = 0;
		for(bit=0;bit<8;bit++){
			len = ir_sens( det );
			if( len > noise && len < data_wait){
				if( len < symbol_len ){
					in = in>>1;
					in += 0;
				}else{
					in = in>>1;
					in += 128;
				}
			}else{
				in = in>>(8 - bit);
				data[i]=in;
				data_len = i * 8 + bit;
				i = data_num -1;	// break for i
				bit=7;				// break for bit
			}
		}
		data[i]=in;
	}
	#ifdef DEBUG	//1234567890
		Serial.print("SYNC LEN= "); Serial.println(len_on+len_off,DEC);
		Serial.print("SYNC ON = "); Serial.println(len_on,DEC);
		Serial.print("SYNC OFF= "); Serial.println(len_off,DEC);
	#endif // DEBUG
	return(data_len);
}
