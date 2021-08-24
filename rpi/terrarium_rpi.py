# Importing Libraries
import serial
import time
import threading

port = '/dev/ttyUSB0'
baud = 9600
connected = False
arduino = serial.Serial(port, baud, timeout=.1)

def write_read(x):
    arduino.write(bytes(x, 'utf-8'))
    time.sleep(0.05)
    data = arduino.readline()
    return data

def handle_data(data):
    print(data)
    num = input("Enter a number: ") # Taking input from user
    value = write_read(num)
    print(value) # printing the value

def read_from_port(ser):
    while not connected:
        #serin = ser.read()
        connected = True

        while True:
           print("test")
           reading = ser.readline().decode()
           handle_data(reading)

thread = threading.Thread(target=read_from_port, args=(arduino,))
thread.start()

# while True:
#     num = input("Enter a number: ") # Taking input from user
#     value = write_read(num)
#     print(value) # printing the value




