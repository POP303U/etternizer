#include "header.h"
#include <stdio.h>
#include <stdlib.h>

char *BuildSMHeader(const QuaverHeader *quaverHeader) {
    char *header = malloc(2048);

    snprintf(header, 2048,
             "#TITLE:%s;\n"
             "#SUBTITLE:;\n"
             "#ARTIST:%s;\n"
             "#CREDIT:%s;\n"
             "#MUSIC:%s;\n"
             "#SELECTABLE:YES;\n"
             "#BACKGROUND:%s;\n"
             "#BANNER:%s;\n"
             "#CDTITLE:cdtitle.png;\n"
             "#OFFSET:%.7f;\n"
             "#SAMPLESTART:%.3f;\n"
             "#SAMPLELENGTH:23.640;\n"
             "#BPMS:%s\n"
             "#STOPS:;\n"
             "//---------------dance-single - ----------------\n"
             "#NOTES:\n"
             "    dance-single:\n"
             "    : \n"
             "    Challenge: \n"
             "    25: \n"
             "    0.000,0.000,0.000,0.000,0.000:\n",
             quaverHeader->title ? quaverHeader->title : "",
             quaverHeader->artist ? quaverHeader->artist : "",
             quaverHeader->creator ? quaverHeader->creator : "",
             quaverHeader->audio ? quaverHeader->audio : "",
             quaverHeader->background ? quaverHeader->background : "",
             quaverHeader->banner ? quaverHeader->banner : "",
             quaverHeader->offset ? quaverHeader->offset : 0,
             quaverHeader->previewTime ? quaverHeader->previewTime : 5,
             quaverHeader->bpms ? quaverHeader->bpms : "");

    return header;
}
