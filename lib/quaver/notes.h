#ifndef NOTES_H
#define NOTES_H

#include <stddef.h>

#define ROWS_PER_MEASURE 192  // SM standard: 4,8,12,16,24,32,48,64,96,192

typedef struct {
    char rows[ROWS_PER_MEASURE][5]; // 4 lanes + null terminator
} Measure;

typedef struct {
    double startMs;
    double endMs;
    int lane;
} QuaverNote;

typedef struct {
    double startMs;
    double bpm;
    double startBeat;
} TimingPoint;

// Parse note section from .qua
QuaverNote *ParseQuaverNotes(const char *filepath, size_t *outCount);

TimingPoint *ParseTimingPoints(const char *filepath, size_t *timingPointCount);

// Convert absolute timing into beat based measures
double TimeToBeat(double timeMs, TimingPoint *timingPoints, size_t tpCount);

// Build measures from notes to convert into string later
Measure *BuildMeasures(QuaverNote *notes, size_t noteCount, TimingPoint *tps, 
                       size_t tpCount, size_t *outMeasureCount);

// Convert all the created measures into a string of notes
char *BuildSMNotes(Measure *measures, size_t measureCount);

#endif