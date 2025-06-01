# Final Arduino exam project
# By Kevin Otto Koit
# IFI7340.DT 2025 Spring, HCI, TLU

import serial

#sets the port and Baud rate value
SERIAL_PORT = 'COM7'
BAUD_RATE = 9600

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
    print("Reading data from Kevin's Arduino, port: COM7, Baud rate: 9600")

    ##reads and prints the values
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            print(f"{line}")
except Exception as e:
    print(f"Error: {e}") ##if there's error