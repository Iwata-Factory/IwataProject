# coding:utf8

import os
import random


def go_suddenly(path, times):
    f = open(path, "a")
    f.write("  go.right1 = 0;\n  go.right2 = 1;\n  go.leght1 = 0;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(")
    f.write(times)
    f.write(");\n  go.right1 = 1;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 1;\n  rover_degital(go);\n")
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

def turn_leght_suddenly(path, times):
    f = open(path, "a")
    f.write("  go.right1 = 1;\n  go.right2 = 0;\n  go.leght1 = 0;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(")
    f.write(times)
    f.write(");\n  go.right1 = 1;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 1;\n  rover_degital(go);\n")
    f.write("  delay(2500);\n")
    f.close()


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
    f.write("void  setup() {\n  Serial.begin(SERIAL_BAUDRATE);\n")
    f.write("  pinMode(M1_1, OUTPUT);\n  pinMode(M1_2, OUTPUT);\n  pinMode(M2_1, OUTPUT);\n  pinMode(M2_2, OUTPUT);\n}\n")
    f.write("void loop() {\n")
    f.write("  DRIVE go;\n  go.right1 = 1;\n  go.right2 = 1;\n  go.leght1 = 1;\n  go.leght2 = 1;\n")
    f.close()
    print("工程のセットアップ")

    while True:
        print("")
        print("何をしますか？")
        print("1:滑らかに発進→停止")
        print("2:急に発進→停止")
        print("3:滑らかに右旋回")
        print("4:急に右旋回")
        print("5:滑らかに左旋回")
        print("6:急に左旋回")
        print("7:もう何もしない。")
        command = input("数字で入力して下さい。\n")
        if command == "2":
            t = input("何ミリ秒行いますか？\n")
            go_suddenly(f_path, t)
        elif command == "7":
            break


if __name__ == "__main__":
    main()





