#ifndef __FLAG_H__
#define __FLAG_H__


// これから色々な状態を試すことも多くなると思うのでここにフラグを記載します

// 各ステータスを飛ばすかどうか
// 1なら実行、0なら飛ばします
#define _S1_ 0
#define _S2_ 0
#define _S3_ 0
#define _S4_ 0
#define _S5_ 1
#define _S6_ 0

#define XBEE_SWITCH 1  // 0にするとxbeeを全て無視します(今は動作不安定っぽい)

#define GROUND1_FLAG 0  //ゴーアンドストップなら0.PIDなら1.
#define LAND_JUDGE_FLAG 0 // 着陸判定を行うならば1.飛ばすなら0.
#define CALIBRATION_FLAG 1 // キャリブレーションを行うなら1.しないなら0.
#define STACK_ESCAPE_FLAG 0 // スタック関連の処理を行うならば1.行わないならば0.
#define DANGER_AREA_FRAG 0 //危険エリア脱出処理を行うならば1.行わないならば0.
#define SD_LOG_FLAG 1  // SDに書き込みを行うなら1.行わないなら0.
#define SPHERE_FLAG 1  // 1で球面三角法を用いる.0では平面として処理.(球面三角法推奨

#endif
