#include "notes.h"
#include "../file.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

QuaverNote *ParseQuaverNotes(const char *filepath, size_t *outCount) {
    *outCount = 0;
    QuaverNote *notes = NULL;

    char *data = LoadFile(filepath);
    if (!data) {
        return NULL;
    }

    char *hitObjectsPos = strstr(data, "HitObjects:");
    if (!hitObjectsPos) {
        free(data);
        return NULL;
    }

    char *line = strtok(hitObjectsPos, "\n");
    
    while (line) {
        if (strstr(line, "- StartTime:")) {
            QuaverNote note;
            note.startMs = atof(strchr(line, ':') + 1);
            note.endMs = -1.0;
            note.lane = -1;

            char *nextLine = strtok(NULL, "\n");
            if (nextLine && strstr(nextLine, "Lane:")) {
                note.lane = atoi(strchr(nextLine, ':') + 1) - 1; // 0-based
                
                notes = realloc(notes, (*outCount + 1) * sizeof(QuaverNote));
                notes[*outCount] = note;
                (*outCount)++;
            }
            // Skip notes without lanes - don't print anything
        }

        line = strtok(NULL, "\n");
    }

    free(data);
    return notes;
}

TimingPoint *ParseTimingPoints(const char *filepath, size_t *timingPointCount) {
    *timingPointCount = 0;
    TimingPoint *points = NULL;

    char *data = LoadFile(filepath);
    if (!data) {
        return NULL;
    }

    const char *pos = strstr(data, "TimingPoints:");
    if (!pos) {
        free(data);
        return NULL;
    }

    pos += strlen("TimingPoints:");

    while (1) {
        const char *startMsPtr = strstr(pos, "StartTime:");
        const char *bpmPtr = strstr(pos, "Bpm:");

        if (!startMsPtr || !bpmPtr) {
            break;
        }

        double startMs = atof(startMsPtr + strlen("StartTime:"));
        double bpm = atof(bpmPtr + strlen("Bpm:"));

        points = realloc(points, (*timingPointCount + 1) * sizeof(TimingPoint));
        points[*timingPointCount].startMs = startMs;
        points[*timingPointCount].bpm = bpm;
        
        if (*timingPointCount == 0) {
            points[*timingPointCount].startBeat = 0.0;
        } else {
            TimingPoint *prevTp = &points[*timingPointCount - 1];
            double deltaMs = startMs - prevTp->startMs;
            double deltaBeats = (deltaMs / 1000.0) * (prevTp->bpm / 60.0);
            points[*timingPointCount].startBeat = prevTp->startBeat + deltaBeats;
        }

        (*timingPointCount)++;
        pos = bpmPtr + 4;
    }

    free(data);
    return points;
}

double TimeToBeat(double timeMs, TimingPoint *timingPoints, size_t tpCount) {
    if (tpCount == 0) {
        return 0.0;
    }

    // Handle notes that occur before the first timing point
    if (timeMs < timingPoints[0].startMs) {
        double deltaMs = timingPoints[0].startMs - timeMs;
        double deltaBeats = (deltaMs / 1000.0) * (timingPoints[0].bpm / 60.0);
        return timingPoints[0].startBeat - deltaBeats;
    }

    // Find the correct timing point - the last one that starts at or before timeMs
    TimingPoint *tp = &timingPoints[0];
    
    for (size_t i = 1; i < tpCount; i++) {
        if (timingPoints[i].startMs <= timeMs) {
            tp = &timingPoints[i];
        } else {
            break;
        }
    }

    // Calculate beats from this timing point
    double deltaMs = timeMs - tp->startMs;
    double deltaBeats = (deltaMs / 1000.0) * (tp->bpm / 60.0);

    return tp->startBeat + deltaBeats;
}

Measure *BuildMeasures(QuaverNote *notes, size_t noteCount,
                       TimingPoint *timingPoints, size_t tpCount,
                       size_t *outMeasureCount) {

    if (noteCount == 0) {
        *outMeasureCount = 1;
        Measure *measures = calloc(1, sizeof(Measure));
        for (int r = 0; r < ROWS_PER_MEASURE; r++) {
            strcpy(measures[0].rows[r], "0000");
        }
        return measures;
    }

    // Find the range of beats to determine measures needed
    double minBeat = 0.0;
    double maxBeat = 0.0;
    
    for (size_t i = 0; i < noteCount; i++) {
        double beat = TimeToBeat(notes[i].startMs, timingPoints, tpCount);
        
        if (i == 0) {
            minBeat = maxBeat = beat;
        } else {
            if (beat < minBeat) minBeat = beat;
            if (beat > maxBeat) maxBeat = beat;
        }
    }
    
    // Calculate measure range
    int minMeasure = (int)floor(minBeat / 4.0);
    int maxMeasure = (int)floor(maxBeat / 4.0);
    
    // Ensure we have at least measure 0
    if (minMeasure < 0) minMeasure = 0;
    if (maxMeasure < 0) maxMeasure = 0;
    
    size_t measureCount = maxMeasure - minMeasure + 1;

    Measure *measures = calloc(measureCount, sizeof(Measure));

    // Initialize all rows
    for (size_t m = 0; m < measureCount; m++) {
        for (int r = 0; r < ROWS_PER_MEASURE; r++) {
            strcpy(measures[m].rows[r], "0000");
        }
    }

    // Place notes
    for (size_t i = 0; i < noteCount; i++) {
        double beat = TimeToBeat(notes[i].startMs, timingPoints, tpCount);
        int measureIndex = (int)floor(beat / 4.0) - minMeasure;
        
        // Skip notes that fall outside our measure range
        if (measureIndex < 0 || measureIndex >= (int)measureCount) {
            continue;
        }
        
        double beatInMeasure = beat - ((int)floor(beat / 4.0) * 4.0);
        
        int row = (int)round((beatInMeasure / 4.0) * ROWS_PER_MEASURE);
        if (row >= ROWS_PER_MEASURE) {
            row = ROWS_PER_MEASURE - 1;
        }
        if (row < 0) {
            row = 0;
        }

        if (notes[i].lane >= 0 && notes[i].lane < 4) {
            measures[measureIndex].rows[row][notes[i].lane] = '1';
        }
    }

    *outMeasureCount = measureCount;
    return measures;
}

char *BuildSMNotes(Measure *measures, size_t measureCount) {
    char *out = malloc(1<<20);
    out[0] = '\0';

    for (size_t m = 0; m < measureCount; m++) {
        for (int r = 0; r < ROWS_PER_MEASURE; r++) {
            strcat(out, measures[m].rows[r]);
            strcat(out, "\n");
        }

        if (m != measureCount - 1) {
            strcat(out, ",\n");
        } else {
            strcat(out, ";\n");
        }
    }

    return out;
}
