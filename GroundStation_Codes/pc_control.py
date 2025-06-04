import pygame
import socket
import json
import threading
import time
import cv2
import struct
import pickle
import sys
import signal

# Joystick control thread
def joystick_sender():
    IP = "10.100.102.56"  # Or "192.168.X.X"
    PORT = 5005
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    pygame.init()
    pygame.joystick.init()
    joystick = pygame.joystick.Joystick(0)
    joystick.init()

    print("Started joystick thread")

    while True:
        pygame.event.pump()
        axis_0 = -int(joystick.get_axis(1) * 1000)
        axis_2 = int(joystick.get_axis(3) * 1000)
        command = {
            "x": axis_0,
            "y": axis_2,
            "start": joystick.get_button(0),  #x
            "stop": joystick.get_button(1) #O
        }
        command2 = [axis_0,axis_2,joystick.get_button(0),joystick.get_button(1)]

        sock.sendto(json.dumps(command2).encode(), (IP, PORT))
        time.sleep(0.1)

# Video stream receiver thread
def video_receiver():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(("0.0.0.0", 8000))
    server_socket.listen(1)
    print("Waiting for video stream...")

    conn, addr = server_socket.accept()
    print(f"Video stream connected from {addr}")
    conn_file = conn.makefile('rb')

    while True:
        try:
            data_len = struct.unpack(">L", conn_file.read(4))[0]
            frame_data = conn_file.read(data_len)
            frame = pickle.loads(frame_data)

            cv2.imshow("Camera", frame)
            if cv2.waitKey(1) == ord('q'):
                break
        except Exception as e:
            print(f"Video stream error: {e}")
            break

    conn_file.close()
    conn.close()
    server_socket.close()

# Graceful shutdown handler
def shutdown_handler(signal, frame):
    print("\nShutting down the program gracefully...")
    stop_threads.set()  # Signal threads to stop
    sys.exit(0)

# Set up signal handler to catch Ctrl+C
signal.signal(signal.SIGINT, shutdown_handler)

# Launch both threads
if __name__ == "__main__":

    stop_threads = threading.Event()
    t1 = threading.Thread(target=joystick_sender)
    t2 = threading.Thread(target=video_receiver)

    t2.start()
    t1.start()

    t2.join()
    t1.join()

