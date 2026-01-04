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

    // find the HitObjects section
    char *hitObjectsPos = strstr(data, "HitObjects:");
    if (!hitObjectsPos) {
        free(data);
        return NULL;
    }

    // only parse lines after HitObjects (fixes reading bpm as hitobject)
    char *line = strtok(hitObjectsPos, "\n");
    while (line) {
        if (strstr(line, "- StartTime:")) {
            QuaverNote note;
            note.startMs = atof(strchr(line, ':') + 1);
            note.endMs = -1.0;
            note.lane = -1;

            // next line should contain Lane
            char *nextLine = strtok(NULL, "\n");
            if (nextLine && strstr(nextLine, "Lane:")) {
                note.lane = atoi(strchr(nextLine, ':') + 1) - 1; // 0-based
            }

            notes = realloc(notes, (*outCount + 1) * sizeof(QuaverNote));
            notes[*outCount] = note;
            (*outCount)++;
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

    double lastMs = 0;
    double lastBpm = 0;
    double startBeat = 0;

    while (1) {
        const char *startMsPtr = strstr(pos, "StartTime:");
        const char *bpmPtr = strstr(pos, "Bpm:");

        if (!startMsPtr || !bpmPtr) {
            break;
        }

        double startMs = atof(startMsPtr + strlen("StartTime:"));
        double bpm = atof(bpmPtr + strlen("Bpm:"));

        if (*timingPointCount > 0) {
            double deltaSec = (startMs - lastMs) / 1000.0;
            startBeat += deltaSec * (lastBpm / 60.0);
        }

        points = realloc(points, (*timingPointCount + 1) * sizeof(TimingPoint));
        points[*timingPointCount].startMs = startMs;
        points[*timingPointCount].bpm = bpm;
        points[*timingPointCount].startBeat = startBeat;

        lastMs = startMs;
        lastBpm = bpm;
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

    TimingPoint *tp = &timingPoints[0];

    for (size_t i = 0; i < tpCount; i++) {
        if (i + 1 < tpCount && timeMs >= timingPoints[i + 1].startMs) {
            continue;
        }

        tp = &timingPoints[i];
        break;
    }

    double deltaSec = (timeMs - tp->startMs) / 1000.0;
    double deltaBeats = deltaSec * (tp->bpm / 60.0);

    return tp->startBeat + deltaBeats;
}

Measure *BuildMeasures(QuaverNote *notes, size_t noteCount,
                       TimingPoint *timingPoints, size_t tpCount,
                       size_t *outMeasureCount) {

    size_t maxMeasure = 0;

    for (size_t i = 0; i < noteCount; i++) {
        double beat = TimeToBeat(notes[i].startMs, timingPoints, tpCount);
        size_t measure = (size_t) floor(beat / 4.0);

        if (measure > maxMeasure) {
            maxMeasure = measure;
        }
    }

    Measure *measures = calloc(maxMeasure + 1, sizeof(Measure));

    for (size_t m = 0; m <= maxMeasure; m++) {
        for (int r = 0; r < ROWS_PER_MEASURE; r++) {
            strcpy(measures[m].rows[r], "0000");
        }
    }

    for (size_t i = 0; i < noteCount; i++) {
        double beat = TimeToBeat(notes[i].startMs, timingPoints, tpCount);
        size_t measure = (size_t) floor(beat / 4.0);
        double beatInMeasure = beat - (measure * 4.0);

        int row = (int) round((beatInMeasure / 4.0) * ROWS_PER_MEASURE);
        if (row >= ROWS_PER_MEASURE) {
            row = ROWS_PER_MEASURE - 1;
        }

        measures[measure].rows[row][notes[i].lane] = '1';
    }

    *outMeasureCount = maxMeasure + 1;
    return measures;
}

char *BuildSMNotes(Measure *measures, size_t measureCount) {
    char *out = malloc(1<<16);
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
