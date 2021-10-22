#include <stdio.h>
#include <stdlib.h>
#include "processus.h"
//TURC Julien
//ZAETTA Lucas
int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr,"usage : ./monprojet int\n");
        exit(-1);
    }
    int n = atoi(argv[1]);
    if (n>=2)
    {
        controller(n);
    } else {
        fprintf(stderr,"Usage: ./monprojet int (>1)\n");
        return -1;
    }
    return 0;
}