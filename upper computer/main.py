import sys
import serial
import pyqtgraph as pg
from PySide6.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout
from PySide6.QtCore import QTimer

class PIDPlotter(QMainWindow):
    def __init__(self):
        super().__init__()
        
        # 1. 只有几行代码配置窗口
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.layout = QVBoxLayout(self.central_widget)
        
        # 2. 配置高性能绘图控件
        self.plot_widget = pg.PlotWidget()
        self.layout.addWidget(self.plot_widget)
        self.curve = self.plot_widget.plot(pen='y') # 黄色曲线
        self.data = [0] * 100 # 初始数据
        
        # 3. 串口初始化 (假设 COM3)
        try:
            self.ser = serial.Serial('COM3', 115200, timeout=0.1)
        except:
            print("串口打开失败，请检查")

        # 4. 定时器：每 20ms 读一次数据并刷新
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(20)

    def update_plot(self):
        if hasattr(self, 'ser') and self.ser.is_open:
            # 简单读取一行数据 (假设单片机发的是纯数字 "123\n")
            try:
                line = self.ser.readline().decode().strip()
                if line:
                    val = float(line)
                    self.data.pop(0)
                    self.data.append(val)
                    self.curve.setData(self.data) # 瞬间刷新
            except:
                pass

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = PIDPlotter()
    window.show()
    sys.exit(app.exec_())
