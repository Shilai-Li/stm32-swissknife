import serial
import serial.tools.list_ports
import struct
import tkinter as tk
from tkinter import ttk
import threading
import time
import re
from collections import deque

import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import matplotlib.animation as animation

# Protocol: Header(2) + Kp(4) + Ki(4) + Kd(4) + Target(4) + Checksum(1)
# Header: 0xAA 0xBB
# Floats are little-endian
PACKET_FMT = '<fffi' # Kp, Ki, Kd, Target(int32)
PACKET_SIZE = struct.calcsize(PACKET_FMT)

class PIDTuner:
    def __init__(self, root):
        self.root = root
        self.root.title("STM32 PID Tuner & Waveform")
        
        self.ser = None
        self.is_connected = False
        
        # Variables
        self.kp = tk.DoubleVar(value=0.5)
        self.ki = tk.DoubleVar(value=0.0)
        self.kd = tk.DoubleVar(value=0.0)
        self.target = tk.IntVar(value=0)
        
        # Data Buffers for Plotting
        self.max_points = 200
        self.time_data = deque(maxlen=self.max_points)
        self.target_data = deque(maxlen=self.max_points)
        self.current_data = deque(maxlen=self.max_points)
        self.start_time = time.time()
        
        self.create_widgets()
        self.scan_ports()
        
        # Start receive thread
        self.running = True
        self.rx_thread = threading.Thread(target=self.rx_task)
        self.rx_thread.daemon = True
        self.rx_thread.start()

        # Start Animation
        self.ani = animation.FuncAnimation(self.fig, self.update_plot, interval=50, blit=False)

    def create_widgets(self):
        # Left Panel: Controls
        left_panel = ttk.Frame(self.root, width=300)
        left_panel.pack(side="left", fill="y", padx=5, pady=5)
        
        # Connection Frame
        conn_frame = ttk.LabelFrame(left_panel, text="Connection")
        conn_frame.pack(fill="x", padx=5, pady=5)
        
        self.port_combo = ttk.Combobox(conn_frame)
        self.port_combo.pack(side="top", fill="x", padx=5, pady=5)
        
        self.btn_refresh = ttk.Button(conn_frame, text="Refresh", command=self.scan_ports)
        self.btn_refresh.pack(side="top", fill="x", padx=5, pady=5)
        
        self.btn_connect = ttk.Button(conn_frame, text="Connect", command=self.toggle_connect)
        self.btn_connect.pack(side="top", fill="x", padx=5, pady=5)
        
        # PID Frame
        pid_frame = ttk.LabelFrame(left_panel, text="PID Parameters")
        pid_frame.pack(fill="x", padx=5, pady=5)
        
        self.create_slider(pid_frame, "Kp", self.kp, 0.0, 10.0, 0.01)
        self.create_slider(pid_frame, "Ki", self.ki, 0.0, 10.0, 0.01)
        self.create_slider(pid_frame, "Kd", self.kd, 0.0, 10.0, 0.01)
        self.create_slider(pid_frame, "Target", self.target, -10000, 10000, 10)
        
        # Send Button
        self.btn_send = ttk.Button(left_panel, text="Send Parameters", command=self.send_params)
        self.btn_send.pack(fill="x", padx=5, pady=5)
        
        # Auto Send Checkbox
        self.auto_send = tk.BooleanVar(value=True)
        ttk.Checkbutton(left_panel, text="Auto Send on Change", variable=self.auto_send).pack(pady=5)

        # Right Panel: Plot
        right_panel = ttk.Frame(self.root)
        right_panel.pack(side="right", fill="both", expand=True)
        
        self.fig = Figure(figsize=(6, 5), dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.ax.set_title("Position Response")
        self.ax.set_xlabel("Time (s)")
        self.ax.set_ylabel("Position")
        self.line_target, = self.ax.plot([], [], 'r--', label='Target')
        self.line_current, = self.ax.plot([], [], 'b-', label='Current')
        self.ax.legend()
        self.ax.grid(True)
        
        self.canvas = FigureCanvasTkAgg(self.fig, master=right_panel)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill="both", expand=True)

    def create_slider(self, parent, label, variable, min_val, max_val, step):
        frame = ttk.Frame(parent)
        frame.pack(fill="x", padx=5, pady=2)
        
        ttk.Label(frame, text=label, width=6).pack(side="left")
        
        # - 按钮
        btn_dec = ttk.Button(frame, text="-", width=2, 
                            command=lambda: self.adjust_value(variable, -step))
        btn_dec.pack(side="left", padx=2)
        
        scale = ttk.Scale(frame, from_=min_val, to=max_val, variable=variable, 
                         command=lambda v: self.on_change())
        scale.pack(side="left", fill="x", expand=True)
        
        # + 按钮
        btn_inc = ttk.Button(frame, text="+", width=2,
                            command=lambda: self.adjust_value(variable, step))
        btn_inc.pack(side="left", padx=2)
        
        entry = ttk.Entry(frame, textvariable=variable, width=8)
        entry.pack(side="right")
    
    def adjust_value(self, variable, delta):
        """微调参数值"""
        try:
            current = variable.get()
            new_value = current + delta
            variable.set(new_value)
            if self.auto_send.get() and self.is_connected:
                self.send_params()
        except:
            pass

    def scan_ports(self):
        # 保存当前选择的端口
        current_port = self.port_combo.get()
        
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.port_combo['values'] = ports
        
        # 如果之前选择的端口还在列表中，继续选择它
        if current_port in ports:
            self.port_combo.set(current_port)
        elif ports:
            # 否则选择第一个
            self.port_combo.current(0)

    def toggle_connect(self):
        if not self.is_connected:
            try:
                port = self.port_combo.get()
                self.ser = serial.Serial(port, 115200, timeout=0.1)
                self.is_connected = True
                self.btn_connect.config(text="Disconnect")
                print(f"Connected to {port}")
                # Reset buffers
                self.time_data.clear()
                self.target_data.clear()
                self.current_data.clear()
                self.start_time = time.time()
            except Exception as e:
                print(f"Error: {e}")
        else:
            if self.ser:
                self.ser.close()
            self.is_connected = False
            self.btn_connect.config(text="Connect")
            print("Disconnected")

    def on_change(self):
        if self.auto_send.get() and self.is_connected:
            self.send_params()

    def send_params(self):
        if not self.is_connected or not self.ser:
            return
            
        kp = float(self.kp.get())
        ki = float(self.ki.get())
        kd = float(self.kd.get())
        target = int(self.target.get())
        
        payload = struct.pack(PACKET_FMT, kp, ki, kd, target)
        header = b'\xAA\xBB'
        
        checksum = 0
        for b in payload:
            checksum = (checksum + b) & 0xFF
            
        packet = header + payload + bytes([checksum])
        
        self.ser.write(packet)

    def rx_task(self):
        # Regex to match T:100 C:99
        pattern = re.compile(r"T:(-?\d+)\s+C:(-?\d+)")
        
        while self.running:
            if self.is_connected and self.ser:
                try:
                    line = self.ser.readline()
                    if line:
                        try:
                            text = line.decode('utf-8', errors='ignore').strip()
                            match = pattern.search(text)
                            if match:
                                t_val = int(match.group(1))
                                c_val = int(match.group(2))
                                
                                now = time.time() - self.start_time
                                self.time_data.append(now)
                                self.target_data.append(t_val)
                                self.current_data.append(c_val)
                        except:
                            pass
                except Exception as e:
                    print(f"RX Error: {e}")
                    time.sleep(1)
            else:
                time.sleep(0.1)

    def update_plot(self, frame):
        if not self.time_data:
            return
        
        # Convert deque to list for plotting
        t = list(self.time_data)
        tgt = list(self.target_data)
        cur = list(self.current_data)
        
        self.line_target.set_data(t, tgt)
        self.line_current.set_data(t, cur)
        
        self.ax.relim()
        self.ax.autoscale_view()
        
        # Keep window moving
        if t[-1] > 10: # If more than 10 seconds
            self.ax.set_xlim(t[-1] - 10, t[-1])
        else:
            self.ax.set_xlim(0, max(10, t[-1]))

if __name__ == "__main__":
    root = tk.Tk()
    app = PIDTuner(root)
    root.mainloop()
