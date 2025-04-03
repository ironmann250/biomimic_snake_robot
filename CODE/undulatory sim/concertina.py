import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# 1. Dynamic curve implementation
def dynamic_curve(x, gamma, omega_n=1):
    if gamma == 0:
        return np.sin(omega_n * x)
    
    alpha = gamma * omega_n
    beta = gamma * omega_n
    delta = -np.sqrt(1 - gamma**2) / gamma
    
    return np.exp(-alpha * x) * np.sin(beta * x + delta)

# 2. Concertina modules (corrected)
class ConcertinaModules:
    def __init__(self, num_body_modules=4, gamma_max=0.8):
        self.num_body = num_body_modules
        self.gamma_max = gamma_max
        self.x = np.linspace(0, 15, 1000)  # Extended x-range
        self.omega_n = 1
        self.module_gamma = {'head': 0, 'body': [0]*num_body_modules, 'tail': 0}

    def _head_module(self, x, gamma):
        if gamma == 0:
            return np.zeros_like(x)
        
        # Calculate parameters from gamma
        alpha = gamma * self.omega_n
        beta = gamma * self.omega_n
        delta = -np.sqrt(1 - gamma**2) / gamma
        
        # Calculate cutoff position (Eq.6)
        N_c = 2  # Second intersection
        x_head_start = 8  # Starting position for head
        x_cut = x_head_start + (N_c * np.pi - delta)/beta
        
        # Generate and truncate head curve
        curve = dynamic_curve(x - x_head_start, gamma)
        curve[x > x_cut] = 0
        return curve

    def _body_module(self, x, idx, gamma):
        phase = 2 * np.pi * idx / self.num_body
        return (-1)**(idx+1) * dynamic_curve(x - phase, gamma)

    def update_gamma(self, t):
        # Sequential activation with phase offset
        cycle_speed = 0.5
        self.module_gamma['head'] = self.gamma_max * abs(np.sin(cycle_speed * t))
        for i in range(self.num_body):
            phase = 0.3 * i  # Stagger body module activation
            self.module_gamma['body'][i] = self.gamma_max * abs(np.sin(cycle_speed * t - phase))

    def get_full_curve(self, t):
        self.update_gamma(t)
        y = np.zeros_like(self.x)
        
        # Tail module (simple sine wave)
        y += 0.3 * np.sin(self.omega_n * self.x)
        
        # Body modules
        for i in range(self.num_body):
            y += self._body_module(self.x, i, self.module_gamma['body'][i])
        
        # Head module
        y += self._head_module(self.x, self.module_gamma['head'])
        
        return y

# 3. Create animation
fig, ax = plt.subplots(figsize=(12, 4))
modules = ConcertinaModules()
line, = ax.plot([], [], lw=2, color='darkgreen')
ax.set_xlim(0, 15)
ax.set_ylim(-500, 500)
ax.grid(True)

def init():
    line.set_data([], [])
    return line,

def update(frame):
    t = frame * 0.2  # Slower animation
    y = modules.get_full_curve(t)
    line.set_data(modules.x, y)
    return line,

anim = FuncAnimation(fig, update, init_func=init,
                    frames=200, interval=50, blit=True)
plt.show()
