import sys
import asyncio
import threading
from bleak import BleakClient, BleakScanner
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtCore import QThread, pyqtSignal
from pyqtgraph import PlotWidget, plot
import pyqtgraph as pg
import numpy as np

class BLEScanWorker(QThread):
    finished = pyqtSignal(list)

    def run(self):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        devices = loop.run_until_complete(self.scan_devices_async())
        self.finished.emit(devices)

    async def scan_devices_async(self):
        devices = await BleakScanner.discover()
        return [f"{device.name}: {device.address}" for device in devices if device.name]

class BLEApp(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.ble_client = None
        self.device_address = None
        self.data_characteristic_uuid = "0000FFE1-0000-1000-8000-00805F9B34FB"  # Example UUID, replace with your device's
        self.data_buffer = []
        self.scan_worker = None

    def initUI(self):
        self.central_widget = QtWidgets.QWidget()
        self.setCentralWidget(self.central_widget)
        self.layout = QtWidgets.QVBoxLayout(self.central_widget)

        # Create buttons
        self.scan_button = QtWidgets.QPushButton("Scan Devices")
        self.connect_button = QtWidgets.QPushButton("Connect")
        self.disconnect_button = QtWidgets.QPushButton("Disconnect")

        # Create plot widget
        self.plot_widget = PlotWidget()
        self.plot = self.plot_widget.plot(pen='y')

        # Add widgets to layout
        self.layout.addWidget(self.scan_button)
        self.layout.addWidget(self.connect_button)
        self.layout.addWidget(self.disconnect_button)
        self.layout.addWidget(self.plot_widget)

        # Connect buttons to functions
        self.scan_button.clicked.connect(self.scan_devices)
        self.connect_button.clicked.connect(self.connect_device)
        self.disconnect_button.clicked.connect(self.disconnect_device)

        # Initial button states
        self.connect_button.setEnabled(False)
        self.disconnect_button.setEnabled(False)

        # Set window properties
        self.setGeometry(100, 100, 600, 400)
        self.setWindowTitle('BLE Data Plotter')

    def scan_devices(self):
        self.scan_button.setEnabled(False)
        self.scan_worker = BLEScanWorker()
        self.scan_worker.finished.connect(self.on_scan_finished)
        self.scan_worker.start()

    def on_scan_finished(self, devices):
        self.scan_worker = None
        device, ok = QtWidgets.QInputDialog.getItem(self, "Select Device", "Available devices:", devices, 0, False)
        if ok and device:
            self.device_address = device.split(": ")[1]
            self.connect_button.setEnabled(True)
        self.scan_button.setEnabled(True)

    def connect_device(self):
        self.ble_thread = threading.Thread(target=self.connect_device_thread)
        self.ble_thread.start()

    def connect_device_thread(self):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        loop.run_until_complete(self.connect_device_async())

    async def connect_device_async(self):
        if self.device_address:
            self.ble_client = BleakClient(self.device_address)
            await self.ble_client.connect()
            await self.ble_client.start_notify(self.data_characteristic_uuid, self.notification_handler)
        QtCore.QMetaObject.invokeMethod(self, "on_connect_finished", QtCore.Qt.QueuedConnection)

    def on_connect_finished(self):
        self.disconnect_button.setEnabled(True)
        self.connect_button.setEnabled(False)

    def disconnect_device(self):
        self.ble_thread = threading.Thread(target=self.disconnect_device_thread)
        self.ble_thread.start()

    def disconnect_device_thread(self):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        loop.run_until_complete(self.disconnect_device_async())

    async def disconnect_device_async(self):
        if self.ble_client and self.ble_client.is_connected:
            await self.ble_client.disconnect()
        QtCore.QMetaObject.invokeMethod(self, "on_disconnect_finished", QtCore.Qt.QueuedConnection)

    def on_disconnect_finished(self):
        self.disconnect_button.setEnabled(False)
        self.connect_button.setEnabled(True)

    def notification_handler(self, sender, data):
        # Assuming the data is a single float value
        value = float(int.from_bytes(data, byteorder='little', signed=True))
        self.data_buffer.append(value)
        if len(self.data_buffer) > 100:  # Keep only the last 100 points
            self.data_buffer = self.data_buffer[-100:]
        QtCore.QMetaObject.invokeMethod(self, "update_plot", 
                                        QtCore.Qt.QueuedConnection, 
                                        QtCore.Q_ARG(list, self.data_buffer))

    def update_plot(self, data):
        self.plot.setData(np.array(data))

def main():
    app = QtWidgets.QApplication(sys.argv)
    window = BLEApp()
    window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
