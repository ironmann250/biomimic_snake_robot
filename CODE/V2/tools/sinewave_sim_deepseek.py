import sys
import numpy as np
from PyQt5 import QtWidgets
import pyqtgraph as pg

class SineWaveControl(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.generate_waveforms()
        
    def initUI(self):
        self.setWindowTitle('Dual Sine Wave Controller')
        self.setGeometry(100, 100, 800, 600)
        
        # Create plot widget
        self.plot_widget = pg.PlotWidget()
        self.plot_widget.setBackground('w')
        self.plot_widget.setLabel('left', 'Amplitude')
        self.plot_widget.setLabel('bottom', 'Time (s)')
        self.plot_widget.addLegend()
        self.plot_widget.showGrid(x=True, y=True)
        
        # Create control elements for Wave 1
        self.wave1_group = QtWidgets.QGroupBox("Wave 1 Controls")
        self.amp1_slider = self.create_slider(0, 10, 1, "Amplitude:")
        self.freq1_slider = self.create_slider(1, 20, 1, "Frequency (Hz):")
        self.phase1_slider = self.create_slider(0, 360, 0, "Phase (deg):")
        
        # Create control elements for Wave 2
        self.wave2_group = QtWidgets.QGroupBox("Wave 2 Controls")
        self.amp2_slider = self.create_slider(0, 10, 1, "Amplitude:")
        self.freq2_slider = self.create_slider(1, 20, 1, "Frequency (Hz):")
        self.phase2_slider = self.create_slider(0, 360, 0, "Phase (deg):")

        # Layout
        wave1_layout = QtWidgets.QVBoxLayout()
        wave1_layout.addWidget(self.amp1_slider['container'])
        wave1_layout.addWidget(self.freq1_slider['container'])
        wave1_layout.addWidget(self.phase1_slider['container'])
        self.wave1_group.setLayout(wave1_layout)

        wave2_layout = QtWidgets.QVBoxLayout()
        wave2_layout.addWidget(self.amp2_slider['container'])
        wave2_layout.addWidget(self.freq2_slider['container'])
        wave2_layout.addWidget(self.phase2_slider['container'])
        self.wave2_group.setLayout(wave2_layout)

        main_layout = QtWidgets.QVBoxLayout()
        main_layout.addWidget(self.plot_widget)
        
        controls_layout = QtWidgets.QHBoxLayout()
        controls_layout.addWidget(self.wave1_group)
        controls_layout.addWidget(self.wave2_group)
        
        main_layout.addLayout(controls_layout)
        self.setLayout(main_layout)

        # Initialize plot curves
        self.curve1 = self.plot_widget.plot(pen=pg.mkPen('r', width=2), name='Wave 1')
        self.curve2 = self.plot_widget.plot(pen=pg.mkPen('b', width=2), name='Wave 2')

    def create_slider(self, min_val, max_val, default, label_text):
        container = QtWidgets.QWidget()
        layout = QtWidgets.QHBoxLayout()
        
        label = QtWidgets.QLabel(label_text)
        value_label = QtWidgets.QLabel(str(default))
        
        slider = QtWidgets.QSlider(QtWidgets.Qt.Horizontal)
        slider.setMinimum(min_val)
        slider.setMaximum(max_val)
        slider.setValue(default)
        
        slider.valueChanged.connect(
            lambda value: value_label.setText(str(value/10 if "Frequency" in label_text else value))
        )
        slider.valueChanged.connect(self.generate_waveforms)
        
        layout.addWidget(label)
        layout.addWidget(slider)
        layout.addWidget(value_label)
        container.setLayout(layout)
        
        return {
            'container': container,
            'slider': slider,
            'value_label': value_label
        }

    def generate_waveforms(self):
        # Time array (0 to 2 seconds with 1000 points)
        t = np.linspace(0, 2, 1000)
        
        # Get parameters for Wave 1
        amp1 = self.amp1_slider['slider'].value()
        freq1 = self.freq1_slider['slider'].value() / 10  # Convert to 0.1-2.0 Hz
        phase1 = np.deg2rad(self.phase1_slider['slider'].value())
        
        # Get parameters for Wave 2
        amp2 = self.amp2_slider['slider'].value()
        freq2 = self.freq2_slider['slider'].value() / 10
        phase2 = np.deg2rad(self.phase2_slider['slider'].value())
        
        # Generate sine waves
        y1 = amp1 * np.sin(2 * np.pi * freq1 * t + phase1)
        y2 = amp2 * np.sin(2 * np.pi * freq2 * t + phase2)
        
        # Update plot data
        self.curve1.setData(t, y1)
        self.curve2.setData(t, y2)
        
        # Auto-scale Y axis
        max_amp = max(np.max(y1), np.max(y2))
        self.plot_widget.setYRange(-max_amp*1.1, max_amp*1.1)

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    main = SineWaveControl()
    main.show()
    sys.exit(app.exec_())
