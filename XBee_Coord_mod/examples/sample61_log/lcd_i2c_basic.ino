/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続の小型液晶に文字を表示する
Arduino標準ライブラリ「Wire」は使用していない(I2Cの手順の学習用サンプル)

                             Copyright (c) 2014 Wataru KUNINO & S.Noda
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/

#define	PORT_SCL	19					// I2C SCLポート番号(ANALOG 5)
#define	PORT_SDA	18					// I2C SDAポート番号(ANALOG 4)
#define	PORT_ERR	13					// エラー時のパルス出力用ポート
#define	DEBUG		1					// デバッグモード
#define	I2C_RAMDA	20					// I2C データシンボル長[us]

void i2c_SCL(byte level){
	if( level ){
		pinMode(PORT_SCL, INPUT);
	}else{
		digitalWrite(PORT_SCL, LOW);
		pinMode(PORT_SCL, OUTPUT);
	}
	delayMicroseconds(I2C_RAMDA);
}

void i2c_SDA(byte level){
	if( level ){
		pinMode(PORT_SDA, INPUT);
	}else{
		digitalWrite(PORT_SDA, LOW);
		pinMode(PORT_SDA, OUTPUT);
	}
	delayMicroseconds(I2C_RAMDA);
}

byte i2c_tx(const byte in){
	byte i;
	for(i=0;i<8;i++){
		if( (in>>(7-i))&0x01 ){
				i2c_SDA(1);					// (SDA)	H Imp
		}else	i2c_SDA(0);					// (SDA)	L Out
		/*Clock*/
		i2c_SCL(1);							// (SCL)	H Imp
		i2c_SCL(0);							// (SCL)	L Out
	}
	/* ACK処理 */
	delayMicroseconds(I2C_RAMDA);
	i2c_SCL(1);								// (SCL)	H Imp
	i2c_SDA(1);								// (SDA)	H Imp
	for(i=50;i>0;i--){
		if( digitalRead(PORT_SDA) == 0 ) break;	// 速やかに確認
		delayMicroseconds(I2C_RAMDA);
	}
	if(i==0){
//		i2c_error("I2C_TX / no ACK");
		return(0);
	}
	return(i);
}

byte i2c_init(void){
	byte i;
	
	/* 液晶のセットアップ */
	for(i=50;i>0;i--){						// リトライ50回まで
		i2c_SDA(1);							// (SDA)	H Imp
		i2c_SCL(1);							// (SCL)	H Imp
		if( digitalRead(PORT_SCL)==1 &&
			digitalRead(PORT_SDA)==1  ) break;
		delay(1);
	}
	delayMicroseconds(I2C_RAMDA*8);
	return(i);
}

byte i2c_start(void){
	if(!i2c_init())return(0);				// SDA,SCL  H Out
	i2c_SDA(0);								// (SDA)	L Out
	delayMicroseconds(I2C_RAMDA);
	i2c_SCL(0);								// (SCL)	L Out
	return(1);
}

byte i2c_write(byte adr, byte *tx, byte len){
/*
入力：byte adr = I2Cアドレス ／SHT 温度測定時0x01	湿度測定時0x02
入力：byte *tx = 送信データ用ポインタ
入力：byte len = 送信データ長
*/
	byte i,ret;
	if(len){
		if( !i2c_start() ) return(0);
	}else return(0);
	adr &= 0xFE;							// RW=0 送信モード
	if( i2c_tx(adr)==0 ) return(0);			// アドレス設定

	/* データ送信 */
	for(ret=0;ret<len;ret++){
		i2c_SDA(0);							// (SDA)	L Out
		i2c_SCL(0);							// (SCL)	L Out
		i2c_tx(tx[ret]);
	}
	/* STOP */
	i2c_SDA(0);								// (SDA)	L Out
	i2c_SCL(0);								// (SCL)	L Out
	delayMicroseconds(I2C_RAMDA);
	i2c_SCL(1);								// (SCL)	H Imp
	i2c_SDA(1);								// (SDA)	H Imp
	return(ret);
}

void i2c_lcd_out(byte y,byte *lcd){
	byte data[2];
	byte i;
	data[0]=0x00;
	if(y==0) data[1]=0x80;
	else{
		data[1]=0xC0;
		y=1;
	}
	i2c_write(0x7C,data,2);
	for(i=0;i<8;i++){
		if(lcd[i]==0x00) break;
		data[0]=0x40;
		data[1]=lcd[i];
		i2c_write(0x7C,data,2);
	}
}

void utf_del_uni(char *s){
	byte i=0;
	byte j=0;
	while(s[i]!='\0'){
		if((byte)s[i]==0xEF){
			if((byte)s[i+1]==0xBE) s[i+2] += 0x40;
			i+=2;
		}
		s[j]=s[i];
		i++;
		j++;
	}
	s[j]='\0';
}

void i2c_lcd_print(char *s){
	byte i,j;
	byte lcd[9];
	
	utf_del_uni(s);
	lcd[8]='\0';
	for(j=0;j<2;j++){
		for(i=0;i<8;i++){
			lcd[i]=(byte)s[i+8*j];
			if(lcd[i]==0x00) for(;i<8;i++) lcd[i]=' ';
		}
		i2c_lcd_out(j,lcd);
	}
}

void i2c_lcd_disp_s(char *s,byte row){
	byte i;
	byte lcd[9];
	if(row>1) row=1;
	utf_del_uni(s);
	lcd[8]='\0';
	for(i=0;i<8;i++){
		lcd[i]=(byte)s[i];
		if(lcd[i]==0x00) for(;i<8;i++) lcd[i]=' ';
	}
	i2c_lcd_out(row,lcd);
}


void i2c_lcd_disp_i(char *s,int in,byte row){
	byte i,sign=0;
	byte lcd[9];
	if(row>1) row=1;
	
	utf_del_uni(s);
	for(i=0;i<8;i++){
		lcd[i]=(byte)s[i];
		if(lcd[i]==0x00) for(;i<8;i++) lcd[i]=' ';
	}
	lcd[8]='\0';
	if(in<0){
		sign=1;
		in *= -1;
	}
	for(i=0;i<5;i++){
		lcd[7-i] = (byte)(in%10)+(byte)'0';
		in /= 10;
		if(in==0) break;
	}
	if(sign)lcd[6-i] = '-';
	i2c_lcd_out(row,lcd);
}

byte i2c_lcd_init(void){
	byte data[2];
	
	/* 液晶のセットアップ */
	if( i2c_init() ){
		data[0]=0x00; data[1]=0x39; i2c_write(0x7C,data,2);	// IS=1
		data[0]=0x00; data[1]=0x11; i2c_write(0x7C,data,2);	// OSC
		data[0]=0x00; data[1]=0x70; i2c_write(0x7C,data,2);	// コントラスト	0
		data[0]=0x00; data[1]=0x56; i2c_write(0x7C,data,2);	// Power/Cont	6
		data[0]=0x00; data[1]=0x6C; i2c_write(0x7C,data,2);	// FollowerCtrl	C
		delay(200);
		data[0]=0x00; data[1]=0x38; i2c_write(0x7C,data,2);	// IS=0
		data[0]=0x00; data[1]=0x0C; i2c_write(0x7C,data,2);	// DisplayON	C
		return(1);
	}
	return(0);
}
