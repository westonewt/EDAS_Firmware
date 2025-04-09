// Assuming all the data is coming in through an array of data
// IncomingDATA is the pointer pointing to where the data is 
#include <stdio.h>
#include <stdint.h>

void CAN_sort(  uint32_t timeD, uint32_t *IncomingDATA, ExtractedDATA *CANdata, 
                uint32_t *mtrV_time, uint32_t *mtr_volt, uint32_t *mtr_curr, uint32_t *mtrC_time,
                uint32_t *fcE_time, uint32_t *fc_joules, uint32_t *fcV_time, uint32_t *fcI_time,
                uint32_t *fc_volt, uint32_t *fc_curr, uint32_t *drv_temp, uint32_t *drv_humd,
                uint32_t *SpeedVal, int *H2_Alarm)
{    
    // Assumming CAN 2.0A standard thus 11 bit identifier
    // "identifier" is 2 array uint8_t
    CANdata -> identifier = ((IncomingDATA[0] & 0x007F) << 4) | (IncomingDATA[1] & 0x00F0);

    // setting up filter for accepting data
    if (CANdata -> identifier == 0x0010) {                                              // indicating H2 alarm has been triggered
        *H2_Alarm = IncomingDATA;
    
    } else if (CANdata -> identifier == 0x0150) {                                       // ID to get motor voltage and current
        *mtrV_time = *mtrC_time = timeD;
        *mtr_volt = IncomingDATA[1];
        *mtr_curr = IncomingDATA[0];
    
    } else if (CANdata -> identifier == 0x0140) {                                       // ID to get fuel cell energy amount left
        *fcE_time = timeD;
        *fc_joules = IncomingDATA[1];
        // dont need Capacitor energy level in IncomingDATA[0];
    
    } else if (CANdata -> identifier == 0x0170) {                                       // ID to get fuel cell voltage and current
        *fcV_time = *fcI_time = timeD;
        *fc_volt = IncomingDATA[1];
        *fc_curr = IncomingDATA[0];
    
    } else if (CANdata -> identifier == 0x0220) {
        *drv_temp = IncomingDATA[1];
        *drv_humd = IncomingDATA[0];
    
    } else if (CANdata -> identifier == 0x0990) {
        *SpeedVal[0] -> time = timeD;
        *SpeedVal[0] -> value = IncomingDATA[0];
    }
    
}









/******************************************************************************************/
// draft code
/* 
    // for CAN FD
    size_t byte2Extract;
    switch (CAN_data_length)
    {
        case 0:
            byte2Extract = 0;
            break;

        case 1:
            byte2Extract = 1;
            break;

        case 2:
            byte2Extract = 2;
            break;
            
        case 3:
            byte2Extract = 3;
            break;
        
        case 4:
            byte2Extract = 4;
            break;
        
        case 5:
            byte2Extract = 5;
            break;

        case 6:
            byte2Extract = 6;
            break;

        case 7:
            byte2Extract = 7;
            break;

        case 8:
            byte2Extract = 8;
            break;

        case 9:
            byte2Extract = 12;
            break;
        
        case 10:
            byte2Extract = 16;
            break;

        case 11:
            byte2Extract = 20;
            break;

        case 12:
            byte2Extract = 24;
            break;

        case 13:
            byte2Extract = 32;
            break;

        case 14:
            byte2Extract = 48;
            break;

        case 15:
            byte2Extract = 64;
            break;
    }

    size_t bit2Extract = byte2Extract * 8;
    data -> WRK_data_size = byte2Extract;

    // Allocating 1 byte more than required memory space
    size_t byteSpace = byte2Extract + 1;
    data -> WRK_data = (uint8_t *) malloc (byteSpace);

    for (size_t i = 0; i < byteSpace; i++)
    {
        data -> WRK_data[i] = IncomingDATA[2 + i];
    }
    */