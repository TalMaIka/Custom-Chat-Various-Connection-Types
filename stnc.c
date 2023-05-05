#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char command[100];
    if(argc < 3 || argc > 4) {
        printf("The client side: stnc -c IP PORT\n");
        printf("The server side:stnc -s PORT\n");
        return 1;
    }
    if(argc == 3 && strcmp(argv[1], "-s") == 0) {
        sprintf(command, "./server %d", atoi(argv[2]));
        printf("%s\n", command);
        system(command);
        return 0;
    }
    if(argc == 4 && strcmp(argv[1], "-c") == 0) {
        sprintf(command, "./client %s %d", argv[2],atoi(argv[3]));
        printf("%s\n", command);
        system(command);
        return 0;
    }
    return 0;
}