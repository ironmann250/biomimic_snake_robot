import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

def dynamic_curve(x, gamma, omega_n=1):
    """Paper's equation (1) with continuity safeguards"""
    if gamma <= 0 or gamma >= 1:
        return np.zeros_like(x)
    
    alpha = gamma * omega_n
    beta = gamma * omega_n
    delta = -np.sqrt(1 - gamma**2) / gamma
    
    # Smooth falloff at boundaries
    x_clipped = np.clip(x, 0, 4*np.pi)  # Limit exponential growth
    return np.exp(-alpha * x_clipped) * np.sin(beta * x_clipped + delta)

class ConcertinaSystem:
    def __init__(self, num_segments=5):
        # Paper parameters
        self.omega_n = 1.0
        self.gamma_max = 0.75
        self.N_c = 2
        self.segment_length = 2*np.pi
        
        # Create overlapping segments
        self.segments = []
        self._create_segments(num_segments)
        
        # High-resolution coordinates
        self.x = np.linspace(0, num_segments*self.segment_length, 3000)
        self.y = np.zeros_like(self.x)

    def _create_segments(self, num_segments):
        """Create overlapping segments per paper's module definitions"""
        # Tail segment (always active base wave)
        self.segments.append({
            'type': 'tail',
            'gamma': 0,
            'x_start': 0,
            'sign': 1,
            'phase': 0
        })
        
        # Body segments with alternating signs and phase offsets
        for i in range(1, num_segments-1):
            self.segments.append({
                'type': 'body',
                'gamma': 0,
                'x_start': max(0, (i-0.5)*self.segment_length),  # Overlap
                'sign': (-1)**i,
                'phase': i*0.4*np.pi
            })
        
        # Head segment
        self.segments.append({
            'type': 'head',
            'gamma': 0,
            'x_start': (num_segments-1.5)*self.segment_length,
            'sign': 1,
            'phase': 0
        })

    def _update_gammas(self, t):
        """Smooth wave of activation with persistence"""
        for seg in self.segments:
            # Progressive activation wave
            phase = 2*np.pi*(0.15*t - seg['phase'])
            seg['gamma'] = self.gamma_max * (1 - np.cos(phase))/2
            
            # Maintain minimum base activation
            seg['gamma'] = np.clip(seg['gamma'], 0.1*self.gamma_max, self.gamma_max)

    def _calculate_head_cutoff(self, x_start, gamma):
        """Smooth head truncation with fading"""
        if gamma == 0:
            return np.inf
        
        alpha = gamma * self.omega_n
        beta = gamma * self.omega_n
        delta = -np.sqrt(1 - gamma**2)/gamma
        cutoff = x_start + (self.N_c*np.pi - delta)/beta
        
        # Add fade-out region
        return cutoff + 0.5*self.segment_length

    def get_smooth_body(self, t):
        """Combine segments with smooth blending"""
        self._update_gammas(t)
        self.y.fill(0)
        
        for seg in self.segments:
            x_local = self.x - seg['x_start']
            mask = x_local >= 0
            
            if seg['type'] == 'tail':
                # Base undulation (equation 2)
                contribution = 0.4 * np.sin(self.omega_n * x_local[mask])
            else:
                # Dynamic curve contribution
                contribution = seg['sign'] * dynamic_curve(x_local[mask], seg['gamma'])
                
                if seg['type'] == 'head':
                    # Head truncation with smooth fade
                    cutoff = self._calculate_head_cutoff(seg['x_start'], seg['gamma'])
                    fade_mask = self.x[mask] > cutoff - self.segment_length
                    fade = 1 - np.clip((self.x[mask][fade_mask] - (cutoff - self.segment_length)) / 
                                     self.segment_length, 0, 1)
                    contribution[fade_mask] *= fade
            
            # Blend contributions using sigmoid function
            blend = 1 / (1 + np.exp(-5*(x_local[mask]/self.segment_length - 0.5)))
            self.y[mask] += blend * contribution + (1 - blend) * self.y[mask]
        
        return self.y

# Create and run animation
system = ConcertinaSystem(num_segments=6)
fig, ax = plt.subplots(figsize=(14, 4))
line, = ax.plot([], [], lw=3, color='darkgreen')
ax.set_xlim(0, system.x[-1])
ax.set_ylim(-1.5, 1.5)
ax.grid(True)

def init():
    line.set_data([], [])
    return line,

def update(frame):
    t = frame * 0.2
    y = system.get_smooth_body(t)
    line.set_data(system.x, y)
    return line,

anim = FuncAnimation(fig, update, init_func=init,
                    frames=200, interval=50, blit=True)
plt.show()
