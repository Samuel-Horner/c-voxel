#ifndef SHADER
#define SHADER

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

char* getShaderSource(const char *file_name){
    FILE* file = fopen(file_name, "r");
    if (file == NULL) { 
        int errsv = errno;
        printf("Error: %s not opened. (%d: %s)\n", file_name, errsv, strerror(errsv)); 
        return NULL; 
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_contents = malloc(file_size + 1);
    if (file_contents == NULL) { return NULL; }

    fread(file_contents, file_size, 1, file);
    file_contents[file_size] = '\0'; // Ensure null-termination
    printf("[Loaded file: %s]\n", file_name);

    unsigned int null_terminated = 0;
    for (unsigned int i = 0; i <= file_size; i++) {
        if (file_contents[i] == '\0') { null_terminated = 1; break;}
    }

    if (!null_terminated) {
        printf("File not null terminated!\n");
        return NULL;
    }

    return file_contents;
}

#endif
