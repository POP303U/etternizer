#ifndef TOOLS_H
#define TOOLS_H

// Extracts a Double value from a yaml key
double ExtractDouble(const char *source, const char *key);

// Extracts a String value from a yaml key
char *ExtractValue(const char *source, const char *key);

// Get the name of the top most folder in a path
const char *GetFolderName(const char *path);

// Basic binary file copy
int CopyFile(const char *src, const char *dst);

#endif