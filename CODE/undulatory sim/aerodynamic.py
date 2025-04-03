import numpy as np
import matplotlib.pyplot as plt

def calculate_aerodynamic_forces(blade_length=61.5, rated_wind_speed=11.4, rated_rpm=12.1, air_density=1.225):
    """
    Calculate aerodynamic forces along the blade span using BEM theory
    
    Parameters:
    - blade_length: total blade length in meters (default: 61.5m)
    - rated_wind_speed: rated wind speed in m/s (default: 11.4m/s)
    - rated_rpm: rotor speed in rpm (default: 12.1rpm)
    - air_density: air density in kg/m³ (default: 1.225)
    
    Returns:
    - span_positions: span positions in meters
    - F_n: normal forces (N/m)
    - F_t: tangential forces (N/m)
    """
    
    # Convert rpm to rad/s
    omega = rated_rpm * (2*np.pi)/60
    
    # Create span positions in meters (starting at 15% of blade length to avoid hub effects)
    span_positions = np.linspace(0.15 * blade_length, blade_length, 50)
    
    # Normalized positions for coefficient calculations
    norm_positions = span_positions / blade_length
    
    # Airfoil coefficients - MODIFY THESE BASED ON YOUR AIRFOIL DATA
    lift_coeff = 0.8 + 0.4*norm_positions - 0.6*norm_positions**2
    drag_coeff = 0.02 + 0.01*norm_positions
    
    # Chord length distribution in meters - MODIFY FOR YOUR BLADE
    chord_lengths = 3.5 * (1 - norm_positions) + 1.0  # Linear taper
    
    # Local speed ratio
    local_speed_ratio = omega * span_positions / rated_wind_speed
    
    # Flow angle calculation
    flow_angle = np.arctan2(rated_wind_speed, omega * span_positions)
    
    # Angle of attack
    pitch_angle = np.deg2rad(0)  # MODIFY FOR DIFFERENT PITCH ANGLES
    alpha = flow_angle - pitch_angle
    
    # Normal and tangential force coefficients
    Cn = lift_coeff * np.cos(alpha) + drag_coeff * np.sin(alpha)
    Ct = lift_coeff * np.sin(alpha) - drag_coeff * np.cos(alpha)
    
    # Dynamic pressure
    q = 0.5 * air_density * rated_wind_speed**2
    
    # Normal and tangential forces per unit length (N/m)
    F_n = q * chord_lengths * Cn
    F_t = q * chord_lengths * Ct
    
    return span_positions, F_n, F_t

def plot_aerodynamic_forces(span_positions, F_n, F_t):
    """Plot the aerodynamic forces with proper units"""
    plt.figure(figsize=(10, 6))
    
    # Plot normal force (N/m)
    plt.plot(span_positions, F_n, 'b-', linewidth=2, label='Normal Force (F$_n$)')
    
    # Plot tangential force (N/m)
    plt.plot(span_positions, F_t, 'r--', linewidth=2, label='Tangential Force (F$_t$)')
    
    # Formatting
    plt.xlabel('Blade Span Position (m)')
    plt.ylabel('Aerodynamic Force (N/m)')
    plt.title('Aerodynamic Load Distribution Along Blade Span\n(Rated Wind Speed = 11.4 m/s)')
    plt.grid(True, which='both', linestyle='--', alpha=0.6)
    plt.legend(loc='upper right')
    
    # Set axis limits
    plt.xlim(0, max(span_positions))
    plt.ylim(0, max(max(F_n), max(F_t)) * 1.1)
    
    # Add vertical line at typical max load position
    max_load_pos = span_positions[np.argmax(F_n)]
    plt.axvline(x=max_load_pos, color='gray', linestyle=':', alpha=0.5)
    plt.text(max_load_pos+2, max(F_n)*0.8, f'Max load at {max_load_pos:.1f}m', 
             rotation=90, va='center', ha='left')
    
    # Add text box with parameters
    params = f'Blade Length: {max(span_positions):.1f}m\n' \
             f'Rated Wind Speed: 11.4 m/s\n' \
             f'Rotor Speed: 12.1 rpm'
    plt.gca().text(0.02, 0.98, params, transform=plt.gca().transAxes,
                   verticalalignment='top', bbox=dict(facecolor='white', alpha=0.8))
    
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # Calculate with default parameters (NREL 5MW)
    #span_pos, F_n, F_t = calculate_aerodynamic_forces()
    span_pos, F_n, F_t = calculate_aerodynamic_forces(
        blade_length=61.5,  # New blade length
        rated_wind_speed=11.4,
        rated_rpm=12.1,
        air_density=1.225
    )
    # Plot results
    plot_aerodynamic_forces(span_pos, F_n, F_t)
    
    # Example of how to modify parameters:
    # span_pos, F_n, F_t = calculate_aerodynamic_forces(
    #     blade_length=61.5,  # New blade length
    #     rated_wind_speed=11.4,
    #     rated_rpm=12.1,
    #     air_density=1.225
    # )
