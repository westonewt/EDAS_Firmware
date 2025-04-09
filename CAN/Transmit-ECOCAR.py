import can
import time
import gpiozero
import os
import ctypes

i=0
j = 0x00

#set up CAN0 with a bitrate of 1000kbps
os.system("sudo ip link set can0 up type can bitrate 1000000 fd off")

try:
    bus=can.interface.Bus(channel='can0',bustype='socketcan',fd=False)
    print("Bus interface connected")

except OSError:
     print("could not find Bus interface")
     exit()
try:
    while True:
        msg=can.Message(arbitration_id=0x123,data=[j,0x22,0x33,0x44,0x55,0x66,0x77,0x88],is_extended_id=False,is_fd=False)
        try:
            bus.send(msg)
            print(f"message sent {msg}")
            time.sleep(0.1)
        except can.CanError:
            print("Error: Message not sent")
        j=j+0x01

except KeyboardInterrupt:
    os.system("sudo ip link set can0 down")
    print('\n\rKeyboard interrtupt')
