#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE *file;

// did user supply a file ?
int isFile(char fileName[]){
    //file = fopen(argv[1], "r");
    printf("%s\n", fileName);

    return 0;
}


int main(int argc, char *argv[]){

    isFile("Text");
    return 0;
}
