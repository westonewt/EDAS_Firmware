// Assumptions are: 
// (1) speed value is directly presented by sensor
// (2) time data is in uint32_t format
// ---------------------------
// Value returned directly: average electrical efficiency
// Values returned through pointers: Count and Instant electrical Efficiency
/* ------------------------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdint.h>

// Acceptable time difference of incoming fuel data and speed data
float setD_Time = 0.5;

// main program to calculate electrical efficiency
float elec_efficiency(uint32_t mtr_volt, uint32_t mtr_curr, uint32_t fc_volt, uint32_t fc_curr, uint32_t fc_joules, uint32_t time_MTR, uint32_t time_FC, float PreCal_EEff, float* instant_EEff, int* data_Ecnt)
{
    // keeping count for data storage
    int count = *data_Ecnt;

    // data processing variables
    // motor voltage and current
    float mtr_V = ((float) mtr_volt) / 10000;
    float mtr_I = ((float) mtr_curr) / 10000;

    // fuel cell voltage and current
    float fc_V = ((float) fc_volt) / 10000;
    float fc_I = ((float) fc_curr) / 10000;

    // energy in Joules
    float fc_E = ((float) fc_joules) / 10000;

    // making sure time of data is within reason
    float timeMT_f = (float) time_MTR;
    float timeFC_f = (float) time_FC;

    // for basic calculations
    float mtr_PWR, fc_PWR, elec_Eff;
    float avg_EEff;

    float instant_eff;

    // "while" loop to make sure data are within acceptable time frame since most data wont be synchronized
    while (abs(timeMT_f - timeFC_f) < setD_Time)
    {
        // Pre-calculation of powers
        mtr_PWR = mtr_V * mtr_I;
        fc_PWR = fc_V * fc_I;

        // Instant efficiency calculation
        instant_eff = mtr_PWR / fc_PWR;

        // Updating Average efficiency calculation
        avg_EEff = (PreCal_EEff * count + instant_eff)/(count + 1);
        
        // Updating other info through pointers
        (*data_Ecnt)++;
        *instant_EEff = instant_eff;

        return avg_EEff;
    }
    
    // if "while" condition not met, then return 0
    exit ();
}