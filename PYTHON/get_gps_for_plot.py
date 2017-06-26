#!/user/bin/env python
# coding: utf-8


def main():
    f = open('gpslog.txt')
    data = f.read()  # ファイル終端まで全て読んだデータを返す
    f.close()

    # 区切り文字はmacとwindowsで違うかも
    lines = data.split('\n')  # 改行で区切る(改行文字そのものは戻り値のデータには含まれない)

    lat = []
    lng = []

    for i, line in enumerate(lines):
        if line == '*':
            lat.append(lines[i + 2])
            lng.append(lines[i + 3])

    for j in range(0, len(lat)):
        print(lat[j] + " " + lng[j])

if __name__ == '__main__':
    main()