#!/usr/bin/python

import argparse
import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as sig

def filter_mains(fs, x):
    f0 = 50
    Q = 30
    b, a = sig.iirnotch(f0 / (fs/2), Q)
    print(b, a)
    return sig.lfilter(b, a, x)

parser = argparse.ArgumentParser()
parser.add_argument("filename", type=str)
args = parser.parse_args()

archive = np.load(args.filename, allow_pickle=True)

fs, channels, t = archive["fs"], archive["channels"], archive["t"]


v = channels[:, 1]
v_mains = filter_mains(fs, v)
v_lp = v * 0
avg = 0

for i, vs in enumerate(v_mains):
    avg = 0.9 * avg + 0.1 * vs
    v_lp[i] = avg


f, a = plt.subplots(2)

# a[0].plot(t, v)
a[0].plot(t, v_mains)
a[0].plot(t, v_lp)

a[1].plot(t, v_lp)

plt.show()
