import can
import time
import gpiozero
import os

#ID's for ECOCAR CAN Bus
H2_ALARM=0x001
FD_FETPACK_ID=0x010
FD_RELPACKMTR_ID=0x015
FDCAN_RELPACK_ID=0x016
FDCAN_RELPACKFC_ID=0x017
FDCAN_FCCPACK1_ID=0x020
FDCAN_FCCPACK2_ID=0x021
FDCAN_FCCPACK3_ID=0x022
ECOCAN_H2_PACK1_ID=0x030
ECOCAN_H2_PACK2_ID=0x031
ECOCAN_H2_ARM_ALARM_ID=0x032
FDCAN_BOOSTPACK_ID=0x040
FDCAN_BOOSTPACK2_ID=0x041
FDCAN_BATTPACK_ID=0x050

#set up CAN0 with a bitrate of 500kbps and a data bitrate of 2000kbps
os.system("sudo ip link set can0 up type can bitrate 1000000 fd off")

#bring up CAN0 interface
try:
    bus1=can.interface.Bus(channel='can0',bustype='socketcan',fd=False)
    print("Bus interface connected")
except:
    print("could not find Bus interface")
    exit()

#loop for receiving messages
try:
    while True:
        message = bus1.recv(timeout=10)
        if message:
            c = '{0:f} {1:x} {2:x} {3:x} {4:x} '.format(message.timestamp, message.is_fd,message.bitrate_switch,message.arbitration_id, message.dlc)
            s=''
            if message.arbitration_id==0x001:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("H2_ALARM message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x010:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("FETPACK message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x015:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("RELPACKMTR message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x016:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("RELPACK message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x017:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("RELPACKFC message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x020:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("FCCPACK1 message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x021:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("FCCPACK2 message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x022:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("FCCPACK3 message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x030:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("H2_PACK1 message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x031:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("H2_PACK2 message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x032:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("H2_ARM_ALARM message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x040:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("BOOSTPACK message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x041:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("BOOSTPACK2 message:")
                print(' {}'.format(c+s))
            if message.arbitration_id==0x050:
                for i in range(message.dlc ):
                    s +=  '{0:x} '.format(message.data[i])
                print("BATTPACK message:")
                print(' {}'.format(c+s))
            
        else:
            print("no message received: ")     
        time.sleep(0.1)

#keyboard interrupt to break out of infinte loop
except KeyboardInterrupt:
    os.system("sudo ip link set can0 down")
    print('\n\rKeyboard interrtupt')
