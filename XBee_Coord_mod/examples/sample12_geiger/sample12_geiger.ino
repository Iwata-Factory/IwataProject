/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

なお、Radiation-Watch.orgのソースコードを含みます。

                               Copyright (c) 2012-2013 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/

//////////////////////////////////////////////////
// Radiation-Watch.org
// URL http://www.radiation-watch.org/
//////////////////////////////////////////////////
/*

Radiation-watch.orgプロジェクトは開発成果をクリエイティブ・コモンズ(CC 
BY 3.0)ライセンスの元でオープンソース化しています。以下では、ポケット
ガイガーの回路図、仕様、ソースコード、実験結果を、順次公開します。
これらは全世界において、商用・非商用を問わず自由に再配布・改変・公開す
ることができます。なお一部のコンテンツは英語またはオランダ語のみとなり
ます。

POCKET GEIGER by Radiation-Watch.org is licensed under a Creative 
Commons Attribution 3.0 Unported License.

*/

/*
使い方：
	(1) ライブラリのダウンロード
	　　当方のサイトから「PC/H8/Arduino用 管理lib. ZB Coordinator API 」を
	　　ダウンロードします。ファイル名＝xbeeCoord_XXX.zip (XXXは数字)
	　　　　http://www.geocities.jp/bokunimowakaru/diy/xbee/xbee-download.html
	(2) ライブラリのインストール
	　　ZIPファイルの中の「xbee_arduino」フォルダ内の「XBee」ファルダを	
	　　 arduinoをインストールしたフォルダ(例 C:\arduino)の中の「libraries」
	　　 フォルダにインポートします。
	　　　　C:\arduino\libraries\XBee\xbee.h
	　　　　C:\arduino\libraries\XBee\xbee.cpp
	　　　　C:\arduino\libraries\XBee\examples\...
	　　Adafuruit LCDをお持ちの方は、そちらのライブラリもインストールします。
	　　　　C:\arduino\libraries\Adafruit_RGBLCDShield
	(3) Arduino IDE を起動します。(既に起動していた場合、一度、終了して再起動)
	(4) ソースにXBeeモジュールのIEEEアドレスを追加
	　　本ソースの「byte dev_gpio」の宣言部分に記載しているZigBee IEEEアドレス
	　　を、お手持ちの子機のアドレス(XBeeモジュールの裏面に記載)に変更します。
	(5) 液晶の種類の設定
	　　Adafuruit LCD Sheildを使用しない場合は本ソースの「#define ADAFURUIT」を
	　　削除します。ライブラリ内の「xbee.cpp」内のdefineも外せば、ライブラリの
	　　動作も液晶に表示できるようになります。
	(6) Arduinoマイコンボードに書き込みます。ATmega 328で動作確認しています。
*/
/*
液晶表示に関する注意点

	Adafuruit LCD Sheildを使用した場合：
	そのまま使用してください。液晶にリモート先のスイッチ状態が
	表示されます。

	Adafuruit LCD Sheildをお持ちでない場合：
	コンパイル前に#define ADAFURUITを消してください。
	通常のキャラクタ液晶へ出力するようになります。
*/
/*
ハードウェア(XBee子機)

	port:	port指定	IO名	ピン番号	
			port= 0		DIO0	XBee_pin 20		SW1 Commision
			port= 1		DIO1	XBee_pin 19		ポケットガイガー信号
			port= 2		DIO2	XBee_pin 18		ポケットガイガーノイズ
*/

/*********************************************************************
インポート処理						※ライブラリ XBeeに必要な処理です
*********************************************************************/

#include <xbee.h>

/*********************************************************************
アプリケーション					※以下を編集してアプリを作成します
*********************************************************************/

#define ADAFURUIT	// 使用する液晶が Adafuruit LCD Sheild の時に定義する

#ifndef ADAFURUIT
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

//　↓お手持ちのXBeeモジュール子機のIEEEアドレスに変更する
byte dev_gpio[]   = {0x00,0x13,0xA2,0x00,0x40,0x30,0xC1,0x6F};

const float alpha=53.032; // cpm = uSv × alpha

// char msg[80]=""; //シリアルで出力する文字列用

int signCount=0;  //信号のカウント回数
int noiseCount=0;  //ノイズのカウント回数

float cpm = 0; //現在のカウント数
float cpmHistory[200]; //カウント回数の履歴
byte cpmIndex=0;//カウント回数の現在配列の格納位置
int  cpmIndexPrev=0;//二重カウントの防止用

//ループの時間間隔の計測用
int prevTime=0;
int currTime=0; 

int totalSec=0; //計測開始からのトータルの時間(秒)
int totalHour=0; //計測開始からのトータルの時間(時間)

//cpm計算用の時間
int cpmTimeMSec=0;
int cpmTimeSec=0;
int cpmTimeMin=0;

//シリアルで出力するときの浮動少数を文字列に変換用
char cpmBuff[20];
char uSvBuff[20];
char uSvdBuff[20];

void setup(){
	lcd.begin(16, 2);
	lcd.clear(); lcd.print("ZigBee Geiger");
	xbee_init( 0x00 );					// XBee用COMポートの初期化(引数はポート番号)
	lcd.clear(); lcd.print("ATNJ=10");
	xbee_atnj( 0x10 );					// デバイスを30秒間、参加受け入れる
	lcd.clear(); lcd.print("gpio init");
	xbee_gpio_init(dev_gpio);			// デバイスdev_gpioにIO設定を行うための送信
	lcd.clear(); lcd.print("DONE");

	//cpmの履歴を保存する配列の初期化
	for(int i=0; i<200;i++ ){
		cpmHistory[i]=0;
	}
}

void loop(){
	byte i;
	int index; //ループ回数

	XBEE_RESULT xbee_result;			// 受信データ用の構造体

	//ループ開始時間を取得
	prevTime = millis();
	while(1){
		for(index=0; index<88 ; index++){
			xbee_result.MODE = MODE_AUTO;			// 自動受信モードに設定
			xbee_rx_call( &xbee_result );
			if( xbee_result.MODE == MODE_GPIN){		// PIO入力(リモート機のボタンが押された)の時
				if( (xbee_result.GPI.BYTE[0] & 0x02 ) == 0x00 ) signCount++;		//放射線パルス(Low)をカウント
				if( (xbee_result.GPI.BYTE[0] & 0x04 ) == 0x04 ) noiseCount++;	//ノイズ(High)をカウント
			//	lcd.setCursor(8,1);	lcd.print(GPI[0],BIN);
			}else if( xbee_result.MODE == MODE_IDNT){	// 新しいデバイスを発見
				for(i=0;i<8;i++ ) dev_gpio[i]=xbee_result.FROM[i];	// 発見したアドレスをdev_gpioに読み込み
				xbee_gpio_init( dev_gpio );
				lcd.clear(); lcd.print("found a new dev");
			}
		}
		currTime = millis();    //現在の時刻を取得
		if(noiseCount == 0){		//ノイズが検出されないとき（ノイズが検出されたときは、処理をしない）
			if( totalSec % 6 == 0 && cpmIndexPrev != totalSec){			//6秒ごとにカウント回数の履歴を格納する配列をずらす
				cpmIndexPrev = totalSec;
				cpmIndex++;
				if(cpmIndex >= 200){        //最後(20分後)まできたら元に戻す
					cpmIndex = 0;
				}
				//一周してきたときに次に格納する配列に値が詰まっていれば、
				//現在の合計しているカウント(変数cpm)から引いて、無かったことにする
				if(cpmHistory[cpmIndex] > 0){
					cpm -= cpmHistory[cpmIndex];
				}
				cpmHistory[cpmIndex]=0;
			}
			cpmHistory[cpmIndex] += signCount;      //カウント回数の履歴を保存
			cpm += signCount;      //カウント回数の蓄積
			cpmTimeMSec += abs(currTime - prevTime);      //データを取得するのにかかった時間を取得
			if(cpmTimeMSec >= 1000){      //ms→secに変換する(オーバーフロー対策)
				cpmTimeMSec -= 1000;
				if( cpmTimeSec >= 20*60 ){        //cpmを求めるときに使用する計測時間(最大20分)を加算
					cpmTimeSec = 20*60;
				}else{
					cpmTimeSec++;
				}
				totalSec++;        //トータルでの計測時間
				if(totalSec >= 3600){        //sec→hourに変換する(オーバーフロー対策)
					totalSec -= 3600;
					totalHour++;
				}
			}
			float min = cpmTimeSec / 60.0;      //現在の計測時間(最大20分)
			if(min!=0){        //cpm、uSv/h、uSv/hの誤差をそれぞれ計算する
				dtostrf(cpm / min, -1, 2, cpmBuff);
				dtostrf(cpm / min / alpha, -1, 2, uSvBuff);
				dtostrf(sqrt(cpm) / min / alpha, -1, 2, uSvdBuff);
			}else{        //0割り算のときは、0にする
				cpmBuff[0]='0'; cpmBuff[1]='\0'; 
				uSvBuff[0]='0'; uSvBuff[1]='\0'; 
				uSvdBuff[0]='0'; uSvdBuff[1]='\0'; 
			}
			//液晶への表示
			char time_s[6] = "00:00";
			time_s[0] = (char)(totalSec/600)+'0';
			time_s[1] = (char)((totalSec/60)%10)+'0';
			time_s[3] = (char)((totalSec%60)/10)+'0';
			time_s[4] = (char)((totalSec%60)%10)+'0';
			lcd.setCursor(0,0);
			lcd.print( cpmBuff );
			lcd.print(" cpm   ");
			lcd.setCursor(11,0);
			lcd.print( time_s );
			lcd.print(" ");
			lcd.setCursor(0,1);
			lcd.print( uSvBuff );
			lcd.print(" uSv/h      ");
			/*
			//シリアルで送信する文字列の生成
			sprintf(msg, "%d,%d.%03d,%d,%s,%s,%s",
				totalHour,totalSec,
				cpmTimeMSec,
				signCount,
				cpmBuff,
				uSvBuff,
				uSvdBuff
			);
			*/
		}
		//次の100回のループのための変数の初期化
		prevTime = currTime;
		signCount=0;
		noiseCount=0;
	}
}
