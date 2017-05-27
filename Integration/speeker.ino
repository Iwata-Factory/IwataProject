//スピーカーなります
/*pin3.11のpwm出力できなくなります*/
//Ａ～Ｇを引数にとるとその音がなります
void speeker(int TONE) {
     tone(TONE_PINNO, TONE, BEAT);
     delay(BEAT);
}

