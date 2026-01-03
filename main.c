#include <stdio.h>
#include <stdlib.h>
#include "lib/etterna/header.h"
#include "lib/quaver/parser.h"
#include "lib/inputhandler.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "USAGE: ./etternizer <directory>\n");
        return 1;
    }

    InputHandler *inputHandler = ValidateFiles(argv[1]);

    // Descriptive error handling
    if (!inputHandler->valid) {
        fprintf(stderr, "%s", inputHandler->reason);
        return 1;
    }

    QuaverHeader *quaverHeader;

    // parse .qua header values into a struct
    quaverHeader = ParseQuaverHeader(inputHandler->chart);

    // insert values into stepmania header
    char *header = BuildSMHeader(quaverHeader);

    printf("%s", header);

    free(header);
    FreeQuaverHeader(quaverHeader);
}
