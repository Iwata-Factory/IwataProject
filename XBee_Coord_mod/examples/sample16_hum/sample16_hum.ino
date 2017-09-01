/*********************************************************************
サンプルアプリ16　自作・湿度センサー HS-15専用
新しいセンサーデバイスを発見すると、そのデバイスから湿度を得ます。

詳細：http://www.geocities.jp/bokunimowakaru/diy/xbee/xbee-hum.html

本ソースリストおよびソフトウェアは、ライセンスフリーです。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

                               Copyright (c) 2010-2014 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/
/*
	port:	port指定	IO名	ピン番号			センサーデバイス
			port= 0		DIO0	XBee_pin 20 (Cms)	 SW1 Commision
			port= 1		DIO1	XBee_pin 19 (AD1)	 照度センサー
			port= 2		DIO2	XBee_pin 18 (AD2)	 温度センサー
			port= 3		DIO3	XBee_pin 17 (AD3)	 湿度センサー
*/
//#define DEBUG
#include <xbee.h>
#include <LiquidCrystal.h>
#define LUX_ADJ		1.0						// 照度センサの補正値
#define TEMP_ADJ	+1.8					// 温度センサの補正値
#define BATT_CHECK	10						// 電圧確認間隔(0～255)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void hum_log(byte row, char *s){
	byte i;
	lcd.setCursor(0, row);
	for(i=0;i<16;i++){
		if((isgraph(s[i]) || s[i]==(char)0xDF) && i<strlen(s)) lcd.write(s[i]);
		else lcd.write(' ');
	}
	delay(250);
}

byte local_at(char *at){
	byte ret;
	char s[17];			// 表示用
	
	ret=xbee_at(at);
	//         0-789ABCDEF
	sprintf(s,"%8s  ret=%02X",at,ret);
	hum_log(1,s);
	return(ret);
}

byte remote_at(byte *dev,char *at){
	byte ret;
	char s[17];			// 表示用
	
	ret=xbee_rat(dev,at);
	sprintf(s,"%8s  ret=%02X",at,ret);
	hum_log(1,s);
	return(ret);
}

byte set_ports(byte *dev){
	byte ret=0;
	/* 子機の設定 */     // 01234567	8バイト 
	if( (ret=remote_at(dev,"ATPR1FF1"))==0 )	// ポート1～3のプルアップを解除
	if( (ret=remote_at(dev,"ATD102"))==0 )		// ポート1をアナログ入力へ
	if( (ret=remote_at(dev,"ATD202"))==0 )		// ポート2をアナログ入力へ
	if( (ret=remote_at(dev,"ATD302"))==0 )		// ポート3をアナログ入力へ
	if( (ret=remote_at(dev,"ATST01F4"))==0 )	// スリープ移行待ち時間	ST=500ms
//	if( (ret=remote_at(dev,"ATSP07D0"))==0 )	// スリープ間隔	 20秒	SP=2000×10ms
	if( (ret=remote_at(dev,"ATSP01F4"))==0 )	// スリープ間隔	 5秒	SP=500×10ms(テスト用)
	if( (ret=remote_at(dev,"ATSN01"))==0 )		// スリープ回数	 1回
	if( (ret=remote_at(dev,"ATSM04"))==0 )		// サイクリックスリープ	SM=04
	if( (ret=remote_at(dev,"ATIR1388"))==0 )	// 自動送信周期			IR=5000ms
	if( (ret=remote_at(dev,"ATWH00"))==0 )		// 測定待ち時間			WH=0ms
	if( (ret=remote_at(dev,"ATAC"))==0 )		// 確定
	if( (ret=remote_at(dev,"ATSI"))==0 )		// 即スリープへ
	;
	if(ret) hum_log(1,"ERROR set_ports");
	return(ret);
}

void setup(){
	lcd.begin(16, 2);								// LCDのサイズを16文字×2桁に設定
	lcd.clear();									// 画面消去
	lcd.print("Hum Sensors");						// 文字を表示
	xbee_init( 0x00 );
	xbee_atnj( 0xFF );
	local_at("ATSP0AF0");				// 親機 SP=28000ms (28秒)
	local_at("ATSN0E10");				// 親機 SN=3600回 (28秒*3600回=28時間)	batt = 3.00;
}

void loop(){
	char s[17];							// 表示用
	XBEE_RESULT xbee_result;
	float lux,temp,hum,batt=3.0;		// 照度[Lux]、温度[℃]、湿度[%]、電池電圧[V]
	byte trig_batt=BATT_CHECK;
	byte dev_sens[8];
	
	sprintf(s,"Receiving  AI=%02X",xbee_atai());
	hum_log(0,s);
	while(1){							// 永久に受信する
		/* XBee子機の電圧測定 */
		if( trig_batt == 0){
			xbee_batt_force(dev_sens);
			trig_batt=BATT_CHECK;
		}
		/* データ受信(待ち受けて受信する) */
		xbee_rx_call( &xbee_result );	// データを受信します。
										// data：受信結果の代表値
										// xbee_result：詳細な受信結果
										// xbee_result.MODE：受信モード
		switch( xbee_result.MODE ){		// 受信したデータの内容(MODE値)に応じて
			case MODE_RESP:				// sensorからの応答の場合に照度と温度を表示
				if( xbee_result.STATUS != STATUS_OK) break;
			case MODE_GPIN:				// sensorからの応答の場合に照度と温度を表示
			//	sprintf(s,"%4d, %4d, %4d",xbee_result.ADCIN[1],xbee_result.ADCIN[2],xbee_result.ADCIN[3]);
			//	hum_log(1,s);
				if( bytecmp(xbee_result.FROM,dev_sens,8)==0 ){
					/* 照度測定
					    照度センサ    [+3.3V]----(C)[照度センサ](E)--+--[10kΩ]----[GND]
					    =NJL7502L                                    |
					                                                 +--(19)[XBee ADC1]
					                                                 
						lux = I(uA) / 0.3 = ADC(mV) / 10(kOhm) / 0.3
						    = ADC値 /1023 * 1200(mV) / 10(kOhm) / 0.3
						    = ADC値 * 0.391
					*/
					lux = (float)xbee_result.ADCIN[1] * 0.391 * LUX_ADJ;
					
					/* 温度測定
					    温度センサ    [+3.3V]----(1VSS)[温度センサ](3GND)---[GND]
					    =LM61CIZ                      (2VOUT)|
					                                         +--(18)[XBee ADC2]
					                                         
						Temp = ( ADC(mV) - 600 ) / 10 = ( ADC値 /1023 * 1200(mV) - 600 ) / 10
						     = ADC値 * 0.1173 - 60;
					*/
					temp = (float)xbee_result.ADCIN[2] * 0.1173 - 60 + TEMP_ADJ;
						
					/* 湿度測定
						湿度センサ HS-15P
						[+3.3V]---[series100k]--+--[0.22uF]--+--[HS-15P]---[GND]
						           分圧         |            |
						    [GND] --[shunt68k]--+            +--(17)[XBee ADC3]
						    
						その他の変換
							HS15湿度センサーと簡易回路による湿度計実験
							http://homepage3.nifty.com/sudamiyako/zk/hs15/hs15.htm
					*/
					hum = (float)xbee_result.ADCIN[3] / batt * 3.256;
					hum = 7.4396E-05*hum*hum - 1.2391E-01*hum + 1.9979E+02;
					hum += (hum*35-13056)/64*temp/64;
					hum = (5*hum /16 *hum + 44* hum )/256 +20;
					//         0-3456 789 AB CD EF
					sprintf(s,"%4dLx %2d%cC %2d%%",(int)lux,(byte)temp,(char)0xDF,(byte)hum);
					hum_log(0,s);
					trig_batt--;
				}
				break;
			case MODE_BATT:				// 電池電圧を受信
				if( bytecmp(xbee_result.FROM,dev_sens,8)==0 ){
					batt = (float)xbee_result.ADCIN[0] /1000.;
					sprintf(s,"    Batt %4dmV",(int)(batt*1000));
					trig_batt=BATT_CHECK;
					hum_log(1,s);
				}
				break;
			case MODE_IDNT:				// 新しいデバイスを発見
				xbee_from( dev_sens );	// 受信したアドレスをdev_sensへ格納
				//         01234567  89  AB  CD  EF
				sprintf(s,"Found : %02X%02X%02X%02X",dev_sens[4],dev_sens[5],dev_sens[6],dev_sens[7]);
				hum_log(0,s);
				if( set_ports( dev_sens ) == 0){
					xbee_atnj( 0 );			// 1台しか参加させない
					trig_batt=0;
				}
				break;
			default:
				break;
		}
	}
}
