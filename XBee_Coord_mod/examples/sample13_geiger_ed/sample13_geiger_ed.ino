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

/*
設計事項
	デバイス：End Device (放射線パルスはトグル仕様)＋ポケガ電源FET SW
	動作間隔：60分間に1回
	測定時間：3分間
*/

#define XBEE_END_DEVICE		// XBeeにエンドデバイスを使用するときの設定
#define TIME_INTERVAL	60
#define TIME_MEAS		3	// 最大20分
#define DATA_NUM		TIME_MEAS * 60 / 6 + 4
		//	24秒分( 6秒 * 4 )の余分が必要(電源オフしてもすぐにオフにならないので)


//　↓お手持ちのXBeeモジュール子機のIEEEアドレスに変更する
byte port=0;		// ↓お手持ちのXBeeモジュールのIEEEアドレスに変更する
byte dev_sens[]   = {0x00,0x13,0xA2,0x00,0x00,0x00,0x00,0x00};	// PocketGeiger

int signCount=0;	//信号のカウント回数
int noiseCount=0;	//ノイズのカウント回数
byte signValue=0;	//前回のsign値 0 or 1

float cpm = 0; //現在のカウント数
float cpmHistory[DATA_NUM]; //カウント回数の履歴
byte cpmIndex=0;//カウント回数の現在配列の格納位置

//cpm計算用の時間
int cpmTimeSec=0;

//出力演算用
float cpmBuff;
float uSvBuff;
float uSvdBuff;

float min;
	
const float alpha=53.032; // cpm = uSv × alpha

void setup(){
	lcd.begin(16, 2);
	lcd.clear(); lcd.print("ZigBee Geiger");
	xbee_init( 0x00 );					// XBee用COMポートの初期化(引数はポート番号)
	lcd.clear(); lcd.print("ATSP=20sec");
	xbee_at( "ATSP07D0");				// End Device用データ保持期間を20秒に設定
}

void loop(){
	byte i;
	byte trig=0;		// 測定状態 0:停止中 1:測定中 2:終了中 3:バッテリなし
	byte dev_en = 0;	// センサー発見の有無(0:未発見)
	
	unsigned long time = 0;
	unsigned long time_ofs;
	char time_s[6] = "00:00";
	byte time_min;
	byte time_sec;
	byte time_prev;
	byte buttons = 0;
	unsigned int battery = 0;

	XBEE_RESULT xbee_result;			// 受信データ用の構造体

	lcd.clear(); lcd.print("ATNJ=30");
	
	if( xbee_atnj(30) ){				// 30秒間の参加受け入れ
		lcd.clear(); lcd.print("Found device");
		xbee_from( dev_sens );			// 受信したアドレスをdev_sensへ格納
		dev_en = 1;						// sensor発見済
		lcd.clear(); lcd.print("gpio init");
		xbee_gpio_init(dev_sens);		// デバイスdev_sensにIO設定を行うための送信(End Deviceは設定できるとは限らない)
		xbee_rat( dev_sens , "ATD405");	// ポケットガイガをOFFにする
		xbee_rat( dev_sens , "ATSM05");	// Cyclic Sleep wakes on timer expirationに設定
		xbee_rat( dev_sens , "ATSP07D0");	// 送信間隔を20秒に設定
		lcd.clear(); lcd.print("DONE");

		for(i=0; i<DATA_NUM;i++ ) cpmHistory[i]=0;

		delay(3000);
		time_ofs = millis();
		lcd.clear(); lcd.print("Measuring...");
		trig = 1 ;						// 測定中
		time_prev = 0 ;					// 前回の測定時刻の秒
		xbee_rat( dev_sens , "ATD404");	// ポケットガイガをONにする
		
		// メイン処理
		while(1){							// 永久に受信する
			
			time = millis() - time_ofs;
			time_sec = (byte)(( time /  1000 ) % 60);
			time_min = (byte)(( time / 60000 ) % 60);
			time_s[0] = (char)( (time_min / 10) + '0');
			time_s[1] = (char)( (time_min % 10) + '0');
			time_s[3] = (char)( (time_sec / 10) + '0');
			time_s[4] = (char)( (time_sec % 10) + '0');
		
			#ifdef ADAFURUIT
			switch( lcd.readButtons() ){
				case BUTTON_UP:
					lcd.clear(); lcd.print("TRIG = ON");
					trig = 0;
					time_min == 0;
					break;
				case BUTTON_DOWN:
					lcd.clear(); lcd.print("TRIG = OFF");
					trig = 1;
					time_min = TIME_MEAS;
					break;
				case BUTTON_SELECT:
					lcd.clear(); lcd.print("BATT CHECK");
					xbee_batt_force( dev_sens );
					break;
				default:
					break;
			}			
			#endif
			
			/* データ受信(待ち受けて受信する) */
			xbee_rx_call( &xbee_result );	// データを受信します。(処理に50msかかります)
											// data：受信結果の代表値
											// xbee_result：詳細な受信結果
											// xbee_result.MODE：受信モード
			switch( xbee_result.MODE ){		// 受信したデータの内容(MODE値)に応じて
				case MODE_RESP:				// sensorからの応答の場合に照度と温度を表示
				case MODE_GPIN:				// 周期方式でデータ受信した場合も想定
					if( bytecmp( dev_sens , xbee_result.FROM ,8 ) == 0 ){
						signCount++;		//放射線パルス(Low)をカウント
						if( xbee_result.GPI.PORT.D1 == signValue ){
							signCount++;	//パケットロス対策
							lcd.clear(); lcd.print("detected packet loss!");
						}
						signValue = xbee_result.GPI.PORT.D1;					// 現在の値を保存
						if( xbee_result.GPI.PORT.D2 == 1 ) {
							noiseCount++;		//ノイズ(High)をカウント
							lcd.clear(); lcd.print("detected noise!");
						}
					}
					break;
				case MODE_BATT:
					battery = xbee_result.ADCIN[0];
					if( battery < 2800 ) trig = 3;
					break;
				default:
					break;
			}
			
			/* 測定トリガ */
			if( trig == 1 ){						// 測定中の場合
				if( time_min == TIME_MEAS ){
					trig = 2;						// 20秒後に停止する
					xbee_rat( dev_sens , "ATD405");	// ポケットガイガをOFFにする
					xbee_batt_force( dev_sens );	// 電圧チェック
				}
			}else if( trig == 0 ){					// 測定していないとき
				if( time_min == 0 ){
					trig = 1;
					xbee_rat( dev_sens , "ATD404");	// ポケットガイガをONにする
					for(i=0; i<DATA_NUM;i++ ) cpmHistory[i]=0;
					cpmIndex = 0;
					cpm = 0;
					cpmTimeSec = 0;
				}
			}else if( trig == 2 ){ 					// 20秒後に停止する
				if( time_sec == 20 ){
					trig = 0;
				}
			}// trig == 3 何もしない
			
			if( ( time_sec % 6 == 0 ) && (time_prev != time_sec ) ){
				time_prev = time_sec;
				if( trig != 0){
					if(noiseCount == 0){				//ノイズが検出されないとき（ノイズが検出されたときは、処理をしない）
						cpmIndex++;
						if(cpmIndex >= DATA_NUM){        //最後(TIME_MEAS分後)まできたら元に戻す
							cpmIndex = 0;
						}
						//一周してきたときに次に格納する配列に値が詰まっていれば、
						//現在の合計しているカウント(変数cpm)から引いて、無かったことにする
						if(cpmHistory[cpmIndex] > 0){
							cpm -= cpmHistory[cpmIndex];
						}

						cpmHistory[cpmIndex] = signCount;
						cpm += signCount;				//カウント回数の蓄積
						
						if( cpmTimeSec >= TIME_MEAS * 60 ){		//cpmを求めるときに使用する計測時間(最大TIME_MEAS分)を加算
							cpmTimeSec = TIME_MEAS * 60;
						}else{
							cpmTimeSec += 6 ;
						}
					}
					min = cpmTimeSec / 60.0;      //現在の計測時間(最大TIME_MEAS分)
					if(min!=0){        //cpm、uSv/h、uSv/hの誤差をそれぞれ計算する
						cpmBuff = cpm / min;
						uSvBuff = cpm / min / alpha;
						uSvdBuff = sqrt(cpm) / min / alpha;
					}else{        //0割り算のときは、0にする
						cpmBuff = 0;
						uSvBuff = 0;
						uSvdBuff = 0;
					}
				}
				//次のループのための変数の初期化
				signCount=0;
				noiseCount=0;
			}
			//液晶への表示
			lcd.setCursor(0,0);
			if( battery > 0 ){
				lcd.print( battery );
				lcd.print(" mV    ");
			}else{
				lcd.print( cpmBuff );
				lcd.print(" cpm   ");
			}
			lcd.setCursor(11,0);
			lcd.print( time_s );
			lcd.print(" ");
			lcd.setCursor(0,1);
			lcd.print( uSvBuff );
			lcd.print(" uSv/h ");
			if( trig == 0 ) lcd.print("off  ");
			else if ( trig == 1 ) lcd.print("ON   ");
			else if ( trig == 2 ) lcd.print("...  ");
			else lcd.print("noBATT");
		}
	}
}
