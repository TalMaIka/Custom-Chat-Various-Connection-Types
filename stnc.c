#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char command[100];
    if (argc < 3 || argc > 8)
    {
        printf("The client side: stnc -c IP PORT\n");
        printf("The server side:stnc -s PORT\n");
        printf("The client side: stnc -c IP PORT -p <type> <param>\n");
        printf("The client side: stnc -c IP PORT -p <type> <param>\n");
        return 1;
    }
    int pFlag = 0;
    int qFlag = 0;
    int cFlag = 0;
    int sFlag = 0;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0)
        {
            pFlag = 1;
        }
        if (strcmp(argv[i], "-q") == 0)
        {
            qFlag = 1;
        }
        if (strcmp(argv[i], "-c") == 0)
        {
            cFlag = 1;
        }
        if (strcmp(argv[i], "-s") == 0)
        {
            sFlag = 1;
        }
    }
    if (cFlag == 1 && sFlag == 1)
    {
        printf("The client side: stnc -c IP PORT\n");
        printf("The server side:stnc -s PORT\n");
        printf("The client side: stnc -c IP PORT -p <type> <param>\n");
        printf("The client side: stnc -c IP PORT -p <type> <param>\n");
        return 1;
    }
    if (cFlag == 1 && sFlag == 0)
    {
        if (pFlag == 0)
        {
            sprintf(command, "./client %s %d", argv[2], atoi(argv[3]));
            system(command);
        }
        else
        {
            sprintf(command, "./client %s %d -p %s %s", argv[2], atoi(argv[3]), argv[5], argv[6]);
            system(command);
        }
    }
    if (cFlag == 0 && sFlag == 1)
    {
        if (pFlag == 0)
        {
            sprintf(command, "./server %d", atoi(argv[2]));
            system(command);
        }
        else
        {
            if (qFlag == 0)
            {
                sprintf(command, "./server %d -p", atoi(argv[2]));
                system(command);
            }
            else
            {
                sprintf(command, "./server %d -p -q", atoi(argv[2]));
                system(command);
            }
        }
    }

    return 0;
}