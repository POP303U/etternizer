#include "file.h"

#include <string.h>

int FileExists(const char *path) {
    FILE *file = fopen(path, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

char *LoadFile(const char *filename) {
    FILE *fileptr = fopen(filename, READ);

    if (fileptr == NULL) {
        exit(-1);
    }

    // Get the file size
    fseek(fileptr, 0L, SEEK_END);
    long sz = ftell(fileptr);
    fseek(fileptr, 0L, SEEK_SET);
    rewind(fileptr);

    // Read into buf until eof
    char *buffer = malloc(sz);
    fread(buffer, sz, 1, fileptr);
    fclose(fileptr);

    if (!buffer) {
        return NULL;
    }

    return buffer;
}

#ifdef _WIN32
#include <windows.h>

char *FindQuaFile(const char *path) {
    char search[MAX_PATH];
    snprintf(search, MAX_PATH, "%s\\*.qua", path);

    WIN32_FIND_DATA finddata;
    HANDLE handle = FindFirstFile(search, &finddata);
    if (handle == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    char *out = _strdup(finddata.cFileName);
    FindClose(handle);
    return out;
}

#else  // POSIX
#include <glob.h>

char *FindQuaFile(const char *path) {
    char pattern[1024];
    snprintf(pattern, sizeof(pattern), "%s/*.qua", path); // POSIX path

    glob_t glob;
    if (glob(pattern, 0, NULL, &glob) != 0 || glob.gl_pathc == 0) {
        globfree(&glob);
        return NULL;
    }

    char *out = strdup(glob.gl_pathv[0]);
    globfree(&glob);
    return out;
}

#endif