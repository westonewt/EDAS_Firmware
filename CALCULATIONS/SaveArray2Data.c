// Name: SaveArray2Data
// Description: Saving program's array stored in memory into file in CSV format

// choice = 1 for Electrical Efficiency data
// choice = 2 for Fuel Efficiency data
// to write other data, add more choice values
/* ---------------------------------------------------------------------------- */
#include <stdio.h>



// main function
void saveArray2File (int choice, float *array,int arr_size, int data_points)
{
    FILE *fp;

    // Open or Create correct file to save relavant data into
    if (choice == 1)
    {
        fp = fopen("Elec_Eff.csv", "w");
        
    } else if (choice == 2) {
        fp = fopen("Fuel_Eff.csv", "w");
    } else {
        exit ();
    }

    // incase there is problem with opening said files
    if (fp == NULL)
    {
        printf("Error handling file.\n");
        exit(1);
    }
    
    // writing data through loop
    for (int i = 0; i < arr_size; i++)
    {
        for (int j = 0; j < data_points; j++)
            fprintf(fp, "%d,%0.3f,%0.3f\n", *(arr + i * cols + j));
        }
    }
    
    // clearning array data since data has been stored in file
    

    // closing appropriate written file
    fclose(fp);
}