int data_sort (int data_in, float *speed_data)
{
    // Creating masks for all the addresses
    int speed_mask = 0b100000001111;
    int fuel_mask = 0b100000001011;

    // If data is done in batches, uncomment below
    /* for (int i = 0; i <= data_size; i++) */
    
    // Compare mask with data
    if (data_in & speed_mask) {
        // returning 1 indicates data is speed
        return 1;
    } else if (data_in & fuel_mask) {
        // returning 2 indicates data is fuel
        return 2;
    } else {
        // returning 0 indicates data is irrelavant
        return 0;
    }
}