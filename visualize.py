import sys
import numpy as np
import matplotlib.pyplot as plt

CHUNK = 512  # smaller = smoother plot

plt.ion()
fig, ax = plt.subplots()

x = np.arange(CHUNK)
line, = ax.plot(x, np.zeros(CHUNK))

ax.set_ylim(-1, 1)
ax.set_xlim(0, CHUNK)
ax.set_title("Real-Time Mic Input")
ax.set_xlabel("Samples")
ax.set_ylabel("Amplitude")

buffer = []

for line_in in sys.stdin:
    try:
        val = float(line_in.strip())
        buffer.append(val)

        if len(buffer) >= CHUNK:
            data = np.array(buffer[:CHUNK])
            buffer = buffer[CHUNK:]

            line.set_ydata(data)
            fig.canvas.draw()
            fig.canvas.flush_events()

    except:
        pass