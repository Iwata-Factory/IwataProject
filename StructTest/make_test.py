# coding:utf8

import os
import random


def go_smoothly(path, times):
    f = open(path, "a")
    # 加速部
    f.write("  for (int i = 1; i < 256; i++) {\n")
    f.write("    go.right1 = 0;\n    go.right2 = i;\n    go.leght1 = 0;\n    go.leght2 = i;\n    rover_analog(go);\n")
    f.write("    delay(5);\n  }\n")
    # 直進部
    f.write("  go.right1 = 0;\n  go.right2 = 1;\n  go.leght1 = 0;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(")
    f.write(times)
    f.write(");\n")
    # 減速部
    f.write("  for (int i = 255; i > 0; i--) {\n")
    f.write("    go.right1 = 0;\n    go.right2 = i;\n    go.leght1 = 0;\n    go.leght2 = i;\n    rover_analog(go);\n")
    f.write("    delay(5);\n  }\n")
    # 停止部
    f.write("  go.right1 = 1;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(2500);\n")
    f.close()


def go_suddenly(path, times):
    f = open(path, "a")
    f.write("  go.right1 = 0;\n  go.right2 = 1;\n  go.leght1 = 0;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(")
    f.write(times)
    f.write(");\n  go.right1 = 1;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(2500);\n")
    f.close()


def turn_right_smoothly(path, times):
    f = open(path, "a")
    # 加速部
    f.write("  for (int i = 1; i < 256; i++) {\n")
    f.write("    go.right1 = 0;\n    go.right2 = i;\n    go.leght1 = i;\n    go.leght2 = 0;\n    rover_analog(go);\n")
    f.write("    delay(5);\n  }\n")
    # 回転部
    f.write("  go.right1 = 0;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 0;\n  rover_degital(go);\n")
    f.write("  delay(")
    f.write(times)
    f.write(");\n")
    # 減速部
    f.write("  for (int i = 255; i > 0; i--) {\n")
    f.write("    go.right1 = 0;\n    go.right2 = i;\n    go.leght1 = i;\n    go.leght2 = 0;\n    rover_analog(go);\n")
    f.write("    delay(5);\n  }\n")
    # 停止部
    f.write("  go.right1 = 1;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(2500);\n")
    f.close()


def turn_right_suddenly(path, times):
    f = open(path, "a")
    f.write("  go.right1 = 0;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 0;\n  rover_degital(go);\n")
    f.write("  delay(")
    f.write(times)
    f.write(");\n  go.right1 = 1;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(2500);\n")
    f.close()


def turn_leght_smoothly(path, times):
    f = open(path, "a")
    # 加速部
    f.write("  for (int i = 1; i < 256; i++) {\n")
    f.write(
        "    go.right1 = i;\n    go.right2 = 0;\n    go.leght1 = 0;\n    go.leght2 = i;\n    rover_analog(go);\n")
    f.write("    delay(5);\n  }\n")
    # 回転部
    f.write("  go.right1 = 1;\n  go.right2 = 0;\n  go.leght1 = 0;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(")
    f.write(times)
    f.write(");\n")
    # 減速部
    f.write("  for (int i = 255; i > 0; i--) {\n")
    f.write(
        "    go.right1 = i;\n    go.right2 = 0;\n    go.leght1 = 0;\n    go.leght2 = i;\n    rover_analog(go);\n")
    f.write("    delay(5);\n  }\n")
    # 停止部
    f.write("  go.right1 = 1;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(2500);\n")
    f.close()


def turn_leght_suddenly(path, times):
    f = open(path, "a")
    f.write("  go.right1 = 1;\n  go.right2 = 0;\n  go.leght1 = 0;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(")
    f.write(times)
    f.write(");\n  go.right1 = 1;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(2500);\n")
    f.close()


def get_t():
    while True:
        t = input("何ミリ秒行いますか？\n")
        if t.isdigit() and int(t) > 0:
            return t
        else:
            print("不適切な値が入力されたので再度入力して下さい。")
            print("")
            continue


def main():
    c = os.getcwd()  # カレントディレクトリを取得
    source_str = 'abcdefghijklmnopqrstuvwxyz1234567890'
    random.choice(source_str)
    name = "".join([random.choice(source_str) for i in range(0, 6)])  # ファイル名をランダムに生成
    print("機体走行テストファイルを作成します。")
    print("テストファイル名は" + name + ".inoです。")
    os.mkdir(os.path.join(c, name))
    print("ディレクトリの用意が完了。")
    f_path = os.path.join(c, name, name + ".ino")
    f = open(f_path, "w")
    print("inoファイルの作成が完了。")
    print("書き込みを開始します。")
    print("枠型のセットアップ")
    f.write("#include <Wire.h>\n")
    f.write("#include <SoftwareSerial.h>\n")
    f.write("#define SERIAL_BAUDRATE 9600\n")
    f.write("#define M1_1 8\n")
    f.write("#define M1_2 9\n")
    f.write("#define M2_1 10\n")
    f.write("#define M2_2 11\n")
    f.write("typedef struct {\n")
    f.write("  int right1 = 0;\n")
    f.write("  int right2 = 0;\n")
    f.write("  int leght1 = 0;\n")
    f.write("  int leght2 = 0;\n")
    f.write("} DRIVE;\n")
    f.write("void rover_degital(DRIVE drive) {\n  digitalWrite(M1_1, drive.right1);\n  digitalWrite(M1_2, drive.right2);\n")
    f.write("  digitalWrite(M2_1, drive.leght1);\n  digitalWrite(M2_2, drive.leght2);\n}\n")
    f.write("void rover_analog(DRIVE drive) {\n  analogWrite(M1_1, drive.right1);\n  analogWrite(M1_2, drive.right2);\n")
    f.write("  analogWrite(M2_1, drive.leght1);\n  analogWrite(M2_2, drive.leght2);\n}\n")
    f.write("void  setup() {\n  Serial.begin(SERIAL_BAUDRATE);\n")
    f.write("  pinMode(M1_1, OUTPUT);\n  pinMode(M1_2, OUTPUT);\n  pinMode(M2_1, OUTPUT);\n  pinMode(M2_2, OUTPUT);\n}\n")
    f.write("void loop() {\n")
    f.write("  DRIVE go;\n  go.right1 = 1;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 1;\n")
    f.close()
    print("工程のセットアップ")

    while True:
        print("")
        print("何をしますか？")
        print("1:滑らかに 発進→停止")
        print("2:急に 発進→停止")
        print("3:滑らかに 右旋回→停止")
        print("4:急に 右旋回→停止")
        print("5:滑らかに 左旋回→停止")
        print("6:急に 左旋回→停止")
        print("7:ランダムに1~6の動きを1つ生成")
        print("8:生成を終了して保存。")
        command = input("数字で入力して下さい。\n")
        if command == "1":
            t = get_t()
            go_smoothly(f_path, t)
            print("")
            print("書き込み完了")
            print("")
        elif command == "2":
            t = get_t()
            go_suddenly(f_path, t)
            print("")
            print("書き込み完了")
            print("")
        elif command == "3":
            t = get_t()
            turn_right_smoothly(f_path, t)
            print("")
            print("書き込み完了")
            print("")
        elif command == "4":
            t = get_t()
            turn_right_suddenly(f_path, t)
            print("")
            print("書き込み完了")
            print("")
        elif command == "5":
            t = get_t()
            turn_leght_smoothly(f_path, t)
            print("")
            print("書き込み完了")
            print("")
        elif command == "6":
            t = get_t()
            turn_right_smoothly(f_path, t)
            print("")
            print("書き込み完了")
            print("")
        elif command == "7":
            print("")
            print("乱数を生成します。")
            num = str(random.randint(1, 6))
            print("ナンバー" + num + "の動きを生成")
            num2 = random.randint(1, 3)
            if num2 == 1:
                t = "1000"
                print("動作時間は" + str(t) + "秒")
            elif num2 == 2:
                t = "3000"
                print("動作時間は" + str(t) + "秒")
            else:
                t = "5000"
                print("動作時間は" + str(t) + "秒")
            print("")
            if num == "1":
                go_smoothly(f_path, t)
            elif num == "2":
                go_suddenly(f_path, t)
            elif num == "3":
                turn_right_smoothly(f_path, t)
            elif num == "4":
                turn_right_suddenly(f_path, t)
            elif num == "5":
                turn_leght_smoothly(f_path, t)
            elif num == "6":
                turn_right_smoothly(f_path, t)
            turn_right_smoothly(f_path, t)
            print("書き込み完了")
            print("")
        elif command == "8":
            print("")
            print("書き込みを終了して保存します。")
            print("")
            f = open(f_path, "a")
            f.write("}\n")
            f.close()
            break
        else:
            print("")
            print("不適切な値が入力されました。")
            print("再度入力して下さい.")
            print("")

if __name__ == "__main__":
    main()





