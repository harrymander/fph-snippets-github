#!/usr/bin/python

import serial
import time
import numpy as np
import matplotlib.pyplot as plt


samples = []

s = serial.Serial("/dev/ttyACM0")
s.write(' '.encode('ascii'))


end = time.time() + 10

while time.time() < end:
    line = s.readline().decode("ascii")
    samples.append([ int(x) for x in line.split(',') ])

s.write(' '.encode("ascii"))

s.close()


fs = 5e3
Vref = 4.096

channels = np.array(samples) * Vref / (2**16)
t = np.arange(channels.shape[0]) / fs

# f, a = plt.subplots(2)

# a[0].plot(t, samples[:, 0])
# a[1].plot(t, samples[:, 1])

# plt.show()

filename = time.strftime("%Y-%m-%d_%H-%M-%S_capture.npz")

np.savez(filename, fs=fs, channels=channels, t=t)

