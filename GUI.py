import Tkinter as tk
import ttk
import serial
import threading
from tkMessageBox import showerror  # For Python 2 compatibility


class RoomMonitorApp:
    def __init__(self, master):
        self.master = master
        self.master.title("Room Monitor")
        self.master.geometry("600x400")
        self.master.configure(bg="#eaeaea")

        # Header
        header_label = tk.Label(
            master, text="Room Monitor", font=("Verdana", 20, "bold"), bg="#eaeaea", fg="#444"
        )
        header_label.pack(pady=15)

     
        self.room_frame = tk.Frame(master, bg="#eaeaea")
        self.room_frame.pack(pady=10)

        # Room labels
        self.room1_label = tk.Label(
            self.room_frame, text="Room 1: 0 people", font=("Verdana", 14), bg="#eaeaea", fg="#0056b3"
        )
        self.room1_label.grid(row=0, column=0, padx=40, pady=10)

        self.room2_label = tk.Label(
            self.room_frame, text="Room 2: 0 people", font=("Verdana", 14), bg="#eaeaea", fg="#0056b3"
        )
        self.room2_label.grid(row=1, column=0, padx=40, pady=10)

        # Door and sensor information
        self.status_frame = tk.Frame(master, bg="#eaeaea")
        self.status_frame.pack(pady=20)

        # Door and sensor labels
        self.door1_status_label = tk.Label(
            self.status_frame, text="Door 1: Closed", font=("Verdana", 14), bg="#eaeaea", fg="#b33f40"
        )
        self.door1_status_label.grid(row=0, column=0, padx=20, pady=10)

        self.door2_status_label = tk.Label(
            self.status_frame, text="Door 2: Closed", font=("Verdana", 14), bg="#eaeaea", fg="#b33f40"
        )
        self.door2_status_label.grid(row=1, column=0, padx=20, pady=10)

        self.sensor_status_label = tk.Label(
            self.status_frame, text="Sensor: Not Detected", font=("Verdana", 14), bg="#eaeaea", fg="#b33f40"
        )
        self.sensor_status_label.grid(row=2, column=0, padx=20, pady=10)

        # Widget that could be used optional
        self.text_display = tk.Text(
            master, height=10, width=60, wrap=tk.WORD, bg="#f4f4f4", font=("Verdana", 12)
        )
        self.text_display.pack(pady=20)

        # Quit button
        self.quit_button = ttk.Button(
            master, text="Quit", command=self.master.quit
        )
        self.quit_button.pack(pady=20)

        # Serial connection setup
        self.serial_port = '/dev/ttyACM0'  
        self.baud_rate = 115200
        self.timeout = 1

        self.serial_thread = threading.Thread(target=self.read_serial_data)
        self.serial_thread.daemon = True
        self.serial_thread.start()

    def read_serial_data(self):
        try:
            ser = serial.Serial(port=self.serial_port, baudrate=self.baud_rate, timeout=self.timeout)
            print "Connected to", self.serial_port

            while True:
                data = ser.readline().strip()
                if data:
                    print "Received data: {}".format(data)
                    self.master.after(0, self.process_serial_data, data)  
        except serial.SerialException as e:
            print "Error:", e
            showerror("Error", "Serial connection failed: %s" % e)

    def process_serial_data(self, data):
        try:
            person_added = False

            if 'Room 1' in data or 'room 2' in data:
                room1_count, room2_count = self.extract_room_counts(data)
                if room1_count is not None:
                    self.room1_label.config(text="Room 1: {} people".format(room1_count))
                    person_added = True
                if room2_count is not None:
                    self.room2_label.config(text="Room 2: {} people".format(room2_count))
                    person_added = True

            if 'Door 1 opened' in data:
                self.door1_status_label.config(text="Door 1: Opened", fg="#2e8b57")
            elif 'Door 1 closed' in data:
                self.door1_status_label.config(text="Door 1: Closed", fg="#b33f40")
            if 'Door 2 opened' in data:
                self.door2_status_label.config(text="Door 2: Opened", fg="#2e8b57")
            elif 'Door 2 closed' in data:
                self.door2_status_label.config(text="Door 2: Closed", fg="#b33f40")
            if 'Movement detected' in data:
                self.sensor_status_label.config(text="Sensor: Detected", fg="#2e8b57")

            if person_added:
                self.reset_statuses()

            self.update_text_display(data)

        except Exception as e:
            print "Error processing serial data:", e

    def extract_room_counts(self, data):
        """Extract people counts for Room 1 and Room 2."""
        room1_count, room2_count = None, None
        if 'Room 1' in data:
            try:
                room1_count = int(data.split('Room 1:')[1].split(',')[0].strip())
            except (IndexError, ValueError):
                print "Failed to parse Room 1 count"
        if 'room 2' in data:
            try:
                room2_count = int(data.split('room 2:')[1].strip())
            except (IndexError, ValueError):
                print "Failed to parse Room 2 count"
        return room1_count, room2_count

    def reset_statuses(self):
        """Reset the statuses of doors and the sensor."""
        self.door1_status_label.config(text="Door 1: Closed", fg="#b33f40")
        self.door2_status_label.config(text="Door 2: Closed", fg="#b33f40")
        self.sensor_status_label.config(text="Sensor: Not Detected", fg="#b33f40")

    def update_text_display(self, data):
        self.text_display.insert(tk.END, data + "\n")
        self.text_display.yview(tk.END)

    def cleanup(self):
        if self.serial_thread.is_alive():
            self.serial_thread.join()


if __name__ == "__main__":
    root = tk.Tk()
    app = RoomMonitorApp(root)
    root.protocol("WM_DELETE_WINDOW", app.cleanup)
    root.mainloop()

