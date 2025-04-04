// Program to control Fan speed inside driver cockpit
// Assuming both fan speed will be set to same RPM

#include <stdio.h>
#include <stdint.h>

// setting up max fan speed
uint32_t FAN_maxRPM = 1000;

// unified fan speed - change if fan is in different area
uint32_t FAN;

uint32_t Fan_Ctrl(uint32_t driver_temp, uint32_t driver_humid)
{
    // calculate Humidity Index value for temperature felt by driver
    float HI = 1.98 * driver_temp + 0.047 * driver_humid + 24.9;

    // controlling fan based on humidity index
    // assuming max acceptable temperature is 32C
    if (HI > 32)
    {
        // blast both fans - assuming 1000 is max rpm
        FAN = FAN_maxRPM;

    } else if (HI > 27 && HI <= 32) {
        // have fan run at 50% max speed
        FAN = FAN_maxRPM / 2;

    } else if (HI > 23 && HI <= 27) {
        // run both fans at 25% max speed
        FAN = FAN_maxRPM / 4;

    } else {
        // turn OFF fan if temperature low or HI value is invalid
        FAN = 0;

    }

    return FAN;
}