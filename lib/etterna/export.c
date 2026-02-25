#include "header.h"
#include "export.h"
#include "../tools.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Using preprocessor is not the most pretty way to do this
#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif

int ExportSMChart(
    const char *quaverDir,
    const QuaverHeader *header,
    const char *smHeader,
    const char *smNotes
) {
    if (!quaverDir || !header || !smHeader || !smNotes) {
        return 0;
    }

    const char *folderName = GetFolderName(quaverDir);

    char exportPath[512];
    snprintf(exportPath, sizeof(exportPath), "exports/%s", folderName);

    /* Create exports/ */
    MKDIR("exports");

    /* Create exports/<song> */
    if (MKDIR(exportPath) != 0) {
        if (errno != EEXIST) {
            perror("mkdir");
            return 0;
        }
    }
    /* ---------- Write chart.sm ---------- */

    char smFile[512];
    snprintf(smFile, sizeof(smFile), "%s/chart.sm", exportPath);

    FILE *sm = fopen(smFile, "w");
    if (!sm) {
        perror("chart.sm");
        return 0;
    }

    fprintf(sm, "%s\n%s\n", smHeader, smNotes);
    fclose(sm);

/* Copy background */
if (header->background) {
    char srcBg[PATH_MAX];
    char dstBg[PATH_MAX];

    snprintf(srcBg, sizeof(srcBg), "%s/%s", quaverDir, header->background);
    snprintf(dstBg, sizeof(dstBg), "%s/BG.png", exportPath);

    if (!CopyFile(srcBg, dstBg)) {
        fprintf(stderr, "Warning: BG copy failed (%s)\n", srcBg);
    }
}

/* Copy banner */
if (header->banner) {
    char srcBn[PATH_MAX];
    char dstBn[PATH_MAX];

    snprintf(srcBn, sizeof(srcBn), "%s/%s", quaverDir, header->banner);
    snprintf(dstBn, sizeof(dstBn), "%s/BN.png", exportPath);

    if (!CopyFile(srcBn, dstBn)) {
        fprintf(stderr, "Warning: BN copy failed (%s)\n", srcBn);
    }
}

/* Copy audio */
if (header->audio) {
    char srcAudio[PATH_MAX];
    char dstAudio[PATH_MAX];

    snprintf(srcAudio, sizeof(srcAudio), "%s/%s", quaverDir, header->audio);
    snprintf(dstAudio, sizeof(dstAudio), "%s/%s", exportPath, header->audio);

    if (!CopyFile(srcAudio, dstAudio)) {
        fprintf(stderr, "Warning: audio copy failed (%s)\n", srcAudio);
    }
}
    // Multiple exports might a thing for later
    return 1;
}