#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#ifdef _WIN32
    #define SEPERATOR "\\"
#else
    #define SEPERATOR "/"
#endif

typedef struct {
    int valid;     // Checks if folder structure is invalid and exists 1/0
    char *reason;  // reason the processing failed

    char *chart;   // .qua (Chart)
    char *BG;      // Background (png)
    char *BN;      // Banner (png)
    char *audio;   // Audio (any format)
    char *cdtitle; // not used (default is already given)
} InputHandler;

// Checks if the folder structure is valid and returns an error code
InputHandler *ValidateFiles(const char *path);

#endif