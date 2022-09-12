#include "Server.h"

int main(int argc, char *argv[])
{
    /////////////////////////////////////////  Initialization  /////////////////////////////////////////////////////

    FILE *fptr;             //Used to read gasData
    vehicle *vehiclePtr;    //Array of vehicle structs
    char buffer[200];        //Buffer for reading in whole lines from gasData
    int count = 0;          //Variable to count total number of lines in gasData


    fptr = fopen("gasData","r");    //Open gasData for reading, if gasData is not found exit code 2
    if (fptr == NULL)
        exit(2);

    while(fgets(buffer, sizeof(buffer), fptr))  //Read gasData to completion and count the number of lines
        count++;

    fclose(fptr);
    vehiclePtr = (vehicle*)malloc(count * sizeof(vehicle)); //Use count to properly allocate array of vehicles

    fptr = fopen("gasData","r");
    if (fptr == NULL)
        exit(2);

    for(int i = 0; i < count; i++)
        {
            fgets(buffer, sizeof(buffer), fptr);
            vehiclePtr[i] = vehicleRecordtoStruct(buffer);
        }

    printRecords(vehiclePtr, count);
    /////////////////////////////////////////  Initialization Complete //////////////////////////////////////////////

    //Construct File Descriptors from Command Line Arguments (convert char[] to ints)
    int parentToChild[2];
    int childToParent[2];
    parentToChild[0] = strtol(argv[1], NULL, 10);
    parentToChild[1] = strtol(argv[2], NULL, 10);
    childToParent[0] = strtol(argv[3], NULL, 10);
    childToParent[1] = strtol(argv[4], NULL, 10);
    int BUFFER_SIZE = strtol(argv[5], NULL, 10);    //Macro defined in main file of calling process, passed through command line argument.

    //Close unused ends of the pipe.
    close(parentToChild[1]);
    close(childToParent[0]);
    char input[BUFFER_SIZE];    //Used to store input read from a pipe.
    char output[BUFFER_SIZE];   //Used to store output to read to a pipe.

    //Send initialization completion signal to Interface
    write(childToParent[1], output, BUFFER_SIZE);

    while(1)
    {
        char firstFour[5];  //Holds first four characters of a string. Size 5 to hold terminating "\0"
        char firstFive[6];  //Holds first five characters of a string. Size 6 to hold terminating "\0"

        read(parentToChild[0], input, BUFFER_SIZE); //Receive input from parent.

        //Populate firstFour based on input.
        if(strlen(input) >= 5)  //If input is at least 5 characters, collect them in firstFive.
        {
            for(int i = 0; i < 5; i++)
            {
                firstFive[i] = input[i];
            }
            firstFive[5] = '\0';
        }

        //Populate firstFive based on input.
        if(strlen(input) >= 4)   //If input is at least 4 characters, collect them in firstFour.
        {
            for(int i = 0; i < 4; i++)
            {
                firstFour[i] = input[i];
            }
            firstFour[4] = '\0';
        }
        else    //Input was less than four characters.
        {
            strcpy(firstFour, "");
            strcpy(firstFive, "");
        }


        //User entered "exit"
        if(strcmp(input, "exit") == 0)
        {
            int child_pid = getpid();
            char temp[20];
            snprintf(temp, sizeof(temp) - 1, "%d", child_pid);

            strcpy(output, "Child process (");
            strcat(output, temp);
            strcat(output, ") completed.");

            free(vehiclePtr);   //Deallocate vehiclePtr from heap.
            write(childToParent[1], output, BUFFER_SIZE);

            exit(0);
        }

        //User entered an mpg command.
        else if(strcmp(firstFour, "mpg,") == 0)
        {
            char* id = input + 4; //id is a string containing the id portion of the mpg command
            double total_gas_usage = 0; //Holds sum of all gas put into vehicle.
            double mpg = 0; //Storing final output of vehicles average mpg.
            int odometer_lowest = 2147483647;   //Stores lowest observed odometer reading. Initially begins as largest signed integer value.
            int odometer_highest = 0;   //Stores highest observed odometer reading. Initially begins at 0.
            int total_miles_driven = 0; //Holds total miles driven for a vehicle (odometer_highest - odometer_lowest)

            for(int i = 0; i < count; i++)  //For total number of vehicle records in the array
            {
                if(strcmp(vehiclePtr[i].id, id) == 0)   //If vehicle record matches id.
                {
                    if(vehiclePtr[i].odometer > odometer_highest)   //If record's odometer reading is the highest observed.
                        odometer_highest = vehiclePtr[i].odometer;

                    if(vehiclePtr[i].odometer < odometer_lowest)    //If record's odometer reading is the lowest observed.
                        odometer_lowest = vehiclePtr[i].odometer;

                    total_gas_usage += vehiclePtr[i].gallons;
                }
            }

            total_miles_driven = odometer_highest - odometer_lowest;
            if(total_gas_usage > 0) //Ensure tank was filled at least once to avoid divide by zero.
                mpg = (total_miles_driven / total_gas_usage);

            char temp1[50] = "Average MPG: ";
            char temp2[50];

            if(total_gas_usage > 0) //Event no gas was added to vehicle, mpg is unknown.
                snprintf(temp2, sizeof(temp2) - 1, "%f", mpg);
            else
                strcpy(temp2,"UNKNOWN");
            strcat(temp1, temp2);   //Concatenate both strings into temp 1;
            strcpy(output,temp1);   //Copy final output into output.

            write(childToParent[1], output, BUFFER_SIZE);   //Send output string.
            strcpy(output, "STOP READING");                 //Store termination signal into ouput
            write(childToParent[1], output, BUFFER_SIZE);   //Send termination signal.
        }

        //User entered a list command.
        else if(strcmp(firstFive, "list,") == 0)
        {
            char* id = input + 5; //id is a string containing id of list command.

            int *orderIndex;    //int array for ordering entries by their odometer reading.
            orderIndex = (int*)malloc(count * sizeof(int)); //allocate enough space for orderIndex to hold the index of every record, if necessary.
            int orderIndexSize = 0; //tracks the number of indexes stored in orderIndex array.

            //Add index of all records with matching id to the orderIndex
            for(int i = 0; i < count; i++)  //For every record in gasData...
            {
                if(strcmp(vehiclePtr[i].id, id) == 0)   //If record id matches command id.
                {
                    orderIndex[orderIndexSize] = i; //Add index of record to orderIndex.
                    orderIndexSize++;   //Update count of record indexes stored in orderIndex.
                }
            }

            int bucket; //Bucket for bubble sort swapping.
            //Bubble sort orderIndex by odometer ascending.
            for(int i = 0; i < orderIndexSize; i++) //For all elements in orderIndex...
            {
                for(int j = i + 1; j < orderIndexSize; j++) //For all elements after element "i"...
                {
                    //if odometer reading of i is greater than odometer reading of j, swap them.
                    if(vehiclePtr[orderIndex[i]].odometer > vehiclePtr[orderIndex[j]].odometer)
                    {
                        bucket = orderIndex[i];
                        orderIndex[i] = orderIndex[j];
                        orderIndex[j] = bucket;
                    }
                }
            }

            //Generate output for list command, record by record and write each record to pipe.
            for(int i = 0; i < orderIndexSize; i++)
            {
                char galStr[50];
                char odoStr[50];

                snprintf(galStr, sizeof(galStr) - 1, "%f", vehiclePtr[orderIndex[i]].gallons);
                snprintf(odoStr, sizeof(odoStr) - 1, "%d", vehiclePtr[orderIndex[i]].odometer);

                strcpy(output,"id = ");
                strcat(output, id);
                strcat(output, ", odometer = ");
                strcat(output, odoStr);
                strcat(output, ", gallons = ");
                strcat(output, galStr);

                write(childToParent[1], output, BUFFER_SIZE);
            }
            strcpy(output, "STOP READING"); //Store the termination signal in output.
            write(childToParent[1], output, BUFFER_SIZE);   //Send the termination signal.

            free(orderIndex);   //Deallocate orderIndex from heap.
        }

        //User entered an unknown command.
        else
        {
            strcpy(output, "Command \"");
            strcat(output, input);
            strcat(output, "\" was not recognized.");

            write(childToParent[1], output, BUFFER_SIZE);   //Send output across pipe.
            strcpy(output, "STOP READING");                 //Store the termination signal in output.
            write(childToParent[1], output, BUFFER_SIZE);   //Send the termination signal.
        }
    }
}

vehicle vehicleRecordtoStruct(char buffer[200])  //Receives a vehicle record as a \n terminated string and unpacks it into a vehicle struct.
{
    float gallons;
    int odometer;
    char id[100];

    char galBuff[20];
    char odoBuff[20];

    int i = 0, j = 0, k = 0, l = 0;

    for(; (int)buffer[i] != 32; i++, j++)  //Extract id from record
        {
            id[j] = buffer[i];
        }
        id[j] = '\0';   //Cap end of id with null terminator
        i++;

    for(; (int)buffer[i] != 32; i++, k++)  //Extract odometer from record
        {
            odoBuff[k] = buffer[i];
        }
        odoBuff[k] = '\0';  //Cap end of odometer with null terminator
        i++;

    for(; (int)buffer[i] != '\n'; i++, l++) //Extract gallons from record
        {
            galBuff[l] = buffer[i];
        }
        galBuff[l] = '\0';  //Cap end of gallons with null terminator

    gallons = atof(galBuff);    //Convert to float
    odometer = atof(odoBuff);   //Convert to integer

    vehicle newVehicle = {gallons, odometer , ""};  //newVehicle.id is initialized with empty string.
    strcpy(newVehicle.id,id);   //Use strcpy() to properly assign newVehicle.id the char[].
    return newVehicle;
};

void printRecords(vehicle *vehiclePtr, int count)
{
    for(int i = 0; i < count; i++)
    {
        fflush(stdout);
        printf("element = %d: id = %s, odometer = %d, gallons = %f\n", i, vehiclePtr[i].id, vehiclePtr[i].odometer, vehiclePtr[i].gallons);
    }

}

