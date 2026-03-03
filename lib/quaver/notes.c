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
    if (!data) return NULL;

    char *hitObjectsPos = strstr(data, "HitObjects:");
    if (!hitObjectsPos) { free(data); return NULL; }

    char *line = strtok(hitObjectsPos, "\n");

    while (line) {
        if (strstr(line, "- StartTime:")) {
            QuaverNote note;
            note.startMs = atof(strchr(line, ':') + 1);
            note.endMs = -1.0;
            note.lane = -1;

            // Scan forward for Lane and EndTime.
            // .qua field order is StartTime → Lane → EndTime, so we must
            // keep scanning past Lane to pick up EndTime for long notes.
            char *nextLine = strtok(NULL, "\n");
            int hitNextNote = 0;
            while (nextLine) {
                if (strstr(nextLine, "- StartTime:")) {
                    // Hit next note — stop scanning this one, but DON'T
                    // skip this line. Save it for the outer loop.
                    hitNextNote = 1;
                    break;
                }
                if (strstr(nextLine, "Lane:")) {
                    note.lane = atoi(strchr(nextLine, ':') + 1) - 1;
                }
                if (strstr(nextLine, "EndTime:")) {
                    note.endMs = atof(strchr(nextLine, ':') + 1);
                }
                nextLine = strtok(NULL, "\n");
            }

            // Save the note we just finished parsing
            if (note.lane >= 0) {
                notes = realloc(notes, (*outCount + 1) * sizeof(QuaverNote));
                notes[*outCount] = note;
                fprintf(stderr, "[DEBUG] Note %zu: timeMs=%.2f endMs=%.2f lane=%d\n",
                        *outCount, note.startMs, note.endMs, note.lane);
                (*outCount)++;
            }

            // If we stopped because we hit the next note's StartTime,
            // set line to it and re-enter the outer loop without calling
            // strtok (which would skip past it).
            if (hitNextNote) {
                line = nextLine;
                continue;
            }
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
    if (!data) return NULL;

    const char *pos = strstr(data, "TimingPoints:");
    if (!pos) { free(data); return NULL; }

    pos += strlen("TimingPoints:");

    while (1) {
        const char *startMsPtr = strstr(pos, "StartTime:");
        const char *bpmPtr = strstr(pos, "Bpm:");
        if (!startMsPtr || !bpmPtr) break;

        double startMs = atof(startMsPtr + strlen("StartTime:"));
        double bpm = atof(bpmPtr + strlen("Bpm:"));

        points = realloc(points, (*timingPointCount + 1) * sizeof(TimingPoint));
        points[*timingPointCount].startMs = startMs;
        points[*timingPointCount].bpm = bpm;

        if (*timingPointCount == 0) {
            points[*timingPointCount].startBeat = 0.0;
        } else {
            TimingPoint *prev = &points[*timingPointCount - 1];
            double deltaBeats = ((startMs - prev->startMs) / 1000.0) * (prev->bpm / 60.0);
            points[*timingPointCount].startBeat = prev->startBeat + deltaBeats;
        }

        (*timingPointCount)++;
        fprintf(stderr, "[DEBUG] TimingPoint %zu: startMs=%.2f bpm=%.2f startBeat=%.2f\n",
                *timingPointCount - 1, startMs, bpm,
                points[*timingPointCount - 1].startBeat);
        pos = bpmPtr + 4;
    }

    free(data);
    return points;
}

double TimeToBeat(double timeMs, TimingPoint *timingPoints, size_t tpCount) {
    if (tpCount == 0) return 0.0;

    if (timeMs < timingPoints[0].startMs) {
        double deltaMs = timingPoints[0].startMs - timeMs;
        return timingPoints[0].startBeat - (deltaMs / 1000.0) * (timingPoints[0].bpm / 60.0);
    }

    TimingPoint *tp = &timingPoints[0];
    for (size_t i = 1; i < tpCount; i++) {
        if (timingPoints[i].startMs <= timeMs) tp = &timingPoints[i];
        else break;
    }

    return tp->startBeat + ((timeMs - tp->startMs) / 1000.0) * (tp->bpm / 60.0);
}

// Helper: place a note character at a given beat into the measures grid
static void PlaceNote(Measure *measures, size_t measureCount, int minMeasure,
                      double beat, int lane, char noteChar) {
    if (beat < 0.0) beat = 0.0;

    int measureIndex = (int)floor(beat / 4.0) - minMeasure;

    if (measureIndex < 0 || measureIndex >= (int)measureCount) return;

    double beatInMeasure = beat - ((int)floor(beat / 4.0) * 4.0);
    int row = (int)round((beatInMeasure / 4.0) * ROWS_PER_MEASURE);
    if (row < 0) row = 0;

    if (row >= ROWS_PER_MEASURE) {
        measureIndex++;
        row = 0;
        if (measureIndex >= (int)measureCount) return;
    }

    if (lane >= 0 && lane < 4)
        measures[measureIndex].rows[row][lane] = noteChar;
}

Measure *BuildMeasures(QuaverNote *notes, size_t noteCount,
                       TimingPoint *timingPoints, size_t tpCount,
                       size_t *outMeasureCount) {

    if (noteCount == 0) {
        *outMeasureCount = 1;
        Measure *measures = calloc(1, sizeof(Measure));
        for (int r = 0; r < ROWS_PER_MEASURE; r++) strcpy(measures[0].rows[r], "0000");
        return measures;
    }

    // Find beat range, including hold tail end times
    double minBeat = 0.0, maxBeat = 0.0;
    int first = 1;
    for (size_t i = 0; i < noteCount; i++) {
        double beat = TimeToBeat(notes[i].startMs, timingPoints, tpCount);
        if (beat < 0.0) beat = 0.0;
        if (first) { minBeat = maxBeat = beat; first = 0; }
        else { if (beat < minBeat) minBeat = beat; if (beat > maxBeat) maxBeat = beat; }

        // Include hold tail end times in range
        if (notes[i].endMs > 0) {
            double endBeat = TimeToBeat(notes[i].endMs, timingPoints, tpCount);
            if (endBeat < 0.0) endBeat = 0.0;
            if (endBeat > maxBeat) maxBeat = endBeat;
        }

        if (i < 10) fprintf(stderr, "[DEBUG-BUILD] Note %zu: timeMs=%.2f beat=%.4f\n",
                            i, notes[i].startMs, beat);
    }

    fprintf(stderr, "[DEBUG-BUILD] Beat range: %.4f to %.4f\n", minBeat, maxBeat);

    int minMeasure = (int)floor(minBeat / 4.0);
    int maxMeasure = (int)floor(maxBeat / 4.0);
    size_t measureCount = maxMeasure - minMeasure + 1;

    fprintf(stderr, "[DEBUG-BUILD] Measure range: minMeasure=%d maxMeasure=%d\n", minMeasure, maxMeasure);
    fprintf(stderr, "[DEBUG-BUILD] Measure count: %zu\n", measureCount);

    // +1 extra as overflow buffer for boundary notes
    Measure *measures = calloc(measureCount + 1, sizeof(Measure));
    for (size_t m = 0; m <= measureCount; m++)
        for (int r = 0; r < ROWS_PER_MEASURE; r++)
            strcpy(measures[m].rows[r], "0000");

    for (size_t i = 0; i < noteCount; i++) {
        double beat = TimeToBeat(notes[i].startMs, timingPoints, tpCount);

        if (notes[i].endMs > 0) {
            // Long note: place hold head '2' at start, hold tail '3' at end
            PlaceNote(measures, measureCount, minMeasure, beat, notes[i].lane, '2');
            double endBeat = TimeToBeat(notes[i].endMs, timingPoints, tpCount);
            PlaceNote(measures, measureCount, minMeasure, endBeat, notes[i].lane, '3');
        } else {
            // Regular tap: place '1'
            PlaceNote(measures, measureCount, minMeasure, beat, notes[i].lane, '1');
        }
    }

    // No sanitization — hold markers (2/3) must be preserved

    // Trim trailing all-empty measures
    while (measureCount > 1) {
        int isEmpty = 1;
        Measure *last = &measures[measureCount - 1];
        for (int r = 0; r < ROWS_PER_MEASURE && isEmpty; r++)
            for (int c = 0; c < 4 && isEmpty; c++)
                if (last->rows[r][c] != '0') isEmpty = 0;
        if (isEmpty) measureCount--;
        else break;
    }

    *outMeasureCount = measureCount;
    return measures;
}

static int gcd(int a, int b) { return b == 0 ? a : gcd(b, a % b); }

// Round x to nearest divisor of 192 to absorb +/-1 FP drift in row spacing.
static int roundToDivisor(int x) {
    static const int divs[] = {1,2,3,4,6,8,12,16,24,32,48,64,96,192};
    if (x <= 0) return 1;
    int best = divs[0], bestDist = abs(x - divs[0]);
    for (int i = 1; i < 14; i++) {
        int d = abs(x - divs[i]);
        if (d < bestDist) { bestDist = d; best = divs[i]; }
    }
    return best;
}

static int DetectSnapLevel(Measure *measure) {
    int occupied[ROWS_PER_MEASURE], count = 0;

    for (int r = 0; r < ROWS_PER_MEASURE; r++) {
        const char *row = measure->rows[r];
        for (int c = 0; c < 4; c++) {
            // '1' = tap, '2' = hold head, '3' = hold tail — all are beat events
            if (row[c] == '1' || row[c] == '2' || row[c] == '3') {
                occupied[count++] = r;
                break;
            }
        }
    }

    if (count == 0) return 4;

    if (count == 1) {
        int g = occupied[0] > 0 ? occupied[0] : ROWS_PER_MEASURE;
        int snap = ROWS_PER_MEASURE / g;
        if (snap < 4) snap = 4;
        if (snap > 192) snap = 192;
        return snap;
    }

    int g = roundToDivisor(occupied[1] - occupied[0]);
    for (int i = 2; i < count; i++)
        g = gcd(g, roundToDivisor(occupied[i] - occupied[i-1]));

    if (g <= 0) g = 1;
    int snap = ROWS_PER_MEASURE / g;
    if (snap < 4)   snap = 4;
    if (snap > 192) snap = 192;

    fprintf(stderr, "[DEBUG-SNAP] Measure snap detection: %d notes, spacing gcd=%d, snap=%d\n",
            count, g, snap);
    return snap;
}

char *BuildSMNotes(Measure *measures, size_t measureCount) {
    char *out = malloc(1 << 22); // 4 MB
    out[0] = '\0';

    fprintf(stderr, "[DEBUG-OUTPUT] Building notes string with %zu measures\n", measureCount);

    for (size_t m = 0; m < measureCount; m++) {
        int snap = DetectSnapLevel(&measures[m]);
        int step = ROWS_PER_MEASURE / snap;

        fprintf(stderr, "[DEBUG-OUTPUT] Measure %zu snap level: %d (step=%d)\n", m, snap, step);

        // Find the correct loop start offset for FP drift compensation
        int start = 0;
        for (int r = 0; r < ROWS_PER_MEASURE; r++) {
            const char *row = measures[m].rows[r];
            for (int c = 0; c < 4; c++) {
                if (row[c] == '1' || row[c] == '2' || row[c] == '3') {
                    start = r % step;
                    goto found;
                }
            }
        }
        found:
        for (int r = start; r < ROWS_PER_MEASURE; r += step) {
            strcat(out, measures[m].rows[r]);
            strcat(out, "\n");
        }

        strcat(out, m != measureCount - 1 ? ",\n" : ";\n");
    }

    return out;
}