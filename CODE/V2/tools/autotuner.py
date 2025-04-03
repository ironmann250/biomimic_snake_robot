import sys
import numpy as np
from PyQt5.QtWidgets import (QApplication, QWidget, QLabel, QLineEdit, 
                            QVBoxLayout, QHBoxLayout, QFormLayout, 
                            QComboBox, QTableWidget, QTableWidgetItem)
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure

class PIDTuningApp(QWidget):
    def __init__(self):
        super().__init__()
        self.tuning_methods = {
            'Classic ZN': {'Kp': 0.6, 'Ki': 1.2, 'Kd': 0.075},
            'Pessen': {'Kp': 0.7, 'Ki': 1.75, 'Kd': 0.105},
            'Some Overshoot': {'Kp': 0.33, 'Ki': 0.66, 'Kd': 0.11},
            'No Overshoot': {'Kp': 0.2, 'Ki': 0.4, 'Kd': 0.066}
        }
        self.initUI()
        self.apply_dark_theme()
        
    def initUI(self):
        self.setWindowTitle('PID Tuning Calculator')
        self.setGeometry(100, 100, 1200, 600)
        
        # Create main layout
        main_layout = QHBoxLayout()
        
        # Left panel (input and results)
        left_panel = QVBoxLayout()
        
        # Input form
        form_layout = QFormLayout()
        self.method_combo = QComboBox()
        self.method_combo.addItems(self.tuning_methods.keys())
        self.ku_input = QLineEdit('10')
        self.tu_input = QLineEdit('2')
        form_layout.addRow(QLabel('Tuning Method:'), self.method_combo)
        form_layout.addRow(QLabel('Ultimate Gain (Ku):'), self.ku_input)
        form_layout.addRow(QLabel('Ultimate Period (Tu):'), self.tu_input)
        
        # Results table
        self.results_table = QTableWidget()
        self.results_table.setColumnCount(3)
        self.results_table.setHorizontalHeaderLabels(['Kp', 'Ki', 'Kd'])
        self.results_table.setRowCount(len(self.tuning_methods))
        
        for row, method in enumerate(self.tuning_methods):
            self.results_table.setVerticalHeaderItem(row, QTableWidgetItem(method))
        
        left_panel.addLayout(form_layout)
        left_panel.addWidget(QLabel("All Methods Results:"))
        left_panel.addWidget(self.results_table)
        
        # Right panel (graphic and explanation)
        right_panel = QVBoxLayout()
        
        # Matplotlib figure
        self.figure = Figure(figsize=(8, 5), dpi=100)
        self.canvas = FigureCanvas(self.figure)
        self.create_explanation_plot()
        
        right_panel.addWidget(self.canvas)
        
        # Add panels to main layout
        main_layout.addLayout(left_panel, 40)
        main_layout.addLayout(right_panel, 60)
        
        self.setLayout(main_layout)
        
        # Connect signals
        self.ku_input.textChanged.connect(self.update_calculations)
        self.tu_input.textChanged.connect(self.update_plot)
        self.method_combo.currentIndexChanged.connect(self.update_plot)
        self.update_calculations()

    def apply_dark_theme(self):
        self.setStyleSheet("""
            QWidget {
                background-color: #2E2E2E;
                color: #FFFFFF;
            }
            QLineEdit {
                background-color: #404040;
                border: 1px solid #606060;
                padding: 5px;
            }
            QTableWidget {
                background-color: #404040;
                gridline-color: #505050;
            }
            QHeaderView::section {
                background-color: #303030;
            }
            QComboBox {
                background-color: #404040;
                border: 1px solid #606060;
                padding: 5px;
            }
        """)

    def create_explanation_plot(self):
        self.figure.clear()
        self.figure.set_facecolor('#2E2E2E')
        ax = self.figure.add_subplot(111)
        ax.set_facecolor('#404040')
        
        try:
            tu = float(self.tu_input.text())
        except:
            tu = 2.0  # Default value if invalid input
            
        t = np.linspace(0, 4*tu, 500)
        
        # Create different response curves
        classic = 0.5 * np.sin(2*np.pi/tu * t) * np.exp(-0.1*t)
        pessen = 0.7 * np.sin(2*np.pi/tu * t) * np.exp(-0.2*t)
        some_os = 0.4 * np.sin(2*np.pi/tu * t) * np.exp(-0.15*t)
        no_os = 0.3 * np.sin(2*np.pi/tu * t) * np.exp(-0.3*t)
        
        # Plot responses
        ax.plot(t, classic, label='Classic ZN', color='#1f77b4')
        ax.plot(t, pessen, label='Pessen', color='#ff7f0e')
        ax.plot(t, some_os, label='Some Overshoot', color='#2ca02c')
        ax.plot(t, no_os, label='No Overshoot', color='#d62728')
        
        # Add tau (Tu) markers
        for n in range(1, 5):
            x_pos = n * tu
            ax.axvline(x=x_pos, color='#FFFFFF', linestyle='--', alpha=0.3)
            if n == 1:
                ax.text(x_pos, ax.get_ylim()[1]*0.9, f'τ = {tu:.1f}s', 
                       color='white', ha='center', rotation=90, alpha=0.7)

        # Styling
        ax.set_title('System Response Comparison', color='white')
        ax.set_xlabel('Time (s)', color='white')
        ax.set_ylabel('Amplitude', color='white')
        ax.tick_params(axis='both', colors='white')
        ax.grid(color='#505050', linestyle='--')
        ax.legend(facecolor='#404040', edgecolor='none', labelcolor='white')
        
        for spine in ax.spines.values():
            spine.set_color('#606060')
            
        self.canvas.draw()

    def update_calculations(self):
        try:
            ku = float(self.ku_input.text())
            tu = float(self.tu_input.text())
            
            if ku <= 0 or tu <= 0:
                raise ValueError
                
            # Calculate for all methods
            for row, (method, coeffs) in enumerate(self.tuning_methods.items()):
                kp = coeffs['Kp'] * ku
                ki = (coeffs['Ki'] * ku) / tu
                kd = coeffs['Kd'] * ku * tu
                
                self.results_table.setItem(row, 0, QTableWidgetItem(f"{kp:.4f}"))
                self.results_table.setItem(row, 1, QTableWidgetItem(f"{ki:.4f}"))
                self.results_table.setItem(row, 2, QTableWidgetItem(f"{kd:.4f}"))
            
        except ValueError:
            for row in range(len(self.tuning_methods)):
                self.results_table.setItem(row, 0, QTableWidgetItem("Invalid"))
                self.results_table.setItem(row, 1, QTableWidgetItem("Invalid"))
                self.results_table.setItem(row, 2, QTableWidgetItem("Invalid"))
    
    def update_plot(self):
        self.create_explanation_plot()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = PIDTuningApp()
    ex.show()
    sys.exit(app.exec_())