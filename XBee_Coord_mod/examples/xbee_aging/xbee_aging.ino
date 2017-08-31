/*********************************************************************
XBee ライブラリ・自動システム試験ツール

　元々、XBeeライブラリ「ZB Coord API」のリリース用のテストツールとして
　作成して、使用していたものです。
　XBeeライブラリを用いた時の評価用に公開することにいたしました。
　電波環境が悪いと100％になりません。
　Arduino用PC用ともにキーを押すとエラー数と合格回数、テスト回数、最後の
　エラーの内容を表示します。

本ソースリストおよびソフトウェアは、ライセンスフリーです。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

                               Copyright (c) 2010-2014 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/

/*********************************************************************

●試験項目●
		①ローカルATコマンド試験
		②ローカルxbeeライブラリ用コマンド試験
		③リモートATコマンド試験
		④リモートxbeeライブラリ管理コマンド試験
		⑤リモートxbeeライブラリ応答待ちコマンド試験
		⑥ループバック試験

●試験環境●
		○本ソフトを動かす親機用ArduinoまたはPC
		○親機用XBee ZBモジュール【ファーム=ZIGBEE COORDINATOR API】
		○子機用XBee ZBモジュール【ファーム=ZIGBEE ROUTER AT】
		○子機のUARTはループバック接続が必要です。(DINとDOUTを接続)
*/

//	#define DEBUG			//DEBUGをdefineするとライブラリの動作ログが出力されます。
//	#define DEBUG_TX		//送信データ表示
//	#define		ERRLOG
#define		EASY_SENSOR
#define		XBEE_ERROR_TIME
#define		PASSED			0
#define		FAILED			1
#ifdef ARDUINO
	#define		SIZEOF_LOG_S	36	// 33以上
#else
	#define		SIZEOF_LOG_S	80
#endif

#ifdef ARDUINO
	#include <xbee.h>
	#include <Wire.h>
	#include <Adafruit_MCP23017.h>
	#include <Adafruit_RGBLCDShield.h>
	Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#else
	#include "../libs/xbee.c"
	#include <ctype.h>					// isgraph
	#include "../libs/kbhit.c"
#endif

byte dev[8];
byte my[8];
byte myt[8];
byte panid[8];
byte panidt[8];
byte TEST_COUNT=0;
byte ERROR_COUNT=0;
byte ERROR_TOTAL=0;
char ERROR_S[SIZEOF_LOG_S];
unsigned int pan;
byte nc,rnc=0xFF,dd=0xFF;

static const char UARTDATA[] = "0123456789ABCDEF";
char *sprint_FAIL(char *s,const char *text){
	sprintf(s,"%s:FAIL####",text);
	return s;
}

void aging_delay(unsigned int ms){
	#ifdef ARDUINO
		delay((unsigned long)ms);
	#else
		wait_millisec(ms);
	#endif
}

#ifdef ARDUINO
void prints(char *s){
	byte x,y;
	byte c=1;
	for(y=0;y<2;y++){
		lcd.setCursor(0, y);
		for(x=0;x<16;x++){
			if(s[16*y+x]=='\0') c=0;
			if(isgraph(s[16*y+x]) && c) lcd.print(s[16*y+x]);
			else lcd.print(' ');
		}
	}
}
#endif

void aging_key(void){
	#ifdef ARDUINO
		if( lcd.readButtons() ){
			lcd.clear();
			lcd.print(ERROR_COUNT,DEC);					// 3
			lcd.print("errs ");							// 5
			lcd.print((TEST_COUNT-ERROR_TOTAL),DEC);	// 3
			lcd.print('/');								// 1
			lcd.print(TEST_COUNT,DEC);					// 3
			lcd.setCursor(0, 1);
			if(ERROR_COUNT || ERROR_TOTAL){
				lcd.print(ERROR_S);
			}
			delay(100);
			while( lcd.readButtons() );
			delay(100);
			while( !lcd.readButtons() );
			prints(ERROR_S);
			delay(3000);
			while( lcd.readButtons() );
		}
	#else
		if( kbhit() ){
			getchar();
			printf("----------------------------------------------------------------\n");
			printf("ERROR=%d(TEST:%d/%d) ",ERROR_COUNT,TEST_COUNT-ERROR_TOTAL,TEST_COUNT);
			if(ERROR_COUNT || ERROR_TOTAL){
				printf("%s",ERROR_S);
			}
			printf("\n----------------------------------------------------------------\n");
			aging_delay(3000);
		}
	#endif
}

void aging_log(char *s){
	#ifdef ARDUINO
		prints(s);
	#else
		time_t error_time;
		struct tm *error_time_st;
		
		time(&error_time);
		error_time_st = localtime(&error_time);
		printf("%4d/%02d/%02d %02d:%02d:%02d %s\n",
			error_time_st->tm_year+1900,
			error_time_st->tm_mon+1,
			error_time_st->tm_mday,
			error_time_st->tm_hour,
			error_time_st->tm_min,
			error_time_st->tm_sec,
			s
		);
	#endif
	aging_key();
}

void error_log(char *s){
	#ifndef ARDUINO
		FILE *fp;
		time_t error_time;
		struct tm *error_time_st;
		
		time(&error_time);
		error_time_st = localtime(&error_time);
		if( (fp=fopen("err.log","a")) != NULL ){
			fprintf(fp,"%4d/%02d/%02d %02d:%02d:%02d %s\n",
				error_time_st->tm_year+1900,
				error_time_st->tm_mon+1,
				error_time_st->tm_mday,
				error_time_st->tm_hour,
				error_time_st->tm_min,
				error_time_st->tm_sec,
				s
			);
			fclose(fp);
		}
	#endif
	aging_log(s);
	strcpy(ERROR_S,s);
	ERROR_COUNT++;
	aging_delay(1000);
	aging_key();
}

void aging_results(void){
	char s[SIZEOF_LOG_S];
	TEST_COUNT++;
	if(ERROR_COUNT==0){
		//         012345678901234567890123456 28～30bytes
		sprintf(s,"###TEST:PASSED  (%d/%d)####",TEST_COUNT-ERROR_TOTAL,TEST_COUNT);
		aging_log(s);
	}else{
		ERROR_TOTAL++;
		//         01234567890123456789012345678 30～33bytes
		sprintf(s,"###TEST:FAILED  %d(%d/%d)####",ERROR_COUNT,TEST_COUNT-ERROR_TOTAL,TEST_COUNT);
		aging_log(s);
		#ifndef ARDUINO
			strcpy(s,ERROR_S);
			aging_log(s);
		#endif
		ERROR_COUNT=0;
	}
	aging_delay(3000);
}

void aging_print(char *text,byte ret,byte target,byte judge){
	char s[SIZEOF_LOG_S];
	if( (judge==PASSED && target == ret) || 
		(judge==FAILED && target != ret)
	){
		sprintf(s,"%s:PASS(%02X)",text,ret);
		aging_log(s);
	}else{
		//         0-789012345678901--234--  26bytes
		sprintf(s,"%sret=%02X:%02X",sprint_FAIL(s,text),ret,target);
		error_log(s);
	}
}

void aging_rx_resp(char *text,char *at,byte n){
	XBEE_RESULT xbee_result;
	byte data;
	byte i,j;
	char s[SIZEOF_LOG_S];
	for(i=0;i<n;i++){
		data=xbee_rx_call( &xbee_result );
		if(	(	xbee_result.MODE==MODE_RESP &&
				xbee_result.STATUS == STATUS_OK &&
				xbee_result.AT[0] == 'I' &&
				xbee_result.AT[1] == 'S' &&
				bytecmp(dev,xbee_result.FROM,8)==0
			)
		//	|| (
		//		xbee_result.MODE==MODE_GPIN &&
		//		bytecmp(dev,xbee_result.FROM,8)==0
		//	)
		){
			sprintf(s,"%s:PASS(%02X)",text,data);
			for(j=0;j<8;j++){
				if( (data>>(7-j))&0x01 ) s[16+j]='1';
				else s[16+j]='0';
			}
			s[24]='\0';
			aging_log(s);
			return;
		}
	}
	error_log(sprint_FAIL(s,text));
}

void aging_rx_uart(char *text,const char *uart,byte n){
	XBEE_RESULT xbee_result;
	byte i,j;
	byte len;
	char s[SIZEOF_LOG_S];
	byte data[SIZEOF_LOG_S];
	
	for(i=0;i<n;i++){
		len=xbee_rx_call( &xbee_result );
		if(	xbee_result.MODE==MODE_UART &&
			bytecmp(dev,xbee_result.FROM,8)==0
		){
			for(j=0;j<SIZEOF_LOG_S;j++) data[j]=(byte)uart[j];
			if(len>SIZEOF_LOG_S) len=SIZEOF_LOG_S;
			if(bytecmp(data,xbee_result.DATA,len)==0){
				sprintf(s,"%s:PASS(%02d)  ",text,len);
				for(j=16;j<32;j++){
					if(isgraph(xbee_result.DATA[j-16])){
						s[j]=(char)xbee_result.DATA[j-16];
					}else s[j]=' ';
				}
				s[32]='\0';
				aging_log(s);
				if(len!=strlen(uart)){
					sprintf(s,"%s:WARNING txLen=%d,rx=%d",text,strlen(uart),len);
					aging_log(s);
				}
				return;
			}else{
				sprintf(s,"%s:FAIL(%02d)################",text,len);
				for(j=16;j<24;j++){
					if(xbee_result.DATA[j-16]>16){
						s[j]=(char)xbee_result.DATA[j-16];
					}else s[j]=' ';
				}
				error_log(s);
				return;
			}
		}
	}
	error_log(sprint_FAIL(s,text));
}

byte s2bytes(byte *bytes,const char *s){
	byte i;
	for(i=0;i<strlen(s);i++) bytes[i]=s[i];
	return(i);
}

void aging_rx_batt(char *text,byte n){
	XBEE_RESULT xbee_result;
	byte i;
	char s[SIZEOF_LOG_S];
	for(i=0;i<n;i++){
		xbee_rx_call( &xbee_result );
		if(	xbee_result.MODE==MODE_BATT &&
			xbee_result.STATUS == STATUS_OK &&
			bytecmp(dev,xbee_result.FROM,8)==0 &&
			xbee_result.ADCIN[0] > 1000
		){
			sprintf(s,"%s:PASS(00)%d[mV]",text,xbee_result.ADCIN[0]);
			aging_log(s);
			return;
		}
	}
	error_log(sprint_FAIL(s,text));
}

char byte2a(byte in, byte i){
	char ret;
	if(i) in /= 16;
	ret = (char)(in%16);
	if(ret<10) ret += '0';
	else ret += 'A' -10;
	return( ret );
}

byte pairing(byte *dev,byte n){
	XBEE_RESULT xbee_result;
	char s[SIZEOF_LOG_S];

	byte i,len;
	for(i=0;i<n;i++){
		xbee_rx_call( &xbee_result );
		if(	xbee_result.MODE==MODE_IDNT ){
			sprintf(s,"Pairing:PASS(%02X)",xbee_result.STATUS);
			len=strlen(s);
			for(i=0;i<8;i++){
				dev[i]=xbee_result.FROM[i];
				s[len]  =byte2a(dev[i],1);
				s[len+1]=byte2a(dev[i],0);
				len+=2;
			}
			s[len]='\0';
			aging_log(s);
			return(1);
		}
	}
	return(0);
}

void aging_rx_none(char *text,byte n){
	XBEE_RESULT xbee_result;
	byte i;
	char s[SIZEOF_LOG_S];
	for(i=0;i<n;i++){
		xbee_rx_call( &xbee_result );
		if(	xbee_result.MODE != 0x00 ){
			sprintf(s,"%smode=%02X stat=%02X",sprint_FAIL(s,text),xbee_result.MODE,xbee_result.STATUS);
			error_log(s);
			return;
		}
	}
	sprintf(s,"%s:PASS",text);
	aging_log(s);
}

void aging_init(void){
	char s[SIZEOF_LOG_S];
	byte i;
	
	xbee_atnj(0xFF);
	aging_delay(1000);
	aging_log("Press Commiss.SW");
	/* ペアリング＆宛先アドレス取得 */
	while(pairing(dev,100)==0);
	sprintf(s,"DestAdr:");
	for(i=0;i<4;i++){
		s[8+2*i]=byte2a(dev[i+4],1);
		s[9+2*i]=byte2a(dev[i+4],0);
	}
	s[16]='\0';
	aging_log(s);
	xbee_atnj(0);
	aging_delay(1000);
	/* 自分のアドレス */
	xbee_myaddress(my);
	sprintf(s,"MyAdr  :");
	for(i=0;i<4;i++){
		s[8+2*i]=byte2a(my[i+4],1);
		s[9+2*i]=byte2a(my[i+4],0);
	}
	s[16]='\0';
	aging_log(s);
	aging_delay(1000);
	/* PAN ID */
	pan=xbee_atop(panid);
	sprintf(s,"PAN ID :%d        ",pan);
	for(i=0;i<8;i++){
		s[16+2*i]=byte2a(panid[i],1);
		s[17+2*i]=byte2a(panid[i],0);
	}
	s[32]='\0';
	aging_log(s);
	aging_delay(1000);
	/* 子機数など */
	nc=xbee_atnc();
	while(rnc==0xFF){
		rnc=xbee_ratnc(dev);
	}
	while(dd==0xFF){
		dd=xbee_ping(dev);
	}
	sprintf(s,"nOfChld:%d,%d,%02X",10-nc,12-rnc,dd);
	aging_log(s);
	aging_delay(1000);
}

#ifdef ARDUINO
void setup(){
	byte i;
	
	lcd.begin(16, 2);
	lcd.clear();
	lcd.print("XBee Aging");
	lcd.setCursor(0, 1);
	lcd.print(COPYRIGHT);
	xbee_init(0);
	aging_init();
	for(i=0;i<SIZEOF_LOG_S;i++) ERROR_S[i]='\0';
}
#endif

void loop(){
	byte data[SIZEOF_LOG_S];
	byte i,len;

	aging_log("Started:========");

	// ①ローカルATコマンド試験
	#ifndef ARDUINO // PCのみ試験
		for(i=0;i<10;i++){
			aging_print("LocAt L",xbee_at("ATP004"),STATUS_OK,PASSED);
			aging_print("LocAt H",xbee_at("ATP005"),STATUS_OK,PASSED);
		}
	#endif
	aging_print("LocAt -",xbee_at("ATP001"),STATUS_OK,PASSED);
	
	// ②ローカルxbeeライブラリ用コマンド試験
//	aging_print("reset  ",xbee_reset(),0,FAILED);
	aging_print("atcb   ",xbee_atcb(1),0,PASSED);
	for(i=0;i<3;i++){
		aging_print("myAdr  ",xbee_myaddress(myt),0,FAILED);
		aging_print("myAdCmp",bytecmp(my,myt,8),0,PASSED);
		aging_print("atvr   ",xbee_atvr(),ZB_TYPE_COORD,PASSED);
		aging_print("atai   ",xbee_atai(),0,PASSED);
		aging_print("atop   ",(byte)xbee_atop(panidt),(byte)pan,PASSED);
		aging_print("atopCmp",bytecmp(panid,panidt,8),0,PASSED);
		aging_print("atnc   ",xbee_atnc(),nc,PASSED);			// ncと一致すること
	}

	// ③リモートATコマンド試験
	#ifndef ARDUINO // PCのみ試験
		for(i=0;i<10;i++){
			aging_print("RemAt L",xbee_rat(dev,"ATP104"),STATUS_OK,PASSED);		// 0xFF=ERROR
			aging_print("RemAt H",xbee_rat(dev,"ATP105"),STATUS_OK,PASSED);		// 0xFF=ERROR
		}
	#endif
	aging_print("LocAt -",xbee_at("ATP100"),STATUS_OK,PASSED);

	// ④リモートxbeeライブラリ管理コマンド試験
	aging_print("ratd   ",xbee_ratd(dev, my),0xFF,FAILED);	// 0xFF=ERROR
	aging_print("ratdMyA",xbee_ratd_myaddress(dev),0xFF,FAILED);	// 0xFF=ERROR
	aging_print("ratnc  ",xbee_ratnc(dev),rnc,PASSED);		// rncと一致すること
	aging_print("ping   ",xbee_ping(dev),dd,PASSED);		// ddと一致すること
	aging_print("batt   ",(byte)(xbee_batt(dev)<0),0,PASSED);
	aging_print("gpioIni",xbee_gpio_init(dev),dd,PASSED);	// ddと一致すること
	#ifdef ARDUINO
		aging_print("gpi 1  ",xbee_gpi(dev,1),0xFF,FAILED);
		aging_print("adc 1  ",(byte)(xbee_adc(dev,1)==0xFFFF),0,PASSED);
	#else // PCのみ試験
		for(i=0;i<3;i++){
			aging_print("gpi 1  ",xbee_gpi(dev,1),0xFF,FAILED);
			aging_print("gpi 2  ",xbee_gpi(dev,2),0xFF,FAILED);
			aging_print("gpi 3  ",xbee_gpi(dev,3),0xFF,FAILED);
		}
		for(i=0;i<3;i++){
			aging_print("adc 1  ",(byte)(xbee_adc(dev,1)==0xFFFF),0,PASSED);
			aging_print("adc 2  ",(byte)(xbee_adc(dev,2)==0xFFFF),0,PASSED);
			aging_print("adc 3  ",(byte)(xbee_adc(dev,3)==0xFFFF),0,PASSED);
		}
		for(i=0;i<3;i++){
			aging_print("sensorL",(byte)(xbee_sensor(dev,LIGHT)<0),0,PASSED);
			aging_print("sensorT",(byte)(xbee_sensor(dev,LIGHT)<0),0,PASSED);
			aging_print("sensorH",(byte)(xbee_sensor(dev,LIGHT)<0),0,PASSED);
		}
	#endif

	// ⑤リモートxbeeライブラリ応答待ちコマンド試験
	for(i=0;i<3;i++){
		aging_print("force  ",xbee_force(dev),0x00,FAILED);
		aging_rx_resp("rxResp ","IS",200);		// STATUS_OKのIS応答を200受信分、待つ
		aging_print("batForc",xbee_batt_force(dev),0x00,FAILED);	
		aging_rx_batt("rxBatt ",200);
	//	aging_print("ratForc",xbee_rat_force(dev,"ATIS"),0x00,FAILED);
	//	aging_rx_resp("rxResp ","IS",200);		// STATUS_OKのIS応答を200受信分、待つ
	}
	for(i=0;i<5;i++){
		aging_print("gpo11,0",xbee_gpo(dev,11,0),0,FAILED);
		aging_rx_none("rxNo110",10);
		aging_print("gpo11,1",xbee_gpo(dev,11,1),0,FAILED);
		aging_rx_none("rxNo111",10);
		#ifndef ARDUINO // PCのみ試験
			aging_print("gpo12,0",xbee_gpo(dev,12,0),0,FAILED);
			aging_rx_none("rxNo120",10);
			aging_print("gpo12,1",xbee_gpo(dev,12,1),0,FAILED);
			aging_rx_none("rxNo121",10);
		#endif
	}
	aging_rx_none("rxNone ",100);

	// ⑥ループバック試験
	for(i=0;i<3;i++){
		aging_print("uartTx ",xbee_uart(dev,UARTDATA),0,FAILED);
		aging_rx_uart("rxUart ",UARTDATA,200);	// UART応答を200受信分、待つ
		len=s2bytes(data,UARTDATA);
		aging_print("byteTx ",xbee_bytes(dev,data,len),0,FAILED);
		aging_rx_uart("rxByte ",UARTDATA,200);	// UART応答を200受信分、待つ
	}
	
	// 試験結果
	aging_results();
}

#ifndef ARDUINO
int main(int argc,char **argv){
	byte port=0;
	byte i;
	
	if( argc==2 ) port = (byte)(atoi(argv[1]));
	xbee_init( port );					// XBee用COMポートの初期化(引数はポート番号)
	aging_init();
	for(i=0;i<SIZEOF_LOG_S;i++) ERROR_S[i]='\0';
	
	while(1)loop();
	exit(1);
}
#endif
