import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Set up the figure and axis
fig, ax = plt.subplots()
ax.set_xlim(0, 2 * np.pi)
ax.set_ylim(-1, 1)
ax.set_xlabel('x')
ax.set_ylabel('y')
line, = ax.plot([], [], lw=2)

def undulation_gen(x, amplitude, t):
    y = 5  # wavelength
    n = 0.52  # coefficient
    vx = 3  # forward velocity
    vw = n/vx
    return amplitude * np.sin(((2 * np.pi)/y) * (x + (vw * t)))

def init():
    line.set_data([], [])
    return (line,)

def update(frame):
    x = np.linspace(0, 2 * np.pi, 201)
    y = undulation_gen(x, 0.5, frame)
    line.set_data(x, y)
    return (line,)

# Create animation with initialization
animation = FuncAnimation(fig, update, 
                        init_func=init,
                        frames=200,
                        interval=50, 
                        blit=True)

plt.show()