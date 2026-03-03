#ifndef QUAVER_PARSER_H
#define QUAVER_PARSER_H

typedef struct {
    char *title;
    char *artist;
    char *creator;
    char *audio;
    char *background;
    char *banner;
    char *cdtitle;       // Empty template

    double previewTime;  // Seconds
    double offset;       // Etterna uses -offset
    char *bpms;          // List of bpms in sm format
} QuaverHeader;

// Parses simple key variables into the header struct
QuaverHeader *ParseQuaverHeader(const char *filepath);

// Parses the complex bpm yaml format into a string
char *ParseQuaverBPMS(const char *data);

// Freeing to avoid memory leaks
void FreeQuaverHeader(QuaverHeader *header);

#endif
