#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tools.h"

double ExtractDouble(const char *source, const char *key) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "%s:", key);

    char *pos = strstr(source, pattern);
    if (!pos) return 0.0;

    pos += strlen(pattern);
    while (*pos == ' ') pos++;

    return atof(pos);
}

char *ExtractValue(const char *source, const char *key) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "%s:", key);

    char *pos = strstr(source, pattern);
    if (!pos) return NULL;

    pos += strlen(pattern);
    while (*pos == ' ') pos++;

    char *end = strchr(pos, '\n');
    if (!end) return NULL;

    size_t len = end - pos;
    char *out = malloc(len + 1);
    strncpy(out, pos, len);
    out[len] = 0;

    return out;
}
