#include "parser.h"
#include "../file.h"
#include "../tools.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

QuaverHeader *ParseQuaverHeader(const char *filepath) {
    char *data = LoadFile(filepath);
    if (!data) {
        return NULL;
    }

    QuaverHeader *header = calloc(1, sizeof(QuaverHeader));

    // Metadata
    header->title      = ExtractValue(data, "Title");
    header->artist     = ExtractValue(data, "Artist");
    header->creator    = ExtractValue(data, "Creator");

    // Strip surrounding single quotes from creator (Quaver stores empty as '')
    if (header->creator) {
        size_t len = strlen(header->creator);
        if (len >= 2 && header->creator[0] == '\'' && header->creator[len - 1] == '\'') {
            memmove(header->creator, header->creator + 1, len - 2);
            header->creator[len - 2] = '\0';
        }
    }
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
        return strdup("0.000=0.000");
    }

    pos += strlen("TimingPoints:");

    double lastMs = 0.0;
    double lastBpm = 0.0;
    double currentBeat = 0.0;
    int first = 1;

    char *bpms = malloc(4096);
    bpms[0] = '\0';

    while (1) {
        char *startPtr = strstr(pos, "StartTime:");
        char *bpmPtr   = strstr(pos, "Bpm:");

        if (!startPtr || !bpmPtr) {
            break;
        }

        double startMs = atof(startPtr + strlen("StartTime:"));
        double bpm     = atof(bpmPtr + strlen("Bpm:"));

        if (!first) {
            double deltaMs = startMs - lastMs;
            currentBeat += (deltaMs / 1000.0) * (lastBpm / 60.0);
        } else {
            currentBeat = 0.0;
            first = 0;
        }

        char buffer[128];
        long long ibeat = (long long)currentBeat;
        long long ibpm  = (long long)bpm;
        if ((double)ibeat == currentBeat && (double)ibpm == bpm) {
            snprintf(buffer, sizeof(buffer), "%s%lld=%lld",
                     bpms[0] ? "," : "", ibeat, ibpm);
        } else if ((double)ibeat == currentBeat) {
            snprintf(buffer, sizeof(buffer), "%s%lld=%.3f",
                     bpms[0] ? "," : "", ibeat, bpm);
        } else {
            snprintf(buffer, sizeof(buffer), "%s%.3f=%.3f",
                     bpms[0] ? "," : "", currentBeat, bpm);
        }

        strcat(bpms, buffer);

        lastMs = startMs;
        lastBpm = bpm;
        pos = bpmPtr + strlen("Bpm:");
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