import sys
import numpy as np
import matplotlib.pyplot as plt

plt.ion()

fig, axs = plt.subplots(4, 1)

while True:
    line = sys.stdin.readline()

    if not line:
        break

    parts = line.split()
    label = parts[0]
    data = np.array(parts[1:], dtype=float)

    if label == "RAW":
        axs[0].clear()
        axs[0].plot(data)
        axs[0].set_title("Raw Mic Signal")

    elif label == "CENTERED":
        axs[1].clear()
        axs[1].plot(data)
        axs[1].set_title("Mean Removed")

    elif label == "WINDOWED":
        axs[2].clear()
        axs[2].plot(data)
        axs[2].set_title("Hann Window Applied")

    elif label == "FFT":
        axs[3].clear()
        axs[3].plot(data)
        axs[3].set_title("FFT Magnitude")

        plt.pause(0.01)