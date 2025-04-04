#ifndef Elec_Efficiency
#define Elec_Efficiency

float elec_efficiency(uint32_t mtr_volt, uint32_t mtr_curr, uint32_t fc_volt, uint32_t fc_curr, uint32_t fc_joules, uint32_t time1, uint32_t time2, float PreCal_EEff, float* instant_EEff, int* data_Ecnt);

#endif