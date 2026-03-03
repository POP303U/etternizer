#ifndef EXPORT_H
#define EXPORT_H

// Create my own PATH_MAX manually because the default is too small
#define MAX_PATH 1024
#include "../quaver/parser.h"

int ExportSMChart(
    const char *quaverDir,
    const QuaverHeader *header,
    const char *smHeader,
    const char *smNotes
);

#endif