import serial
import threading
import time
import numpy as np
import matplotlib.pyplot as plt

PORT = "COM3"
BAUD = 9600

ser = serial.Serial(PORT, BAUD, timeout=0.01)

latest_angle = 0
latest_distance = 0
data_lock = threading.Lock()
running = True

def serial_reader():
    global latest_angle, latest_distance, running
    while running:
        try:
            line = ser.readline().decode('utf-8').strip()
            if not line:
                continue

            a, d = line.split(",")
            angle = int(a)
            distance = int(d)

            with data_lock:
                latest_angle = angle
                latest_distance = distance

        except:
            continue

thread = threading.Thread(target=serial_reader, daemon=True)
thread.start()

plt.ion()
fig = plt.figure()
ax = fig.add_subplot(111, polar=True)

ax.set_theta_zero_location("W")
ax.set_theta_direction(1)
ax.set_thetamin(0)
ax.set_thetamax(180)

ax.set_rlim(0, 40)
ax.set_rticks([10, 20, 30, 40])
ax.grid(True)

line, = ax.plot([], [], color="green", linewidth=2)
point, = ax.plot([], [], "ro")

smoothed_distance = 0
alpha = 0.3
last_valid_time = 0
valid_timeout = 0.1

FPS = 30
FRAME_TIME = 1 / FPS

try:
    while True:
        start_time = time.time()

        with data_lock:
            angle = latest_angle
            distance = latest_distance

        theta = np.deg2rad(angle)

        if 0 < distance <= 40:
            if distance < 3 and smoothed_distance > 3:
                distance = smoothed_distance

            if smoothed_distance == 0:
                smoothed_distance = distance
                last_valid_time = time.time()
            elif abs(distance - smoothed_distance) < 5:
                smoothed_distance = alpha * distance + (1 - alpha) * smoothed_distance
                last_valid_time = time.time()
            else:
                smoothed_distance = distance
                last_valid_time = time.time()
        else:
            if time.time() - last_valid_time > valid_timeout:
                smoothed_distance = 0

        line.set_data([theta, theta], [0, 40])

        if smoothed_distance > 0:
            point.set_data([theta], [smoothed_distance])
        else:
            point.set_data([], [])

        plt.pause(0.001)

        elapsed = time.time() - start_time
        sleep_time = FRAME_TIME - elapsed
        if sleep_time > 0:
            time.sleep(sleep_time)

except KeyboardInterrupt:
    running = False
    ser.close()
    print("Radar visualization stopped cleanly.")
