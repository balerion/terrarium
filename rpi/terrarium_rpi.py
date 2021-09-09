#!/usr/bin/env python
'''
A Python class implementing KBHIT, the standard keyboard-interrupt poller.
Works transparently on Windows and Posix (Linux, Mac OS X).  Doesn't work
with IDLE.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 of the 
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

'''

import os
import serial
import time
import json
import sched
from datetime import datetime

# while True:
#     try:
#         arduino = serial.Serial(port='/dev/ttyUSB0', baudrate=9600, timeout=.1)
#         break
#     except serial.SerialException:
#         print ('No serial connection')
#         time.sleep(2)


# Windows
if os.name == 'nt':
    import msvcrt

# Posix (Linux, OS X)
else:
    import sys
    import termios
    import atexit
    from select import select


class KBHit:

    def __init__(self):
        '''Creates a KBHit object that you can call to do various keyboard things.
        '''

        if os.name == 'nt':
            pass

        else:

            # Save the terminal settings
            self.fd = sys.stdin.fileno()
            self.new_term = termios.tcgetattr(self.fd)
            self.old_term = termios.tcgetattr(self.fd)

            # New terminal setting unbuffered
            self.new_term[3] = (self.new_term[3] & ~termios.ICANON & ~termios.ECHO)
            termios.tcsetattr(self.fd, termios.TCSAFLUSH, self.new_term)

            # Support normal-terminal reset at exit
            atexit.register(self.set_normal_term)


    def set_normal_term(self):
        ''' Resets to normal terminal.  On Windows this is a no-op.
        '''

        if os.name == 'nt':
            pass

        else:
            termios.tcsetattr(self.fd, termios.TCSAFLUSH, self.old_term)


    def getch(self):
        ''' Returns a keyboard character after kbhit() has been called.
            Should not be called in the same program as getarrow().
        '''

        s = ''

        if os.name == 'nt':
            return msvcrt.getch().decode('utf-8')

        else:
            return sys.stdin.read(1)


    def getarrow(self):
        ''' Returns an arrow-key code after kbhit() has been called. Codes are
        0 : up
        1 : right
        2 : down
        3 : left
        Should not be called in the same program as getch().
        '''

        if os.name == 'nt':
            msvcrt.getch() # skip 0xE0
            c = msvcrt.getch()
            vals = [72, 77, 80, 75]

        else:
            c = sys.stdin.read(3)[2]
            vals = [65, 67, 66, 68]

        return vals.index(ord(c.decode('utf-8')))


    def kbhit(self):
        ''' Returns True if keyboard character was hit, False otherwise.
        '''
        if os.name == 'nt':
            return msvcrt.kbhit()

        else:
            dr,dw,de = select([sys.stdin], [], [], 0)
            return dr != []


def write_read(x):
    arduino.write(bytes(x, 'utf-8'))
    time.sleep(0.05)
    data = arduino.readline()
    return data

# Test    
if __name__ == "__main__":

    kb = KBHit()
    buffer = bytes()  # .read() returns bytes right?

    print('Hit any key, or ESC to exit')

    start = time.time()
    data = {}
    while True:
        if kb.kbhit():
            c = kb.getch()
            if ord(c) == 27: # ESC
                break
            print(f"sending {c.encode('ascii')} to arduino")
            # arduino.write(c.encode('ascii'))
            # arduino.write(b'\n')
            data["sensor"] = "gps"
            data["time"] = int(datetime.now().timestamp())
            data["control"] = int(c)
            data["data"] = [48.756080,2.34838]

            arduino.write(json.dumps(data).encode('ascii'))
            arduino.flush()


        if arduino.in_waiting > 0:
            buffer += arduino.read(arduino.in_waiting)
            jsonIn=''
            ascii=''

            try:
                jsonIn = json.loads(buffer[buffer.index(b'{'):buffer.index(b'}')+1])  # get up to '}'
                buffer = buffer[buffer.index(b'}')+1:]  # leave the rest in buffer
                jsonRcvd=True
            except ValueError:
                jsonRcvd=False
                
            try:
                ascii = buffer[:buffer.index(b'\n')]  # get up to '\n'
                buffer = b''
                ascii = ascii.decode('ascii')
                asciiRcvd=True
            except ValueError:
                asciiRcvd=False
            
            if jsonRcvd or asciiRcvd:
                pass  # Go back and keep reading
            else:
                continue
            
            print('json =', jsonIn)
            print('ascii =', ascii)
            try:
                print('received time: ', datetime.fromtimestamp(jsonIn['time']).time())
            except TypeError:
                print('no time received')

        if time.time() - start >= 60:
            start = time.time()
            data["time"] = int(datetime.now().timestamp())
            # data=json.dumps(data)
            arduino.write(json.dumps(data).encode('ascii'))
            arduino.flush()

        time.sleep(0.001)

    kb.set_normal_term()


