#!/usr/bin/python

import argparse
import numpy as np
import matplotlib.pyplot as plt


parser = argparse.ArgumentParser()
parser.add_argument("filename", type=str)
args = parser.parse_args()

archive = np.load(args.filename, allow_pickle=True)

fs, channels, t = archive["fs"], archive["channels"], archive["t"]

f, a = plt.subplots(2)

a[0].plot(t, channels[:, 0])
a[1].plot(t, channels[:, 1])

plt.show()
