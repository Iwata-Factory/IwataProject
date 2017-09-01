/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続の小型液晶に文字を表示する
Arduino標準ライブラリ「Wire」は使用していない(I2Cの手順の学習用サンプル)

                             Copyright (c) 2014 Wataru KUNINO & S.Noda
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************//*
取得した情報をファイルに保存するロガーの製作
秋月超小型I2C液晶版（S.Noda）
*/
#include <xbee.h>
#include <SD.h>

#define PIN_SDCS		4                           // SDのCSポート番号
#define TIME_HOUR		18							// ログを開始する時刻(時)
#define TIME_MIN		0							// ログを開始する時刻(分)
#define PAIRING			0							// 起動時にペアリングを行う
#define FORCE_TRIG		1							// 子機XBeeへ取得指示を送信
#define FORCE_INTERVAL	10							// 送信間隔(5～59)[秒]


File file;                                          // SDファイルの定義
unsigned long time_prev;
int value[3];
char text[]="ｻﾝﾌﾟﾙ 61XBLogger";
byte trig=0;
byte address[8];

byte setupRemoteXBee(){
}

void timeToString(char *timeS,unsigned long time){
	timeS[0]= '0' + (char)(((time/3600ul)%24ul)/10);
	timeS[1]= '0' + (char)(((time/3600ul)%24ul)%10);
	timeS[2]= ':';
	timeS[3]= '0' + (char)(((time/60ul)%60ul)/10);
	timeS[4]= '0' + (char)(((time/60ul)%60ul)%10);
	timeS[5]= ':';
	timeS[6]= '0' + (char)((time%60ul)/10);
	timeS[7]= '0' + (char)((time%60ul)%10);
}

void setup() {
	i2c_lcd_init();
    i2c_lcd_print(text);                            // 文字を表示
    while(SD.begin(PIN_SDCS)==false){               // SDカードの開始
        delay(5000);                                // 失敗時は5秒毎に繰り返し
    }
    xbee_init( 0 );                                 // XBee用COMポートの初期化

    if(PAIRING){
        i2c_lcd_print("ﾍﾟｱﾘﾝｸﾞ ﾁｭｳ");               // 表示
        xbee_atnj(30);                              // デバイスの参加受け入れを
        xbee_from(address);							// 変数addressにアドレス代入
    }else{
		xbee_atnj(0);                               // ジョイン非許可
	}
    for(byte i=0;i<3;i++)value[i]=-1;				// 測定結果の初期化
}

void loop() {
    byte i,data;
    char s[17];
    
    XBEE_RESULT xbee_result;                        // 受信データ(詳細)
    char  filename[]="01234567.CSV";                // 書き込みファイル名
    unsigned long time=millis()/1000ul;             // 経過時間の取得(約50日迄)
    time += TIME_HOUR * 3600ul;						// 時刻に変換(時)
    time += TIME_MIN * 60ul;						// 時刻に変換(分)

	if(time != time_prev){							// 前回の時刻から変化時
		time_prev=time;								// 現在時刻を前回時刻に代入
		i=time%4;									// 時刻を4で割った余りを代入
		if(i<3){
			timeToString(s,time);					// 時刻を文字列に変換
			i2c_lcd_disp_s(s,0);					// 時刻を液晶の上段に表示
			sprintf(s,"ch%d:",i+1);					// DIOポート番号の文字列作成
			i2c_lcd_disp_i(s,value[i],1);			// 表示
		}else{
			i2c_lcd_print(text);					// 文字列の表示
		}
		if(FORCE_TRIG && trig && time%FORCE_INTERVAL==0){
			xbee_force(address);
		}
	}

    data=xbee_rx_call(&xbee_result);              	// データを受信
    if(xbee_result.MODE==0x00) return;          	// データが無い時は終了

    for(i=0;i<4;i++){                               // 送信元をファイル名に
        filename[i*2]  =(char)(((xbee_result.FROM[i+4])>>4)+'0');
        filename[i*2+1]=(char)(((xbee_result.FROM[i+4])%16)+'0');
    }
    for(i=0;i<8;i++){
        if(filename[i]>'9') filename[i] += (char)('A'-'0'-10);
    }
    strcpy(s,filename);								// ファイル名をコピー
    i2c_lcd_disp_s(s,1);							// 液晶の下段に表示
    file=SD.open(filename,FILE_WRITE);              // 書込みファイルのオープン
    file.print(time);                               // 時刻の出力
    if(file == true){                               // オープンが成功した場合
        switch( xbee_result.MODE ){
            case MODE_GPIN:                         // IOデータを受信した場合
            case MODE_RESP:
                for(i=1;i<=3;i++){                  // 入力値をSDへ保存、表示
                    file.print(", ");
                    if(xbee_result.ADCIN[i]==0xFFFF){
                        file.print((xbee_result.GPI.BYTE[0]>>i)&0x01,BIN);
                        value[i-1]=(int)((xbee_result.GPI.BYTE[0]>>i)&0x01);
                    }else{
                        file.print(xbee_result.ADCIN[i],DEC);
                        value[i-1]=(int)xbee_result.ADCIN[i];
                    }
                }
                break;
            case MODE_UART:                         // UARTシリアルデータを受信した場合
                for(i=0;i<=16;i++) s[i]='\0';       // 文字列変数sの初期化
                if(data > 16) data=16;              // 文字の扱いは16文字まで
                for(i=0;i<data;i++){                // 受信データを文字列変数sへ代入
                    if(isprint((char)xbee_result.DATA[i])){
                        s[i] = (char)xbee_result.DATA[i];
                    }else s[i] = ' ';
                }
                file.print(s);                      // 受信した文字列をSDへ保存
                strcpy(text,s);
                break;
            case MODE_IDNT:                         // コミッショニングボタン通知の時
            //  xbee_end_device(xbee_result.FROM,10,10,0);  // 自動測定10秒,S端子無効
                i2c_lcd_disp_s("Dev Idnt",0);
                for(i=0;i<8;i++)address[i]=xbee_result.FROM[i];
                setupRemoteXBee();
                trig=1;
                delay(3000);
                break;
        }
        file.println("");							// SDへ改行を保存
        file.close();                               // ファイルクローズ
    }else{
        i2c_lcd_disp_s("File ERR",0);
        delay(5000);
    }
}
