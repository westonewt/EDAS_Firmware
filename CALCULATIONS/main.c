#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Calling .C files with functions in it
#include "Elec_Efficiency.h"
#include "Fuel_Efficiency.h"
#include "Fan_Control.h"

/*------Declaring variables by component------*/
// main.c variables
int array_size = 50;

// Raw data variables for program
uint32_t speed1, speed2, fuel1, fuel2;
uint32_t mtr_volt, mtr_curr, fc_volt, fc_curr, fc_joules;
uint32_t driver_temp;

// Calculated variables for program
float Fuel_Eff, Elec_Eff;
uint32_t fan_RPM;

/*-----------------Main Program---------------*/
int main()
{   
    // Bringing in CAN Bus raw data
    

    // Organize data
    for (int i = 0, )

    // Identify data


    // Sort Data
    

    // Calculate efficiency - instant and running average
    Fuel_Eff = fuel_efficiency(speed1, speed2, fuel1, fuel2, time1, time2, PreCal_FEff, inst_FEff, data_Fcnt);
    Elec_Eff = elec_efficiency(mtr_volt, mtr_curr, fc_volt, fc_curr, fc_joules, time1, time2, PreCal_EEff, instant_EEff, data_Ecnt);

    // Controlling driver fan
    fan_RPM = 

    // Storing data into file
    FILE* efw;


}