//
// Created by robert on 11/21/24.
//
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "make_uuid.h"
void make_uuid(char** uuid_buffer)
{
    srand(time(NULL));

    // Generate four 32-bit random numbers
    unsigned int num1 = rand();
    unsigned int num2 = rand();
    unsigned int num3 = rand();
    unsigned int num4 = rand();

    // Convert the random numbers to a string
    char uuidStr[37];
    sprintf(*uuid_buffer, "%08x-%04x-%04x-%04x-%08x%04x",
            num1, num2 >> 16, num2 & 0xFFFF,
            num3 >> 16, num3 & 0xFFFF, num4);
}
