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
        printf("DEBUG: Failed to load file\n");
        return NULL;
    }

    char *hitObjectsPos = strstr(data, "HitObjects:");
    if (!hitObjectsPos) {
        printf("DEBUG: No HitObjects section found\n");
        free(data);
        return NULL;
    }
    printf("DEBUG: Found HitObjects section\n");

    char *line = strtok(hitObjectsPos, "\n");
    int lineCount = 0;
    
    while (line) {
        lineCount++;
        
        if (strstr(line, "- StartTime:")) {
            printf("DEBUG: Found StartTime line %d: %s\n", lineCount, line);
            
            QuaverNote note;
            note.startMs = atof(strchr(line, ':') + 1);
            note.endMs = -1.0;
            note.lane = -1;

            char *nextLine = strtok(NULL, "\n");
            if (nextLine && strstr(nextLine, "Lane:")) {
                note.lane = atoi(strchr(nextLine, ':') + 1) - 1; // 0-based
                printf("DEBUG: Found note at %f ms, lane %d\n", note.startMs, note.lane);
            } else {
                printf("DEBUG: No lane found for StartTime line\n");
                continue; // Skip notes without lanes
            }

            notes = realloc(notes, (*outCount + 1) * sizeof(QuaverNote));
            notes[*outCount] = note;
            (*outCount)++;
        }

        line = strtok(NULL, "\n");
    }

    printf("DEBUG: Total notes parsed: %zu\n", *outCount);
    free(data);
    return notes;
}

TimingPoint *ParseTimingPoints(const char *filepath, size_t *timingPointCount) {
    *timingPointCount = 0;
    TimingPoint *points = NULL;

    char *data = LoadFile(filepath);
    if (!data) {
        printf("DEBUG: Failed to load file for timing points\n");
        return NULL;
    }

    const char *pos = strstr(data, "TimingPoints:");
    if (!pos) {
        printf("DEBUG: No TimingPoints section found\n");
        free(data);
        return NULL;
    }
    printf("DEBUG: Found TimingPoints section\n");

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

        printf("DEBUG: Timing point %zu: %f ms, %f BPM, start beat %f\n", 
               *timingPointCount, startMs, bpm, points[*timingPointCount].startBeat);

        (*timingPointCount)++;
        pos = bpmPtr + 4;
    }

    printf("DEBUG: Total timing points parsed: %zu\n", *timingPointCount);
    free(data);
    return points;
}

double TimeToBeat(double timeMs, TimingPoint *timingPoints, size_t tpCount) {
    if (tpCount == 0) {
        return 0.0;
    }

    // Handle notes that occur before the first timing point
    if (timeMs < timingPoints[0].startMs) {
        // Calculate beats backwards from first timing point
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

    printf("DEBUG: Building measures for %zu notes with %zu timing points\n", noteCount, tpCount);
    
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
        
        if (i < 5) {
            printf("DEBUG: Note %zu: %f ms -> %f beats\n", i, notes[i].startMs, beat);
        }
    }
    
    // Calculate measure range
    int minMeasure = (int)floor(minBeat / 4.0);
    int maxMeasure = (int)floor(maxBeat / 4.0);
    
    // Ensure we have at least measure 0
    if (minMeasure < 0) minMeasure = 0;
    if (maxMeasure < 0) maxMeasure = 0;
    
    size_t measureCount = maxMeasure - minMeasure + 1;
    
    printf("DEBUG: Beat range: %f to %f\n", minBeat, maxBeat);
    printf("DEBUG: Measure range: %d to %d (count: %zu)\n", minMeasure, maxMeasure, measureCount);

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

        if (i < 5) {
            printf("DEBUG: Note %zu placed at measure %d, row %d, lane %d\n", 
                   i, measureIndex, row, notes[i].lane);
        }

        if (notes[i].lane >= 0 && notes[i].lane < 4) {
            measures[measureIndex].rows[row][notes[i].lane] = '1';
        }
    }

    *outMeasureCount = measureCount;
    return measures;
}

char *BuildSMNotes(Measure *measures, size_t measureCount) {
    char *out = malloc(1<<20); // Increased buffer size
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
