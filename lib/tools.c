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

const char *GetFolderName(const char *path) {
    const char *slash = strrchr(path, '/');
// Solution is fine but something else would be better
#ifdef _WIN32
    const char *backSlash = strrchr(path, '\\');
    if (!slash || (backSlash && backSlash > slash)) {
        slash = backSlash;
    }
#endif
    return slash ? slash + 1 : path;
}

int CopyFile(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return 0;

    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }

    char buffer[18096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        fwrite(buffer, 1, bytes, out);
    }

    fclose(in);
    fclose(out);
    return 1;
}