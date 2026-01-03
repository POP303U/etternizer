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

    // hardcoded png since quaver doesn't use those
    inputHandler->cdtitle = "assets/cdtitle.png";

    char *qua = FindQuaFile(path);

    if (!qua) {
        inputHandler->valid = 0;
        inputHandler->reason = "No Valid .qua file found!";
        return inputHandler;
    }

    char filepath[250];
    snprintf(filepath, sizeof(filepath), "%s%s%s", path, SEPERATOR, qua);

    char *data = LoadFile(filepath);
    inputHandler->chart = strdup(filepath);

    snprintf(filepath, sizeof(filepath), "%s%s%s", path, SEPERATOR, ExtractValue(data, "AudioFile"));
    if (ExtractValue(data, "AudioFile") == NULL) {
        inputHandler->audio = NULL;
    } else {
        inputHandler->audio = strdup(filepath);
    }

    snprintf(filepath, sizeof(filepath), "%s%s%s", path, SEPERATOR, ExtractValue(data, "BackgroundFile"));
    if (ExtractValue(data, "BackgroundFile") == NULL) {
        inputHandler->BG = NULL;
    } else {
        inputHandler->BG = strdup(filepath);
    }

    snprintf(filepath, sizeof(filepath), "%s%s%s", path, SEPERATOR, ExtractValue(data, "BannerFile"));
    if (ExtractValue(data, "BannerFile") == NULL) {
        inputHandler->BN = NULL;
    } else {
        inputHandler->BN = strdup(filepath);
    }

    // not smart using multiple snprintf calls
    if (!FileExists(inputHandler->audio) && inputHandler->audio != NULL) {
        inputHandler->valid = 0;
        inputHandler->reason = "No valid audio file found!";
        return inputHandler;
    }

    if (!FileExists(inputHandler->BG) && inputHandler->BG != NULL) {
        inputHandler->valid = 0;
        inputHandler->reason = "No valid background file found!";
        return inputHandler;
    }

    if (!FileExists(inputHandler->BN) && inputHandler->BN != NULL) {
        inputHandler->valid = 0;
        inputHandler->reason = "No valid banner file found!";
        return inputHandler;
    }

    inputHandler->valid = 1;
    return inputHandler;
}