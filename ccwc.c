// Include the standard input/output library for file operations
#include <stdio.h>

// Include the standard library for memory allocation functions
#include <stdlib.h>

// The main function - where the program starts execution
int main(int argc, char *argv[]) {

    int opt;

    for (int i = 1; i < argc; i++) {
        printf("Argument %d: %s\n", i, argv[i]);
    }

    // Declare a FILE pointer to hold a reference to the opened file
    // "FILE *" is the type for file handles in C
    FILE *file = fopen("test.txt", "rb");

    // Check if fopen() failed (returns NULL on error)
    // This handles cases like file not found or permission denied
    if (!file) {
        // Print an error message to the user
        // perror() prints both the custom message and the system error
        perror("Error opening file");
        // Return 1 to indicate the program failed
        return 1;
    }

    // Move to end of file
    fseek(file, 0, SEEK_END);

    // Get current position (which is now the file size)
    long file_size = ftell(file);

    //printf("File size: %ld bytes\n", file_size);

    fclose(file);

    // Return 0 to indicate the program succeeded
    return 0;
}
