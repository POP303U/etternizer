#ifndef QUAVER_HEADER_H
#define QUAVER_HEADER_H

typedef struct {
    char *title;
    char *artist;
    char *creator;
    char *audio;
    char *background;
    char *banner;

    double preview_time; // seconds
    double offset;       // etterna uses -offset
    double bpm;
} QuaverHeader;

QuaverHeader *ParseQuaverHeader(const char *file);
void FreeQuaverHeader(QuaverHeader *h);

#endif
