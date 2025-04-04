// Assumptions are: 
// (1) speed value is directly presented by sensor
// (2) fuel amount is given in terms of remaining amount of energy -> in joules thus km/joule
// (3) time data is in uint32_t format
// ---------------------------
// Values returned directly: Overall Average Efficiency
// Values returned through pointers: Count and Instant Efficiency
/* ------------------------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdint.h>

// Acceptable time difference of incoming fuel data and speed data
float setD_Time = 0.5;

float fuel_efficiency(uint32_t speed1, uint32_t speed2, uint32_t fuel1, uint32_t fuel2, uint32_t speed_T1, uint32_t speed_T2, uint32_t fuel_T1, uint32_t fuel_T2, float PreCal_FEff, float* inst_FEff, int* data_Fcnt)
{
    // Data from CAN Bus
    int count = *data_Fcnt;

    // data processing variables
    float speed1_f = ((float) speed1) / 10000;
    float speed2_f = ((float) speed2) / 10000;
    
    float fuel1_f = ((float) fuel1) / 10000;
    float fuel2_f = ((float) fuel2) / 10000;

    float Tspeed_f1 = (float) speed_T1;
    float Tspeed_f2 = (float) speed_T2;

    float Tfuel_f1 = (float) fuel_T1;
    float Tfuel_f2 = (float) fuel_T2;
    
    float avg_speed, dist_traveled, fuel_used, time_diff;
    float instant_eff;
    float avg_FEff;

    // verifying if data coming in at the same time
    while (abs(Tspeed_f1 - Tfuel_f1) < setD_Time || abs(Tspeed_f2 - Tfuel_f2) < setD_Time)
    {
        // Basic calculations
        avg_speed = (speed1_f + speed2_f)/2;
        fuel_used = fuel1_f - fuel2_f;
        time_diff = Tspeed_f1 - Tspeed_f2;
        
        // Instant efficiency calculation
        dist_traveled = avg_speed * time_diff;
        instant_eff = dist_traveled/fuel_used;

        // Updating Average efficiency calculation
        avg_FEff = (PreCal_FEff * count + instant_eff)/(count + 1);
        
        // Updating other info through pointers
        (*data_Fcnt)++;
        *inst_FEff = instant_eff;

        return avg_FEff;
    }

    // if "while" condition above not met
    exit ();
}