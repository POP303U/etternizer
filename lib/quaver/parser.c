#include "parser.h"
#include "../file.h"
#include "../tools.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

QuaverHeader *ParseQuaverHeader(const char *file) {
    char *data = LoadFile(file);
    if (!data) {
        return NULL;
    }

    QuaverHeader *header = calloc(1, sizeof(QuaverHeader));

    // Metadata
    header->title      = ExtractValue(data, "Title");
    header->artist     = ExtractValue(data, "Artist");
    header->creator    = ExtractValue(data, "Creator");
    header->audio      = ExtractValue(data, "AudioFile");
    header->background = ExtractValue(data, "BackgroundFile");
    header->banner     = ExtractValue(data, "BannerFile");

    char *preview = ExtractValue(data, "SongPreviewTime");
    if (preview) {
        header->previewTime = atof(preview) / 1000.0;
        free(preview);
    }

    // TimingPoints
    header->offset = (-ExtractDouble(data, "StartTime") / 1000.0);
    header->bpms   = ParseQuaverBPMS(data);

    free(data);
    return header;
}

char *ParseQuaverBPMS(const char *data) {
    if (!data) {
        return NULL;
    }

    char *pos = strstr(data, "TimingPoints:");

    if (!pos) {
        return strdup("0=0");
    }

    pos += strlen("TimingPoints:");

    double baseTime = -1.0;

    char *bpms = malloc(2048);
    bpms[0] = '\0';

    while (1) {
        // temporary vars for counting string length
        char *_startTime = strstr(pos, "StartTime:");
        char *_bpm = strstr(pos, "Bpm:");

        if (!_startTime || !_bpm) {
            break;
        }

        // starttime in ms
        double startTime = atof(_startTime + strlen("Starttime:")) / 1000.0;
        double bpm = atof(_bpm + strlen("Bpm:"));

        // boundschecking
        if (baseTime < 0.0) {
            baseTime = startTime;
        }

        // stepmania does timing in a negative offset way
        double smTime = startTime - baseTime;

        char buffer[64];

        // dont add another comma if the current bpm doesn't exist
        snprintf(buffer, sizeof(buffer), "%s%.3f=%.3f", bpms[0] ? "," : "", smTime, bpm);

        strcat(bpms, buffer);

        pos = _bpm + strlen("Bpm:");

        if (bpms[0] == '\0') {
            strcpy(bpms, "0=0");
        }
    }

    return bpms;
}

void FreeQuaverHeader(QuaverHeader *header) {
    if (!header) return;
    free(header->title);
    free(header->artist);
    free(header->creator);
    free(header->audio);
    free(header->background);
    free(header->banner);
    free(header->bpms);
    free(header);
}
