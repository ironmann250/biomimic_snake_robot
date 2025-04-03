import sys
import numpy as np
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                            QHBoxLayout, QGroupBox, QFormLayout, QDoubleSpinBox,
                            QPushButton)
from PyQt5.QtCore import Qt, QTimer
import pyqtgraph as pg

class PIDVisualizer(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setup_ui()
        self.setup_plot()
        self.setup_pid()
        self.setup_simulation()
        self.apply_dark_theme()

    def setup_ui(self):
        self.setWindowTitle("Fast PID Response Visualizer")
        self.setGeometry(100, 100, 1200, 800)
        
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        layout = QHBoxLayout(main_widget)

        # Control Panel
        control_panel = QWidget()
        control_panel.setFixedWidth(300)
        control_layout = QVBoxLayout(control_panel)

        # PID Parameters
        pid_group = QGroupBox("PID Parameters")
        form = QFormLayout()
        
        self.kp_spin = QDoubleSpinBox()
        self.kp_spin.setRange(0, 1000)  # Allow higher Kp for fast systems
        self.kp_spin.setValue(100.0)
        self.kp_spin.setDecimals(3)
        
        self.ki_spin = QDoubleSpinBox()
        self.ki_spin.setRange(0, 1000)
        self.ki_spin.setValue(50.0)
        self.ki_spin.setDecimals(3)
        
        self.kd_spin = QDoubleSpinBox()
        self.kd_spin.setRange(0, 1000)
        self.kd_spin.setValue(10.0)
        self.kd_spin.setDecimals(3)
        
        self.setpoint_spin = QDoubleSpinBox()
        self.setpoint_spin.setRange(0, 100)
        self.setpoint_spin.setValue(50.0)
        self.setpoint_spin.setDecimals(3)

        form.addRow("Proportional (Kp):", self.kp_spin)
        form.addRow("Integral (Ki):", self.ki_spin)
        form.addRow("Derivative (Kd):", self.kd_spin)
        form.addRow("Setpoint:", self.setpoint_spin)
        
        pid_group.setLayout(form)
        control_layout.addWidget(pid_group)

        # System Parameters
        sys_group = QGroupBox("System Parameters")
        sys_form = QFormLayout()
        
        self.tau_spin = QDoubleSpinBox()
        self.tau_spin.setRange(0.001, 0.1)  # 1ms to 100ms
        self.tau_spin.setValue(0.01)        # 10ms time constant
        self.tau_spin.setDecimals(3)
        
        self.dt_spin = QDoubleSpinBox()
        self.dt_spin.setRange(0.001, 0.01)  # 1ms to 10ms steps
        self.dt_spin.setValue(0.001)        # 1ms time step
        self.dt_spin.setDecimals(3)
        
        sys_form.addRow("Time Constant (τ):", self.tau_spin)
        sys_form.addRow("Time Step (dt):", self.dt_spin)
        sys_group.setLayout(sys_form)
        control_layout.addWidget(sys_group)

        # Control Buttons
        self.run_button = QPushButton("Run Simulation")
        self.run_button.clicked.connect(self.start_simulation)
        control_layout.addWidget(self.run_button)

        layout.addWidget(control_panel)
        
        # Plot Area
        self.plot_widget = pg.PlotWidget()
        layout.addWidget(self.plot_widget, 1)

    def setup_plot(self):
        self.plot_widget.setBackground("#1E1E1E")
        self.plot_widget.showGrid(x=True, y=True, alpha=0.3)
        self.plot_widget.setLabel('left', 'Process Variable')
        self.plot_widget.setLabel('bottom', 'Time (s)')
        self.plot_widget.setXRange(0, 0.5)  # Show 500ms window
        
        self.response_curve = self.plot_widget.plot(pen=pg.mkPen('#3498DB', width=2))
        self.setpoint_line = self.plot_widget.plot(pen=pg.mkPen('#2ECC71', width=2, style=Qt.DashLine))

    def setup_pid(self):
        self.integral = 0
        self.prev_error = 0
        self.prev_time = 0

    def setup_simulation(self):
        self.sim_timer = QTimer()
        self.sim_timer.timeout.connect(self.update_simulation)
        self.time = []
        self.process_var = []
        self.setpoints = []
        self.sim_time = 0
        self.running = False

    def apply_dark_theme(self):
        self.setStyleSheet("""
            QWidget {
                background-color: #2D2D2D;
                color: #FFFFFF;
                font-family: Segoe UI;
            }
            QGroupBox {
                border: 1px solid #3498DB;
                border-radius: 5px;
                margin-top: 1ex;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                color: #3498DB;
            }
            QDoubleSpinBox {
                background-color: #404040;
                border: 1px solid #606060;
                padding: 5px;
            }
            QPushButton {
                background-color: #3498DB;
                color: white;
                border: none;
                padding: 8px;
                border-radius: 4px;
            }
            QPushButton:hover {
                background-color: #2980B9;
            }
        """)

    def start_simulation(self):
        if not self.running:
            self.reset_simulation()
            self.sim_timer.start(1)  # Fastest possible timer (≈1ms)
            self.run_button.setText("Stop Simulation")
            self.running = True
        else:
            self.sim_timer.stop()
            self.run_button.setText("Run Simulation")
            self.running = False

    def reset_simulation(self):
        self.time = []
        self.process_var = []
        self.setpoints = []
        self.sim_time = 0
        self.integral = 0
        self.prev_error = 0
        self.prev_time = 0
        self.response_curve.setData([], [])
        self.setpoint_line.setData([], [])

    def update_simulation(self):
        dt = self.dt_spin.value()
        tau = self.tau_spin.value()
        setpoint = self.setpoint_spin.value()
        
        # Current process variable
        pv = self.process_var[-1] if self.process_var else 0
        
        # PID calculations
        error = setpoint - pv
        self.integral += error * dt
        derivative = (error - self.prev_error) / dt if dt > 0 else 0
        
        # PID output (clamped to ±1000 for stability)
        output = np.clip(
            (self.kp_spin.value() * error +
             self.ki_spin.value() * self.integral +
             self.kd_spin.value() * derivative),
            -1000, 1000
        )
        
        # First-order system dynamics
        pv += (output - pv) * dt / tau
        
        # Store data
        self.time.append(self.sim_time)
        self.process_var.append(pv)
        self.setpoints.append(setpoint)
        
        # Update plot
        self.response_curve.setData(self.time, self.process_var)
        self.setpoint_line.setData(self.time, self.setpoints)
        
        # Scroll viewport
        if self.sim_time > 0.5:
            self.plot_widget.setXRange(self.sim_time - 0.5, self.sim_time)
        
        # Increment simulation time
        self.sim_time += dt
        self.prev_error = error
        
        # Stop after 1 second
        if self.sim_time >= 1.0:
            self.start_simulation()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = PIDVisualizer()
    window.show()
    sys.exit(app.exec_())