#include "errors.h"
#include <string.h>
static const pl_error_entry error_catalog[] = {
    {"PYT-E0107", "error", "active", "Low-pass cutoff out of range",
     "The low-pass cutoff must be greater than the high-pass cutoff and below 0.49 of the WAV sample rate.",
     "The requested cutoff is too high for the selected sample rate, is not above the high-pass cutoff, or the input sample rate was interpreted incorrectly.",
     "Choose a cutoff below 0.49 × sample rate and above the high-pass cutoff. For 44.1 kHz audio, the upper bound is 21,609 Hz.",
     "docs/errors/PYT-E0107.md"},
    {"PYT-E0108", "error", "active", "WAV input cannot be read",
     "The selected file is not a supported PCM WAV stream or its header and payload are inconsistent.",
     "The file may be truncated, use an unsupported encoding, have an invalid channel or sample-rate field, or contain a chunk whose declared size exceeds the file.",
     "Re-export the source as a valid PCM16 WAV, confirm that the file is complete, and inspect it with pytyliki --audio-info FILE.wav before processing.",
     "docs/errors/PYT-E0108.md"},
    {"PYT-E0109", "error", "active", "Audio processing parameter is not finite",
     "A DSP parameter must be a finite numeric value.",
     "The command line may contain NaN, infinity, an overflowed value, or a value parsed from an invalid configuration source.",
     "Supply a finite number in the documented range. Do not use NaN or infinity as a sentinel value.",
     "docs/errors/PYT-E0109.md"}
};
size_t pl_error_count(void) {
    return sizeof(error_catalog) / sizeof(error_catalog[0]);
}
const pl_error_entry *pl_error_at(size_t index) {
    return index < pl_error_count() ? &error_catalog[index] : NULL;
}
const pl_error_entry *pl_error_find(const char *code) {
    size_t index;
    if (code == NULL || code[0] == '\0') return NULL;
    for (index = 0u; index < pl_error_count(); ++index)
        if (strcmp(error_catalog[index].code, code) == 0) return &error_catalog[index];
    return NULL;
}
void pl_error_print(FILE *stream, const pl_error_entry *entry) {
    if (stream == NULL || entry == NULL) return;
    fprintf(stream, "%s  %s  [%s]\n", entry->code, entry->title, entry->status);
    fprintf(stream, "Severity: %s\n\n", entry->severity);
    fprintf(stream, "What happened\n  %s\n\n", entry->summary);
    fprintf(stream, "Typical causes\n  %s\n\n", entry->causes);
    fprintf(stream, "How to fix it\n  %s\n\n", entry->remedy);
    fprintf(stream, "Documentation: %s\n", entry->documentation);
}
