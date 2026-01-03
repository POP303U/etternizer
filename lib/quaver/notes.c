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