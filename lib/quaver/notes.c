#include "notes.h"
#include <stdlib.h>

QuaverNote *ParseQuaverNotes(const char *data, size_t *outCount) {
    *outCount = 0;
    QuaverNote *notes = NULL;

    const char *pos = strstr(data, "HitObjects:");
    if (!pos) {
        return NULL;
    }

    pos += strlen("HitObjects:");

    while (1) {
        const char *startTime = strstr(pos, "StartTime:");
        const char *lane = strstr(pos, "Column");

        if (!startTime || !lane) {
            break;
        }

        // this will be parsed into an array of notes to be returned
        QuaverNote note;

        note.startMs = atof(startTime + strlen("StartTime:"));
        note.endMs = -1.0;

        // LNs
        const char *endTime = strstr(pos, "EndTime");
        if (endTime && endTime < (lane + 100)) {
            note.endMs = atof(endTime + strlen("EndTime:"));
        }

        note.lane = atoi(lane + strlen("Column:"));

        // pointer math to shift onto the next note element
        notes = realloc(notes, (*outCount + 1) * sizeof(QuaverNote));
        notes[*outCount] = note;
        (*outCount)++;

        pos = startTime + 10;
    }

    return notes;
}

double TimeToBeat(double timeMs, TimingPoint *timingPoints, size_t tpCount) {
    if (tpCount == 0) {
        return 0.0;
    }

    TimingPoint *timingPoint = &timingPoints[0];
    for (size_t i = 0; i < tpCount; i++) {
        if (i + 1 < tpCount && timeMs >= timingPoints[i + 1].startMs) {
            continue;
        }

        timingPoint = &timingPoints[i];
        break;
    }

    // Math for converting timings to beats
    // Beat = (HitTime(s) - Offset) / 60 * BPM
    double deltaSec = (timeMs - timingPoint->startMs) / 1000.0;
    double deltaBeats = deltaSec * (timingPoint->bpm / 60.0);
    return timingPoint->startBeat + deltaBeats;
}

Measure *BuildMeasures(QuaverNote *notes, size_t noteCount,
                       TimingPoint *timingPoints, size_t tpCount,
                       size_t *outMeasureCount) {
    size_t maxMeasure = 0;

    // find the highest measure to apply
    for (size_t i = 0; i < noteCount; i++) {
        double beat = timeToBeat(notes[i].startMs, timingPoints, tpCount);
        size_t measure = (size_t) floor(beat / 4.0);

        if (measure > maxMeasure) {
            maxMeasure = measure;
        }

        if (notes[i].endMs > 0) {
            double endBeat = TimeToBeat(notes[i].endMs, timingPoints, tpCount);
            size_t endMeasure = (size_t) floor(endBeat / 4.0);

            if (endMeasure > maxMeasure) {
                maxMeasure = endMeasure;
            }
        }
    }

    // fill up all measures with 0s
    Measure *measures = calloc(maxMeasure + 1, sizeof(Measure));
    for (size_t m = 0; m <= maxMeasure; m++) {
        for (int r = 0; r < ROWS_PER_MEASURE; r++) {
            // unsafe but this is not a critical application
            strcpy(measures[m].rows[r], "0000");
        }
    }

    for (size_t i = 0; i < noteCount; i++) {
        double beat = TimeToBeat(notes[i].startMs, timingPoints, tpCount);
        size_t m = (size_t) floor(beat / 4.0);
        double beatInMeasure = beat - (m* 4.0);

        int row = (int) round((beatInMeasure / 4.0) * ROWS_PER_MEASURE);
        // account for array indexing
        if (row >= ROWS_PER_MEASURE) {
            row = ROWS_PER_MEASURE - 1;
        }

        // probably bad to do it this way
        measures[m].rows[row][notes[i].lane] = '1';

        if (notes[i].endMs > 0) {
            double endBeat = TimeToBeat(notes[i].endMs, timingPoints, tpCount);
            size_t endMeasure = (size_t) floor(endMeasure / 4.0);
            double endBeatInMeasure = endBeat - (endMeasure * 4.0);
            
            int endRow = (int) round((endBeatInMeasure / 4.0) * ROWS_PER_MEASURE);
            // account for array indexing
            if (endRow >= ROWS_PER_MEASURE) {
                endRow = ROWS_PER_MEASURE - 1;
            }

            // LNs
            measures[endMeasure].rows[endRow][notes[i].lane] = '3';
        }
    }

    *outMeasureCount = maxMeasure + 1;
    return measures;
}

char *BuildSMNotes(Measure *measures, size_t measureCount) {
    char *out = malloc(2^16); // 65k~ Bytes
    out[0] = '\0';

    // actually build the long string of notes
    for (size_t m = 0; m < measureCount; m++) {
        for (int r = 0; r < ROWS_PER_MEASURE; r++) {
            strcat(out, measures[m].rows[r]);
            strcat(out, "\n");
        }
        strcat(out, m == measureCount - 1 ? ";\n" : ",\n");
    }

    return out;
}