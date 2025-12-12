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

# Protocol Configuration
USE_NEW_PROTOCOL = True  # Firmware is using 25-byte new protocol

if USE_NEW_PROTOCOL:
    # New Protocol: Header(2) + Kp(4) + Ki(4) + Kd(4) + Target(4) + StepAmp(4) + StepInt(2) + Checksum(1) = 25 bytes
    PACKET_FMT = '<fffiih' # Kp, Ki, Kd, Target(int32), StepAmplitude(int32), StepInterval(uint16)
else:
    # Old Protocol: Header(2) + Kp(4) + Ki(4) + Kd(4) + Target(4) + Checksum(1) = 19 bytes
    PACKET_FMT = '<fffi' # Kp, Ki, Kd, Target(int32)

# Header: 0xAA 0xBB, Floats are little-endian
PACKET_SIZE = struct.calcsize(PACKET_FMT)

class PIDTuner:
    def __init__(self, root):
        self.root = root
        self.root.title("STM32 PID Tuner & Waveform")
        
        self.ser = None
        self.is_connected = False
        
        # Variables
        self.kp = tk.DoubleVar(value=0.0)
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

        # Step Response Frame
        step_frame = ttk.LabelFrame(left_panel, text="Step Response Test")
        step_frame.pack(fill="x", padx=5, pady=5)
        
        step_row1 = ttk.Frame(step_frame)
        step_row1.pack(fill="x", padx=5, pady=2)
        ttk.Label(step_row1, text="Amplitude:").pack(side="left")
        self.step_amplitude = tk.IntVar(value=500)
        ttk.Entry(step_row1, textvariable=self.step_amplitude, width=8).pack(side="left", padx=5)
        
        step_row2 = ttk.Frame(step_frame)
        step_row2.pack(fill="x", padx=5, pady=2)
        ttk.Label(step_row2, text="Interval (ms):").pack(side="left")
        self.step_interval = tk.IntVar(value=5000)
        ttk.Entry(step_row2, textvariable=self.step_interval, width=8).pack(side="left", padx=5)
        
        self.btn_step = ttk.Button(step_frame, text="▶ Start Step Test", command=self.start_step_test)
        self.btn_step.pack(fill="x", padx=5, pady=5)
        
        self.btn_clear = ttk.Button(step_frame, text="Clear Plot", command=self.clear_plot)
        self.btn_clear.pack(fill="x", padx=5, pady=5)

        # Right Panel: Plot
        right_panel = ttk.Frame(self.root)
        right_panel.pack(side="right", fill="both", expand=True)
        
        self.fig = Figure(figsize=(6, 4), dpi=100)
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
        
        # Serial Log Frame
        log_frame = ttk.LabelFrame(right_panel, text="Serial Log")
        log_frame.pack(fill="x", padx=5, pady=5)
        
        self.log_text = tk.Text(log_frame, height=6, wrap=tk.WORD, state=tk.DISABLED)
        self.log_text.pack(fill="x", padx=5, pady=5)
        
        log_btn_frame = ttk.Frame(log_frame)
        log_btn_frame.pack(fill="x", padx=5, pady=2)
        ttk.Button(log_btn_frame, text="Clear Log", command=self.clear_log).pack(side="left")

    def create_slider(self, parent, label, variable, min_val, max_val, step):
        frame = ttk.Frame(parent)
        frame.pack(fill="x", padx=5, pady=2)
        
        ttk.Label(frame, text=label, width=6).pack(side="left")
        
        # - 按钮
        btn_dec = ttk.Button(frame, text="-", width=2, 
                            command=lambda: self.adjust_value(variable, -step))
        btn_dec.pack(side="left", padx=2)
        
        scale = ttk.Scale(frame, from_=min_val, to=max_val, variable=variable, 
                         command=lambda v: self.on_change(variable))
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
            if isinstance(variable, tk.DoubleVar):
                new_value = round(new_value, 2)
            variable.set(new_value)
            if self.auto_send.get() and self.is_connected:
                self.send_params()
        except:
            pass

    def on_change(self, variable=None):
        if variable and isinstance(variable, tk.DoubleVar):
            try:
                # Round to 2 decimal places
                val = variable.get()
                variable.set(round(val, 2))
            except:
                pass

        if self.auto_send.get() and self.is_connected:
            self.send_params()

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



    def send_params(self):
        if not self.is_connected or not self.ser:
            return
            
        kp = float(self.kp.get())
        ki = float(self.ki.get())
        kd = float(self.kd.get())
        target = int(self.target.get())
        
        if USE_NEW_PROTOCOL:
            step_amp = int(self.step_amplitude.get())
            step_int = int(self.step_interval.get())
            payload = struct.pack(PACKET_FMT, kp, ki, kd, target, step_amp, step_int)
        else:
            payload = struct.pack(PACKET_FMT, kp, ki, kd, target)
        
        header = b'\xAA\xBB'
        
        checksum = 0
        for b in payload:
            checksum = (checksum + b) & 0xFF
            
        packet = header + payload + bytes([checksum])
        
        self.ser.write(packet)
        self.ser.flush()  # Ensure data is sent immediately
        
        # Debug: print packet hex
        print(f"TX: Kp={kp:.2f} Ki={ki:.2f} Kd={kd:.2f} Target={target} | {packet.hex()}")

    def clear_plot(self):
        """Clear the plot data"""
        self.time_data.clear()
        self.target_data.clear()
        self.current_data.clear()
        self.start_time = time.time()
        print("Plot cleared")

    def clear_log(self):
        """Clear the serial log"""
        self.log_text.config(state=tk.NORMAL)
        self.log_text.delete(1.0, tk.END)
        self.log_text.config(state=tk.DISABLED)

    def log_message(self, msg):
        """Add a message to the serial log"""
        def _update():
            self.log_text.config(state=tk.NORMAL)
            self.log_text.insert(tk.END, msg + "\n")
            self.log_text.see(tk.END)  # Auto-scroll
            self.log_text.config(state=tk.DISABLED)
        self.root.after(0, _update)

    def start_step_test(self):
        """Start a step response test"""
        if not self.is_connected:
            print("Not connected!")
            return
        
        amplitude = self.step_amplitude.get()
        
        # Clear plot and reset time
        self.clear_plot()
        
        # First set target to 0
        self.target.set(0)
        self.send_params()
        print(f"Step test: Starting from 0, will step to {amplitude}")
        
        # Schedule step after 1 second
        def do_step():
            self.target.set(amplitude)
            self.send_params()
            print(f"Step test: Stepped to {amplitude}")
        
        self.root.after(1000, do_step)

    def rx_task(self):
        # Updated regex to match new format: T:500 C:123 E:377 | Kp:... | P:... | O:...
        # We only need T and C for plotting
        pattern = re.compile(r"T:(-?\d+)\s+C:(-?\d+)")
        
        while self.running:
            if self.is_connected and self.ser:
                try:
                    line = self.ser.readline()
                    if line:
                        try:
                            text = line.decode('utf-8', errors='ignore').strip()
                            if text:
                                # Log non-T:C: messages (debug output, etc.)
                                if not text.startswith("T:"):
                                    self.log_message(text)
                                
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
                    self.log_message(f"RX Error: {e}")
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
