#ifndef EXPORT_H
#define EXPORT_H

#define PATH_MAX 4096
#include "../quaver/parser.h"

int ExportSMChart(
    const char *quaverDir,
    const QuaverHeader *header,
    const char *smHeader,
    const char *smNotes
);

#endif