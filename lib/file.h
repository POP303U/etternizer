#ifndef FILE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define READ "r"

// Load a file into a buffer for processing
char *LoadFile(const char *filename);

// Check if a given file exists, returns error code 1/0
int FileExists(const char *path);

// Checks if the .qua file exists, cross-platform
char *FindQuaFile(const char *path);

#endif