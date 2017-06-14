//スピーカーなります
/*pin3.11のpwm出力できなくなります*/
//Ａ～Ｇを引数にとるとその音がなります
void speaker(int TONE) {
  tone(TONE_PINNO, TONE, BEAT_LONG);
  delay(BEAT_LONG);
}

