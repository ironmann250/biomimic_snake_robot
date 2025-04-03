import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
plt.switch_backend('TkAgg')  # Switch to TkAgg backend for better stability

def undulation_gen(x, amplitude, t):
    y = 5  # wavelength 
    n = 0.52  # coefficient
    vx = 3  # forward velocity
    vw = n/vx
    return amplitude * np.sin(((2*np.pi)/y)*(x + (vw*t)))
def current_wave_gen(x,amplitude, t):
    f=0.5
    phase=0
    return amplitude*np.sin(2*np.pi*f*t+phase)
# Set up the figure and axis
fig = plt.figure()
ax = plt.axes(xlim=(0, 2*np.pi), ylim=(-1, 1))
line, = ax.plot([], [], lw=2)
ax.set_xlabel('x')
ax.set_ylabel('y')
ax.grid(True)

def init():
    line.set_data(np.array([]), np.array([]))
    return (line,)

def update(frame):
    x = np.array(np.linspace(0, 2*np.pi, 201))
    y = np.array(undulation_gen(x, 0.5, frame))
    line.set_data(x, y)
    return (line,)

# Create animation
anim = FuncAnimation(fig, update,
                    init_func=init,
                    frames=np.arange(0, 200),
                    interval=50,
                    blit=True)

plt.show()