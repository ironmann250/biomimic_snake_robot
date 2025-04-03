import sys
import csv
import serial
from serial.tools import list_ports
from functools import partial
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                            QSlider, QLabel, QHBoxLayout, QGroupBox, QMessageBox,
                            QPushButton, QComboBox, QFileDialog)
from PyQt5.QtCore import QThread, pyqtSignal, pyqtSlot, Qt, QTimer, QElapsedTimer
from PyQt5.QtGui import QFont
import pyqtgraph as pg
import numpy as np
from datetime import datetime

# Configuration
DEFAULT_PORT = 'COM6'
BUFFER_SIZE = 500
SEND_DELAY_MS = 500
SETTLING_THRESHOLD = 6
DEBUG = False

# Parameter ranges
PARAM_RANGES = {
    'tar': (-41720, 41720),
    'kp': (0, 15),
    'ki': (0, 40),
    'kd': (-1, 15.0)
}

# Dark Theme
DARK_PALETTE = {
    'background': "#2D2D2D",
    'text': "#FFFFFF",
    'accent': "#3498DB",
    'secondary': "#2ECC71",
    'highlight': "#F1C40F",
    'warning': "#E74C3C",
    'plot_bg': "#1E1E1E",
    'slider_handle': "#000000"
}

STYLE_SHEET = f"""
    QWidget {{
        background-color: {DARK_PALETTE['background']};
        color: {DARK_PALETTE['text']};
        font-family: Segoe UI;
    }}
    QGroupBox {{
        border: 2px solid {DARK_PALETTE['accent']};
        border-radius: 5px;
        margin-top: 1ex;
        padding-top: 12px;
    }}
    QGroupBox::title {{
        subcontrol-origin: margin;
        left: 10px;
        color: {DARK_PALETTE['accent']};
    }}
    QSlider::groove:vertical {{
        background: {DARK_PALETTE['plot_bg']};
        width: 12px;
        border-radius: 6px;
    }}
    QSlider::handle:vertical {{
        background: {DARK_PALETTE['slider_handle']};
        border: 2px solid {DARK_PALETTE['accent']};
        height: 24px;
        width: 24px;
        margin: -8px -8px;
        border-radius: 12px;
    }}
    QPushButton {{
        background-color: {DARK_PALETTE['accent']};
        color: {DARK_PALETTE['text']};
        border: none;
        padding: 8px 16px;
        border-radius: 4px;
        font-size: 14px;
    }}
    QPushButton:hover {{
        background-color: #2980b9;
    }}
    QComboBox {{
        background: {DARK_PALETTE['plot_bg']};
        color: {DARK_PALETTE['text']};
        padding: 4px;
        min-width: 120px;
    }}
"""

class SerialWorker(QThread):
    data_received = pyqtSignal(list)
    error_occurred = pyqtSignal(str)

    def __init__(self, port, baud_rate):
        super().__init__()
        self.port = port
        self.baud_rate = baud_rate
        self.ser = None
        self.running = False

    def run(self):
        self.running = True
        try:
            self.ser = serial.Serial(self.port, self.baud_rate, timeout=0.1)
            while self.running:
                if self.ser.in_waiting > 0:
                    raw_line = self.ser.readline()
                    try:
                        line = raw_line.decode().strip()
                        if DEBUG:
                            print(f"Received: {line}")
                        if line:
                            parts = list(map(float, line.split('\t')))
                            if len(parts) == 12:
                                self.data_received.emit(parts)
                    except Exception as e:
                        continue
        except Exception as e:
            self.error_occurred.emit(str(e))
        finally:
            if self.ser and self.ser.is_open:
                self.ser.close()

    def send_command(self, command):
        if self.ser and self.running:
            try:
                self.ser.write(f"{command}\n".encode())
                if DEBUG:
                    print(f"Sent: {command}")
            except Exception as e:
                self.error_occurred.emit(str(e))

    def stop(self):
        self.running = False
        self.wait()

class MotorControlPanel(QWidget):
    def __init__(self, motor_id, color):
        super().__init__()
        self.motor_id = motor_id
        self.color = color
        self.slider_values = {'tar': 0, 'kp': 0, 'ki': 0, 'kd': 0}
        self.slider_timers = {}
        self.settling_timer = QElapsedTimer()
        self.settling_time_ms = None
        self.current_target = None
        self.sliders_initialized = False
        
        self.init_ui()
        self.init_slider_timers()

    def init_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(15, 15, 15, 15)
        
        # Title
        title = QLabel(f"MOTOR {self.motor_id}")
        title.setStyleSheet(f"font-size: 24px; color: {self.color};")
        layout.addWidget(title, alignment=Qt.AlignCenter)

        # Control Sliders
        sliders_group = QGroupBox("PID CONTROLS")
        sliders_layout = QHBoxLayout()
        
        slider_params = [
            ('tar', 'Target', *PARAM_RANGES['tar'], self.color),
            ('kp', 'Kp', *PARAM_RANGES['kp'], DARK_PALETTE['text']),
            ('ki', 'Ki', *PARAM_RANGES['ki'], DARK_PALETTE['text']),
            ('kd', 'Kd', *PARAM_RANGES['kd'], DARK_PALETTE['text'])
        ]

        self.sliders = {}
        for param, label, min_val, max_val, color in slider_params:
            group = QWidget()
            vbox = QVBoxLayout(group)
            vbox.setAlignment(Qt.AlignCenter)
            
            param_label = QLabel(label)
            param_label.setStyleSheet(f"color: {color}; font-size: 16px;")
            
            value_label = QLabel("0.00")
            value_label.setStyleSheet(f"color: {color}; font-size: 14px;")
            
            slider = QSlider(Qt.Vertical)
            slider.setRange(0, 1000)
            slider.valueChanged.connect(partial(
                self.on_slider_change, 
                param, min_val, max_val, value_label
            ))
            slider.setStyleSheet(f"""
                QSlider::handle:vertical {{
                    background: {color};
                    border: 2px solid {DARK_PALETTE['accent']};
                }}
            """)
            
            vbox.addWidget(param_label)
            vbox.addWidget(value_label)
            vbox.addWidget(slider)
            sliders_layout.addWidget(group)
            self.sliders[param] = (value_label, slider)

        sliders_group.setLayout(sliders_layout)
        layout.addWidget(sliders_group)

        # Status Display
        status_group = QGroupBox("CURRENT STATUS")
        status_layout = QVBoxLayout()
        
        self.labels = {
            'setpoint': QLabel("Setpoint: 0"),
            'position': QLabel("Position: 0"),
            'output': QLabel("Output: 0.0%"),
            'kp': QLabel("Kp: 0.00"),
            'ki': QLabel("Ki: 0.00"),
            'kd': QLabel("Kd: 0.00"),
            'settling': QLabel("Settling Time: --")
        }
        
        for lbl in self.labels.values():
            lbl.setStyleSheet(f"font-size: 14px; padding: 8px; color: {self.color};")
            status_layout.addWidget(lbl)
            
        status_group.setLayout(status_layout)
        layout.addWidget(status_group)

    def init_slider_timers(self):
        for param in self.slider_values:
            timer = QTimer()
            timer.setSingleShot(True)
            timer.timeout.connect(partial(self.send_param, param))
            self.slider_timers[param] = timer

    def on_slider_change(self, param, min_val, max_val, value_label, value):
        mapped_value = min_val + (max_val - min_val) * (value / 1000)
        self.slider_values[param] = mapped_value
        value_label.setText(f"{mapped_value:.0f}" if param == 'tar' else f"{mapped_value:.2f}")
        self.slider_timers[param].start(SEND_DELAY_MS)

    def send_param(self, param):
        value = self.slider_values[param]
        command = f"{param}{self.motor_id}={value:.4f}"
        self.window().serial_worker.send_command(command)

    def update_from_data(self, sp, pos, out, kp, ki, kd):
        self.labels['setpoint'].setText(f"Setpoint: {sp:.0f}")
        self.labels['position'].setText(f"Position: {pos:.0f}")
        self.labels['output'].setText(f"Output: {out:.1f}%")
        self.labels['kp'].setText(f"Kp: {kp:.2f}")
        self.labels['ki'].setText(f"Ki: {ki:.2f}")
        self.labels['kd'].setText(f"Kd: {kd:.2f}")

        if not self.sliders_initialized:
            self.initialize_sliders(sp, kp, ki, kd)
            self.sliders_initialized = True

        self.update_settling_time(sp, pos)

    def initialize_sliders(self, sp, kp, ki, kd):
        params = {
            'tar': (sp, PARAM_RANGES['tar']),
            'kp': (kp, PARAM_RANGES['kp']),
            'ki': (ki, PARAM_RANGES['ki']),
            'kd': (kd, PARAM_RANGES['kd'])
        }
        
        for param, (value, (min_val, max_val)) in params.items():
            value_label, slider = self.sliders[param]
            slider_value = int(((value - min_val) / (max_val - min_val)) * 1000)
            
            slider.blockSignals(True)
            slider.setValue(slider_value)
            slider.blockSignals(False)
            
            formatted = f"{value:.0f}" if param == 'tar' else f"{value:.2f}"
            value_label.setText(formatted)
            self.slider_values[param] = value

    def update_settling_time(self, new_sp, current_pos):
        if new_sp != self.current_target:
            self.current_target = new_sp
            self.settling_time_ms = None
            self.settling_timer.start()
            self.labels['settling'].setText("Settling Time: --")

        if self.settling_timer.isValid():
            if abs(current_pos - self.current_target) <= SETTLING_THRESHOLD:
                if not self.settling_time_ms:
                    self.settling_time_ms = self.settling_timer.elapsed()
                    self.labels['settling'].setText(f"Settling Time: {self.settling_time_ms} ms")
                    self.settling_timer.invalidate()
            else:
                self.settling_time_ms = None

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.serial_worker = None
        self.paused = False
        self.plot_data = {
            '1': {'setpoint': np.zeros(BUFFER_SIZE),
                  'position': np.zeros(BUFFER_SIZE)},
            '2': {'setpoint': np.zeros(BUFFER_SIZE),
                  'position': np.zeros(BUFFER_SIZE)}
        }
        self.x = np.arange(BUFFER_SIZE)
        self.is_recording = False
        self.csv_file = None
        self.csv_writer = None
        self.record_start_time = None
        self.record_file_path = None
        self.init_ui()
        self.refresh_serial_ports()

    def init_ui(self):
        self.setWindowTitle("Dual Motor PID Controller")
        self.setGeometry(100, 100, 1600, 900)
        self.setStyleSheet(STYLE_SHEET)

        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)
        layout.setContentsMargins(10, 10, 10, 10)

        # Connection Controls
        control_widget = QWidget()
        control_layout = QHBoxLayout(control_widget)
        control_layout.setContentsMargins(0, 0, 0, 0)
        
        # Serial Port Selection
        port_box = QWidget()
        port_layout = QHBoxLayout(port_box)
        self.port_combo = QComboBox()
        port_layout.addWidget(QLabel("Port:"))
        port_layout.addWidget(self.port_combo)

        # Baud Rate Selection
        baud_box = QWidget()
        baud_layout = QHBoxLayout(baud_box)
        self.baud_combo = QComboBox()
        self.baud_combo.addItems(['9600', '19200', '38400', '57600', '115200'])
        self.baud_combo.setCurrentText('115200')
        baud_layout.addWidget(QLabel("Baud:"))
        baud_layout.addWidget(self.baud_combo)

        # Buttons
        self.connect_btn = QPushButton("Connect")
        self.connect_btn.clicked.connect(self.toggle_connection)
        self.pause_btn = QPushButton("Pause")
        self.pause_btn.clicked.connect(self.toggle_pause)
        self.pause_btn.setEnabled(False)

        # Add record button
        self.record_button = QPushButton("Select File")
        self.record_button.clicked.connect(self.toggle_recording)
        control_layout.addWidget(self.record_button)

        # Add widgets to control layout
        control_layout.addStretch()
        control_layout.addWidget(port_box)
        control_layout.addWidget(baud_box)
        control_layout.addWidget(self.connect_btn)
        control_layout.addWidget(self.pause_btn)
        control_layout.addStretch()

        layout.addWidget(control_widget, alignment=Qt.AlignCenter)

        # Main Content
        content_layout = QHBoxLayout()
        
        # Left Panel (Motor 1)
        self.motor1_panel = MotorControlPanel(1, DARK_PALETTE['highlight'])
        content_layout.addWidget(self.motor1_panel)

        # Plots
        plot_widget = QWidget()
        plot_layout = QVBoxLayout(plot_widget)
        
        self.plot1 = pg.PlotWidget(title="Motor 1 Response")
        self.plot1.setBackground(DARK_PALETTE['plot_bg'])
        self.plot1.addLegend()
        self.setpoint1_curve = self.plot1.plot(
            pen=pg.mkPen(DARK_PALETTE['highlight'], width=2),
            name="Setpoint"
        )
        self.position1_curve = self.plot1.plot(
            pen=pg.mkPen(DARK_PALETTE['secondary'], width=2),
            name="Position"
        )
        
        self.plot2 = pg.PlotWidget(title="Motor 2 Response")
        self.plot2.setBackground(DARK_PALETTE['plot_bg'])
        self.plot2.addLegend()
        self.setpoint2_curve = self.plot2.plot(
            pen=pg.mkPen(DARK_PALETTE['accent'], width=2),
            name="Setpoint"
        )
        self.position2_curve = self.plot2.plot(
            pen=pg.mkPen("#E74C3C", width=2),
            name="Position"
        )
        
        plot_layout.addWidget(self.plot1)
        plot_layout.addWidget(self.plot2)
        content_layout.addWidget(plot_widget, stretch=3)

        # Right Panel (Motor 2)
        self.motor2_panel = MotorControlPanel(2, DARK_PALETTE['accent'])
        content_layout.addWidget(self.motor2_panel)

        layout.addLayout(content_layout)

    def refresh_serial_ports(self):
        self.port_combo.clear()
        ports = list_ports.comports()
        for port in ports:
            self.port_combo.addItem(port.device)
        if not ports:
            self.port_combo.addItem("No ports found")
        else:
            com6_index = self.port_combo.findText(DEFAULT_PORT)
            if com6_index != -1:
                self.port_combo.setCurrentIndex(com6_index)
                QTimer.singleShot(1000, self.attempt_auto_connect)

    def attempt_auto_connect(self):
        if self.port_combo.currentText() == DEFAULT_PORT:
            self.toggle_connection()

    def toggle_connection(self):
        if self.serial_worker and self.serial_worker.isRunning():
            self.serial_worker.stop()
            self.connect_btn.setText("Connect")
            self.pause_btn.setEnabled(False)
        else:
            port = self.port_combo.currentText()
            baud_rate = int(self.baud_combo.currentText())
            
            if port == "No ports found":
                QMessageBox.critical(self, "Error", "No serial ports available!")
                return
            
            self.serial_worker = SerialWorker(port, baud_rate)
            self.serial_worker.data_received.connect(self.update_data)
            self.serial_worker.error_occurred.connect(self.show_error)
            self.serial_worker.start()
            self.connect_btn.setText("Disconnect")
            self.pause_btn.setEnabled(True)

    def toggle_pause(self):
        self.paused = not self.paused
        self.pause_btn.setText("Resume" if self.paused else "Pause")

    def toggle_recording(self):
        if not self.is_recording and not self.record_file_path:
            # First click - Select file
            file_path, _ = QFileDialog.getSaveFileName(
                self,
                "Save CSV File",
                "",
                "CSV Files (*.csv);;All Files (*)"
            )
            
            if file_path:
                self.record_file_path = file_path
                self.record_button.setText("Start Recording")
                
        elif not self.is_recording and self.record_file_path:
            # Second click - Start recording
            try:
                self.csv_file = open(self.record_file_path, 'w', newline='')
                self.csv_writer = csv.writer(self.csv_file)
                self.csv_writer.writerow(['Timestamp', 'Data...'])
                self.is_recording = True
                self.record_button.setText("Stop Recording")
            except Exception as e:
                QMessageBox.critical(self, "Error", f"Could not open file: {str(e)}")
                self.record_file_path = None
                self.record_button.setText("Select File")
                
        else:
            # Third click - Stop recording and save
            if self.csv_file:
                self.csv_file.close()
                self.csv_file = None
                self.csv_writer = None
            self.is_recording = False
            self.record_file_path = None
            self.record_button.setText("Select File")

    def get_elapsed_ms(self):
        if self.record_start_time:
            return self.record_start_time.elapsed()
        return 0

    def update_data(self, data):
        if self.paused:
            return
        
        try:
            if len(data) != 12:
                return

            # Motor 1
            self.plot_data['1']['setpoint'] = np.roll(self.plot_data['1']['setpoint'], -1)
            self.plot_data['1']['setpoint'][-1] = data[0]
            self.plot_data['1']['position'] = np.roll(self.plot_data['1']['position'], -1)
            self.plot_data['1']['position'][-1] = data[1]
            
            # Motor 2
            self.plot_data['2']['setpoint'] = np.roll(self.plot_data['2']['setpoint'], -1)
            self.plot_data['2']['setpoint'][-1] = data[6]
            self.plot_data['2']['position'] = np.roll(self.plot_data['2']['position'], -1)
            self.plot_data['2']['position'][-1] = data[7]

            # Update plots
            self.setpoint1_curve.setData(self.x, self.plot_data['1']['setpoint'])
            self.position1_curve.setData(self.x, self.plot_data['1']['position'])
            self.setpoint2_curve.setData(self.x, self.plot_data['2']['setpoint'])
            self.position2_curve.setData(self.x, self.plot_data['2']['position'])

            # Update panels
            self.motor1_panel.update_from_data(data[0], data[1], data[2], data[3], data[4], data[5])
            self.motor2_panel.update_from_data(data[6], data[7], data[8], data[9], data[10], data[11])

            if self.is_recording and self.csv_writer:
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")
                self.csv_writer.writerow([timestamp] + data)
                self.csv_file.flush()

        except Exception as e:
            print(f"Data error: {str(e)}")

    @pyqtSlot(str)
    def show_error(self, msg):
        QMessageBox.critical(self, "Error", 
            f"<span style='color:{DARK_PALETTE['text']};'>Error: {msg}</span>")

    def closeEvent(self, event):
        if self.is_recording:
            self.toggle_recording()  # Stop recording if active
        if self.serial_worker and self.serial_worker.isRunning():
            self.serial_worker.stop()
        event.accept()

if __name__ == "__main__":
    # Create the application instance
    app = QApplication(sys.argv)
    
    # Set the default font for the application
    app.setFont(QFont("Segoe UI", 10))
    
    # Create and show the main window
    window = MainWindow()
    window.show()
    
    # Execute the application's main loop
    sys.exit(app.exec_())