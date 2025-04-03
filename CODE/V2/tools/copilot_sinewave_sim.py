import sys
import numpy as np
from PyQt5 import QtWidgets, QtCore
import pyqtgraph as pg

class SineWavePlotter(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        # Initialize sine wave parameters
        self.amplitude1 = 1.0
        self.frequency1 = 1.0
        self.phase1 = 0.0

        self.amplitude2 = 1.0
        self.frequency2 = 1.0
        self.phase2 = 0.0

        self.time = 0.0  # Initialize time parameter
        self.time_step = 0.05  # Time increment for animation

        self.legend = None  # Initialize legend variable

        # Initialize fixed-size buffers for Lissajous data
        self.max_points = 1000  # Adjust this value for performance
        self.y1_data = np.zeros(self.max_points)
        self.y2_data = np.zeros(self.max_points)
        self.buffer_index = 0

        self.setup_ui()
        self.start_timer()

    def setup_ui(self):
        # Set up the window layout
        self.setWindowTitle("Optimized Lissajous Curve Plotter with Erase Button")
        main_layout = QtWidgets.QVBoxLayout()

        # Create the plot widgets
        self.wave_plot_widget = pg.PlotWidget(title="Sine Waves")
        self.wave_plot_widget.showGrid(x=True, y=True)

        self.lissajous_plot_widget = pg.PlotWidget(title="Lissajous Curve")
        self.lissajous_plot_widget.showGrid(x=True, y=True)
        self.lissajous_plot_widget.setAspectLocked(True)

        # Add plot widgets to the layout
        main_layout.addWidget(self.wave_plot_widget)
        main_layout.addWidget(self.lissajous_plot_widget)

        # Create control panels
        controls_layout = QtWidgets.QHBoxLayout()

        # Controls for Sine Wave 1
        controls1 = self.create_controls("Sine Wave 1", self.amplitude1, self.frequency1, self.phase1)
        controls_layout.addLayout(controls1)

        # Controls for Sine Wave 2
        controls2 = self.create_controls("Sine Wave 2", self.amplitude2, self.frequency2, self.phase2)
        controls_layout.addLayout(controls2)

        # Add erase button
        erase_button = QtWidgets.QPushButton("Erase Lissajous Figure")
        erase_button.clicked.connect(self.clear_lissajous_plot)
        controls_layout.addWidget(erase_button)

        main_layout.addLayout(controls_layout)
        self.setLayout(main_layout)

        # Initialize plots
        self.init_plots()

    def create_controls(self, title, amplitude, frequency, phase):
        # Create a vertical layout for controls
        vbox = QtWidgets.QVBoxLayout()
        group_box = QtWidgets.QGroupBox(title)
        group_layout = QtWidgets.QFormLayout()

        # Amplitude control
        amplitude_spin = QtWidgets.QDoubleSpinBox()
        amplitude_spin.setRange(0, 10)
        amplitude_spin.setSingleStep(0.1)
        amplitude_spin.setValue(amplitude)
        amplitude_spin.valueChanged.connect(self.reset_lissajous_data)
        group_layout.addRow("Amplitude:", amplitude_spin)

        # Frequency control
        frequency_spin = QtWidgets.QDoubleSpinBox()
        frequency_spin.setRange(0.1, 10)
        frequency_spin.setSingleStep(0.1)
        frequency_spin.setValue(frequency)
        frequency_spin.valueChanged.connect(self.reset_lissajous_data)
        group_layout.addRow("Frequency:", frequency_spin)

        # Phase control
        phase_spin = QtWidgets.QDoubleSpinBox()
        phase_spin.setRange(0, 360)
        phase_spin.setSingleStep(1)
        phase_spin.setValue(phase)
        phase_spin.valueChanged.connect(self.reset_lissajous_data)
        group_layout.addRow("Phase (°):", phase_spin)

        # Store the controls for later access
        if "1" in title:
            self.amp1_spin = amplitude_spin
            self.freq1_spin = frequency_spin
            self.phase1_spin = phase_spin
        else:
            self.amp2_spin = amplitude_spin
            self.freq2_spin = frequency_spin
            self.phase2_spin = phase_spin

        group_box.setLayout(group_layout)
        vbox.addWidget(group_box)

        return vbox

    def reset_lissajous_data(self):
        # Update parameters from controls
        self.amplitude1 = self.amp1_spin.value()
        self.frequency1 = self.freq1_spin.value()
        self.phase1 = self.phase1_spin.value()

        self.amplitude2 = self.amp2_spin.value()
        self.frequency2 = self.freq2_spin.value()
        self.phase2 = self.phase2_spin.value()

        # Clear Lissajous data
        self.y1_data.fill(0)
        self.y2_data.fill(0)
        self.buffer_index = 0

        # Clear the Lissajous plot
        self.clear_lissajous_plot()

    def clear_lissajous_plot(self):
        self.lissajous_curve.setData([], [])
        self.y1_data.fill(0)
        self.y2_data.fill(0)
        self.buffer_index = 0

    def start_timer(self):
        # Set up a timer for animation
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(50)  # Update every 50 milliseconds

    def init_plots(self):
        # Initialize the sine wave plots
        self.wave1_plot = self.wave_plot_widget.plot(pen=pg.mkPen('r', width=2), name='Sine Wave 1')
        self.wave2_plot = self.wave_plot_widget.plot(pen=pg.mkPen('b', width=2), name='Sine Wave 2')

        # Initialize the Lissajous plot
        self.lissajous_curve = self.lissajous_plot_widget.plot(pen=pg.mkPen('g', width=1))

        # Add legend if not already added
        if self.legend is None:
            self.legend = self.wave_plot_widget.addLegend()

    def update_plot(self):
        # Update time parameter
        self.time += self.time_step

        # Generate sine wave data
        t_wave = np.linspace(0, 2 * np.pi, 1000) + self.time
        y1_wave = self.amplitude1 * np.sin(self.frequency1 * t_wave + np.radians(self.phase1))
        y2_wave = self.amplitude2 * np.sin(self.frequency2 * t_wave + np.radians(self.phase2))

        # Update sine wave plots
        self.wave1_plot.setData(t_wave, y1_wave)
        self.wave2_plot.setData(t_wave, y2_wave)

        # Update Lissajous data using circular buffer
        t = self.time
        y1_new = self.amplitude1 * np.sin(self.frequency1 * t + np.radians(self.phase1))
        y2_new = self.amplitude2 * np.sin(self.frequency2 * t + np.radians(self.phase2))

        self.y1_data[self.buffer_index % self.max_points] = y1_new
        self.y2_data[self.buffer_index % self.max_points] = y2_new
        self.buffer_index += 1

        # Update Lissajous plot
        if self.buffer_index < self.max_points:
            # Not yet filled buffer
            self.lissajous_curve.setData(self.y1_data[:self.buffer_index], self.y2_data[:self.buffer_index])
        else:
            # Buffer filled, show all data
            self.lissajous_curve.setData(self.y1_data, self.y2_data)

        # Set axis labels for the sine wave plot
        self.wave_plot_widget.setLabel('left', 'Amplitude')
        self.wave_plot_widget.setLabel('bottom', 'Time')

        # Set axis labels for the Lissajous plot
        self.lissajous_plot_widget.setLabel('left', 'Sine Wave 2 Amplitude')
        self.lissajous_plot_widget.setLabel('bottom', 'Sine Wave 1 Amplitude')

    def closeEvent(self, event):
        # Stop the timer when the window is closed
        self.timer.stop()
        event.accept()

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    window = SineWavePlotter()
    window.show()
    sys.exit(app.exec_())
