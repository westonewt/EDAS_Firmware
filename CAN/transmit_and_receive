import can
import time
import gpiozero
import os

#initialize can
try:
    bus1=can.interface.Bus(channel='can0',bustype='socketcan',fd=False)
    bus2=can.interface.Bus(channel='can1',bustype='socketcan',fd=False)
    os.system("sudo ip link set can0 up type can bitrate 1000000 fd off")
    os.system("sudo ip link set can1 up type can bitrate 1000000 fd off")
    print("Bus interface connected")

except OSError:
     print("could not find Bus interface")
     exit()

try:
    while True:
        msg=can.Message(arbitration_id=0x123,data=[j,0x22,0x33,0x44,0x55,0x66,0x77,0x88],is_extended_id=False,is_fd=False)
        try:
            bus1.send(msg)
            print(f"message sent {msg}")
        except can.CanError:
            print("Error: Message not sent")
        mes = bus2.recv(timeout=10.0)
        if mes:
            print(f"message received: {mes}")
        else:
            print("no message received: ")
        j=j+0x01
        
except KeyboardInterrupt:
    os.system("sudo ip link set can0 down")
    os.system("sudo ip link set can1 down")
    print('\n\rKeyboard interrtupt')

    
