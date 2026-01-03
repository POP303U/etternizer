#include "header.h"
#include "../file.h"
#include "../tools.h"

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
        header->preview_time = atof(preview) / 1000.0;
        free(preview);
    }

    // TimingPoints
    header->offset = -ExtractDouble(data, "StartTime");
    header->bpm    =  ExtractDouble(data, "Bpm");

    free(data);
    return header;
}

void FreeQuaverHeader(QuaverHeader *header) {
    if (!header) return;
    free(header->title);
    free(header->artist);
    free(header->creator);
    free(header->audio);
    free(header->background);
    free(header->banner);
    free(header);
}
