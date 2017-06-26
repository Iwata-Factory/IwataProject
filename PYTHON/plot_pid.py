#!/user/env/bin python
# coding: utf-8

import numpy as np
import matplotlib.pyplot as plt


def main():

    f = open('text.txt')
    data = f.read()  # ファイル終端まで全て読んだデータを返す
    f.close()

    lines = data.split('\n')  # 改行で区切る(改行文字そのものは戻り値のデータには含まれない)

    # h = np.array()
    # l = np.array()
    h = []

    for line in lines:
        if line == '':
            continue
        line = float(line)
        h.append(line)

    h2 = np.array(h)
    l = np.arange(len(h))


    plt.plot(l, h2)
    plt.show()

if __name__ == '__main__':
    main()