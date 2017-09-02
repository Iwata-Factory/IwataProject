#ifndef __FLAG_H__
#define __FLAG_H__

// 各ステータスを飛ばすかどうか
// 1なら実行、0なら飛ばします
#define _S1_ 0
#define _S2_ 0
#define _S3_ 0
#define _S4_ 0
#define _S5_ 1
#define _S6_ 0

// 設定用フラグ(1のほうが標準)
#define XBEE_SWITCH 1  // 0にするとxbeeを全て無視します(今は動作不安定っぽい)
#define LAND_JUDGE_FLAG 1 // 着陸判定を行うならば1.飛ばすなら0.
#define CALIBRATION_FLAG 0 // キャリブレーションを行うなら1.しないなら0.
#define STACK_ESCAPE_FLAG 0 // スタック関連の処理を行うならば1.行わないならば0
#define STACK_MODE 1  //0でスタック状況を確かめる。1で面倒なことをしない.
#define DANGER_AREA_FRAG 0 //危険エリア脱出処理を行うならば1.行わないならば0.
#define SD_LOG_FLAG 1  // SDに書き込みを行うなら1.行わないなら0.
#define SPHERE_FLAG 1  // 1で球面三角法を用いる.0では平面として処理.(球面三角法推奨
#define GPS_GET_FLAG 1 // 0の時GPSを取らない
#define PI_FLAG 1 
#define CAMERA 0  // 0の時カメラを動作させない
// 1の時PI制御で直進する。

// テスト用フラグ
#define STACK_EXP 0  // 1の場合スタックのフラグが立った地点までスキップ
#define NEAR_GOAL_STACK_EXP 0 // 本来のシーケンスではゴール付近でスタック判定は行いませんが、1にした場合はゴール付近でもスタック判定を行います.

#endif
