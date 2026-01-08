#include <stdio.h>
#include <stdlib.h>
#include "lib/quaver/notes.h"
#include "lib/inputhandler.h"
#include "lib/etterna/header.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "USAGE: ./etternizer <directory>\n");
        return 1;
    }

    InputHandler *inputHandler = ValidateFiles(argv[1]);
    if (!inputHandler->valid) {
        fprintf(stderr, "%s", inputHandler->reason);
        return 1;
    }

    // Parse .qua header
    QuaverHeader *quaverHeader = ParseQuaverHeader(inputHandler->chart);
    if (!quaverHeader) {
        fprintf(stderr, "Failed to parse .qua header.\n");
        return 1;
    }

    // Parse timing points
    size_t timingPointCount = 0;
    TimingPoint *timingPoints = ParseTimingPoints(inputHandler->chart, &timingPointCount);
    if (!timingPoints) {
        fprintf(stderr, "Failed to parse timing points.\n");
        FreeQuaverHeader(quaverHeader);
        return 1;
    }

    // Parse hit objects / notes
    size_t noteCount = 0;
    QuaverNote *notes = ParseQuaverNotes(inputHandler->chart, &noteCount);
    if (!notes || noteCount == 0) {
        fprintf(stderr, "Failed to parse notes.\n");
        FreeQuaverHeader(quaverHeader);
        free(timingPoints);
        return 1;
    }

    // DEBUG: print parsed notes
    printf("Parsed %zu notes:\n", noteCount);
    for (size_t i = 0; i < noteCount; i++) {
        printf("Note %2zu: start=%.2f ms, lane=%d\n", i, notes[i].startMs, notes[i].lane);
    }

    // Build measures
    size_t measureCount = 0;
    Measure *measures = BuildMeasures(notes, noteCount, timingPoints, timingPointCount, &measureCount);

    // Build StepMania header and note body
    char *smHeader = BuildSMHeader(quaverHeader);
    char *smNotes = BuildSMNotes(measures, measureCount);

    printf("%s", smHeader);
    printf("%s", smNotes);

    // Free memory
    free(smHeader);
    free(smNotes);
    free(measures);
    free(notes);
    free(timingPoints);
    FreeQuaverHeader(quaverHeader);

    return 0;
}
