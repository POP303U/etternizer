#include "inputhandler.h"
#include <string.h>
#include "file.h"
#include "tools.h"

InputHandler *ValidateFiles(const char *path) {
    InputHandler *inputHandler = malloc(sizeof(InputHandler));

    if (inputHandler == NULL) {
        fprintf(stderr, "Couldn't allocate header");
        exit(1);
    }

    char *qua = FindQuaFile(path);

    if (!qua) {
        inputHandler->valid = 0;
        inputHandler->reason = "No Valid .qua file found!";
        return inputHandler;
    }

    char filepath[250];
    snprintf(filepath, sizeof(filepath), "%s%s%s", path, SEPERATOR, qua);

    char *data = LoadFile(filepath);

    inputHandler->chart      = strdup(filepath);
    inputHandler->audio      = ExtractValue(data, "AudioFile");
    inputHandler->BG         = ExtractValue(data, "BackgroundFile");
    inputHandler->BN         = ExtractValue(data, "BannerFile");

    // not smart using multiple snprintf calls
    snprintf(filepath, sizeof(filepath), "%s//%s", path, inputHandler->audio);
    if (!FileExists(filepath) && inputHandler->audio != NULL) {
        inputHandler->valid = 0;
        inputHandler->reason = "No valid audio file found!";
        return inputHandler;
    }

    snprintf(filepath, sizeof(filepath), "%s//%s", path, inputHandler->BG);
    if (!FileExists(filepath) && inputHandler->BG != NULL) {
        inputHandler->valid = 0;
        inputHandler->reason = "No valid background file found!";
        return inputHandler;
    }

    snprintf(filepath, sizeof(filepath), "%s//%s", path, inputHandler->BN);
    if (!FileExists(filepath) && inputHandler->BN != NULL) {
        inputHandler->valid = 0;
        inputHandler->reason = "No valid banner file found!";
        return inputHandler;
    }

    inputHandler->valid = 1;
    return inputHandler;
}