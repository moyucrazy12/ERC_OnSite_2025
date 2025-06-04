import socket
import threading
import json
import cv2
import struct
import pickle
import signal
import sys
import serial
import time
import pyrealsense2 as rs

# Set up the serial connection (modify if needed)
ser = serial.Serial('/dev/ttyACM1', 115200, timeout=1)  # Use ttyAMA0 if direct UART is used
time.sleep(1.5) 
ser2 = serial.Serial('/dev/ttyACM0', 115200, timeout=1)  # Use ttyAMA0 if direct UART is used

time.sleep(1.5)  # Allow time for serial to initialize

#Thread to receive joystick commands from PC (UDP)
def receive_commands():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", 5005))
    print("Listening for joystick commands on UDP port 5005...")

    while True:
        try:
            data, addr = sock.recvfrom(1024)
            command = json.loads(data.decode())
            ser.write((json.dumps(command) + "\n").encode())
            ser2.write((json.dumps(command) + "\n").encode())
            print(f"[COMMAND RECEIVED] {command}")
            time.sleep(0.1)
            # Use command['x'], command['y'], etc. here to control motors
        except Exception as e:
            print(f"Command error: {e}")

# Thread to send video stream (TCP)
def send_video():
    
	pipeline = rs.pipeline()
	config = rs.config()
	config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)

	pipeline.start(config)

	while True:
	    frames = pipeline.wait_for_frames()
	    color_frame = frames.get_color_frame()

	    if not color_frame:
		continue

	    color_image = np.asanyarray(color_frame.get_data())
	    cv2.imshow('RealSense', color_image)
	    
	    if cv2.waitKey(1) & 0xFF == ord('q'):
		break

	pipeline.stop()
	cv2.destroyAllWindows()
# Start both threads
def start_threads():
    t1 = threading.Thread(target=receive_commands, daemon=True)
    t2 = threading.Thread(target=send_video, daemon=True)

    t2.start()
    t1.start()

    t2.join()
    t1.join()

# Graceful shutdown handler
def shutdown_handler(signal, frame):
    print("\nShutting down the program gracefully...")
    sys.exit(0)

# Set up signal handler to catch Ctrl+C
signal.signal(signal.SIGINT, shutdown_handler)

if __name__ == "__main__":
    print("Program started. Press Ctrl+C to stop.")
    start_threads()
