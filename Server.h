#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct vehicle
{
    float gallons;
    int odometer;
    char id[101];   //Allow a maximum of 100 character string for an id (101 to include '\0')

} vehicle;

vehicle vehicleRecordtoStruct(char buffer[50]); //Take in a char[] containing a vehicle record and unpack and return the record as a vehicle struct

void printRecords(vehicle *vehiclePtr, int count);  //Takes a struct vehicle* and prints out "count" number of the records.
