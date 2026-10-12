#!/usr/bin/python

import argparse
import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as sig

f, ax = plt.subplots(2)

fs = 5e3
f0 = 50
Q = 30

b, a = sig.iirnotch(f0 / (fs/2), Q)

f, h = sig.freqz(b, a, fs=fs)


ax[0].plot(f, np.abs(h))
ax[1].plot(f, np.degrees(np.angle(h)))


plt.show()
