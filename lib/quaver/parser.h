#ifndef QUAVER_PARSER_H
#define QUAVER_PARSER_H

typedef struct {
    char *title;
    char *artist;
    char *creator;
    char *audio;
    char *background;
    char *banner;

    double preview_time; // seconds
    double offset;       // etterna uses -offset
    char *bpms;          // list of bpms in sm format
} QuaverHeader;

// Parses simple key variables into the header struct
QuaverHeader *ParseQuaverHeader(const char *file);

// Parses the complex bpm yaml format into a string
char *ParseQuaverBPMS(const char *data);

// Freeing to avoid memory leaks
void FreeQuaverHeader(QuaverHeader *header);

#endif
