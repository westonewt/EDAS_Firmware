#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Calling .C files with functions in it
#include "Elec_Efficiency.h"
#include "Fuel_Efficiency.h"
#include "Fan_Control.h"
#include "typedefs.h"

/***************************************************************/
/*---------------Declaring variables by component--------------*/
uint8_t ProgStarted = 0;

// raw data temporary store
uint32_t    mtr_volt, mtr_curr,                                 // MTR = motor
            mtrV_time, mtrC_time,
            fc_volt, fc_curr,                                   // FC = fuel cell
            fcV_time, fcI_time,
            fc_joules, fcE_time;                                // remaining amount of fuel in tank - unit joules
uint32_t driver_temp, driver_humid;
rData SpeedVal[2], fuelVal[2];

// function to initialize values so calculation can start now
void initializeSpeedVal() {
    SpeedVal[0].time    = 0.0;
    SpeedVal[1].time    = 0.0;
    fuelVal[0].time     = 0.0;
    fuelVal[1].time     = 0.0;

    SpeedVal[0].value   = 0;
    SpeedVal[1].value   = 0;
    fuelVal[0].value    = 0;
    fuelVal[1].value    = 0;

    data_Fcnt = 0;
    PreCal_FEff = 0;
}

/*-------------------------------------------------------------*/
// storing calculated data from program
dataStruct Fuel_Eff, Elec_Eff;
uint32_t fan_RPM;
float PreCal_FEff = 0;

// for storing calculated running average values 
float PreCal_FEff;
float *inst_FEff;
int data_Fcnt;

float PreCal_EEff;
float inst_EEff;
int data_Ecnt;

// control when array filing happens -> for file storage
#define array_Limit = 50;

// FEff = Fuel Efficiency and EEff = Electrical Efficiency
int array_Count_FEff, array_Count_EEff;

// count for keeping track of data size
int EE_cnt = 0;
int FE_cnt = 0;

/***************************************************************/
/*--------------------------Main Program-----------------------*/
int main()
{   
    // initializing to prevent random data
    while (ProgStarted == 0)
    {
        initializeSpeedVal();
        ProgStarted = 1;
    }
    
    // Fan control in RPM
    uint32_t FANspeed = Fan_Ctrl (driver_temp, driver_humid);

    // Calculate efficiency - instant and running average
    Fuel_Eff.value = fuel_efficiency(   SpeedVal[0].value, 
                                        SpeedVal[1].value, 
                                        fuelVal[0].value, 
                                        fuelVal[1].value, 
                                        SpeedVal[0].time,
                                        SpeedVal[1].time,
                                        fuelVal[0].time,
                                        fuelVal[1].time,
                                        PreCal_FEff, &inst_FEff, data_Fcnt);
    
    Elec_Eff.value = elec_efficiency(   mtr_volt, mtr_curr, fc_volt, fc_curr, fc_joules,
                                        mtrV_time, fcV_time, PreCal_EEff, &inst_EEff, data_Ecnt);

    // Controlling driver fan
    fan_RPM = Fan_Ctrl(driver_temp, driver_humid);

    // Storing data into file
    while (array_Count < array_Limit)
    {
        File_Array(*);
    }
}