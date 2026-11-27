#include "dsp.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void write_u16(FILE *file, uint16_t value) {
    fputc((int)(value & 0xffu), file);
    fputc((int)(value >> 8), file);
}

static void write_u32(FILE *file, uint32_t value) {
    write_u16(file, (uint16_t)(value & 0xffffu));
    write_u16(file, (uint16_t)(value >> 16));
}

static FILE *open_fixture(const char *path) {
#if defined(_MSC_VER)
    FILE *file = NULL;
    if (fopen_s(&file, path, "wb") != 0) return NULL;
    return file;
#else
    return fopen(path, "wb");
#endif
}

static int write_fixture(const char *path, uint32_t riff_size,
                         uint32_t data_size, size_t actual_data_bytes) {
    FILE *file = open_fixture(path);
    if (file == NULL) return -1;
    fwrite("RIFF", 1u, 4u, file);
    write_u32(file, riff_size);
    fwrite("WAVEfmt ", 1u, 8u, file);
    write_u32(file, 16u);
    write_u16(file, 1u);
    write_u16(file, 1u);
    write_u32(file, 48000u);
    write_u32(file, 96000u);
    write_u16(file, 2u);
    write_u16(file, 16u);
    fwrite("data", 1u, 4u, file);
    write_u32(file, data_size);
    for (size_t i = 0u; i < actual_data_bytes; ++i) fputc(0, file);
    if (actual_data_bytes == data_size && (data_size & 1u)) fputc(0, file);
    if (fclose(file) != 0) return -1;
    return 0;
}

int main(void) {
    const char *path = "pytyliki_wav_boundary_fixture.wav";
    pl_audio_buffer audio = {0};

    assert(write_fixture(path, 38u, 2u, 2u) == 0);
    assert(pl_audio_load_wav(path, &audio) == 0);
    assert(audio.frames == 1u && audio.channels == 1u && audio.sample_rate == 48000u);
    pl_audio_free(&audio);

    /* The file has bytes after the declared RIFF end; they are not part of WAVE. */
    assert(write_fixture(path, 36u, 2u, 2u) == 0);
    assert(pl_audio_load_wav(path, &audio) != 0);
    pl_audio_free(&audio);

    /* A chunk may not claim more bytes than remain in its RIFF container. */
    assert(write_fixture(path, 38u, 4u, 4u) == 0);
    assert(pl_audio_load_wav(path, &audio) != 0);
    pl_audio_free(&audio);

    /* The RIFF extent itself must be present in the physical file. */
    assert(write_fixture(path, 38u, 2u, 1u) == 0);
    assert(pl_audio_load_wav(path, &audio) != 0);
    pl_audio_free(&audio);

    /* Deterministic malformed-input fuzz corpus for the production WAV loader. */
    uint32_t state = UINT32_C(0x5AFE2026);
    unsigned char bytes[256];
    for (size_t round = 0u; round < 256u; ++round) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        size_t length = (size_t)(state % (uint32_t)sizeof(bytes));
        for (size_t i = 0u; i < length; ++i) {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            bytes[i] = (unsigned char)(state & 0xffu);
        }
        /* Bias part of the corpus toward RIFF-shaped but inconsistent headers. */
        if (round % 3u == 0u && length >= 12u) {
            bytes[0] = 'R'; bytes[1] = 'I'; bytes[2] = 'F'; bytes[3] = 'F';
            bytes[8] = 'W'; bytes[9] = 'A'; bytes[10] = 'V'; bytes[11] = 'E';
        }
        FILE *fuzz_file = open_fixture(path);
        assert(fuzz_file != NULL);
        assert(fwrite(bytes, 1u, length, fuzz_file) == length);
        assert(fclose(fuzz_file) == 0);
        pl_audio_buffer fuzzed = {0};
        (void)pl_audio_load_wav(path, &fuzzed);
        pl_audio_free(&fuzzed);
    }

    remove(path);
    puts("WAV RIFF boundary and deterministic fuzz-smoke tests passed.");
    return 0;
}
