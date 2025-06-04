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
    IP = "10.100.100.164"  # Or "192.168.X.X"
    PORT = 5005
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    pygame.init()
    pygame.joystick.init()
    joystick = pygame.joystick.Joystick(0)
    joystick.init()
    # MODE = 0 : Diferential
    # MODE = 1 : Ackerman
    # MODE = 2 : Pure rotation
    # MODE = 3 : Pure translation
    mode = 0

    print("Started joystick thread")

    while True:
        pygame.event.pump()
        axis_0 = -int(joystick.get_axis(1) * 1000)
        axis_2 = int(joystick.get_axis(2) * 1000)
        command = {
            "x": axis_0,
            "y": axis_2,
            "start": joystick.get_button(0),  #x
            "stop": joystick.get_button(1) #O
        }
        #R1 = joystick.get_button(5)
        #L1 = joystick.get_button(4)
        if int(joystick.get_button(7)) == 1:
            mode+=1
            if mode > 3:
                mode = 3
            if mode == 0:
                print ("Differential mode")
            elif mode == 1:
                print ("Ackerman mode")
            elif mode == 2:
                print ("Rotation mode")
            elif mode == 3:
                print ("Translation mode")
        elif int(joystick.get_button(6)) == 1:
            mode-=1
            if mode < 0:
                mode = 0
            if mode == 0:
                print ("Differential mode")
            elif mode == 1:
                print ("Ackerman mode")
            elif mode == 2:
                print ("Rotation mode")
            elif mode == 3:
                print ("Translation mode")
        

        command2 = [mode,axis_0,axis_2,joystick.get_button(0),joystick.get_button(1)]

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
            # Read the size of the incoming frame
            packed_size = conn_file.read(4)
            if not packed_size:
                break

            data_len = struct.unpack(">L", packed_size)[0]
            frame_data = conn_file.read(data_len)
            if not frame_data:
                break

            # Decode from pickle
            encoded_image = pickle.loads(frame_data)

            # Decode the JPEG image to get the frame
            frame = cv2.imdecode(encoded_image, cv2.IMREAD_COLOR)

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

