#include "core.h"
#include "processor.h"
#include "dsp.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    pl_cpu_info cpu;
    assert(pl_cpu_profile(&cpu) == 0);
    assert(cpu.logical_cpus >= 1);
    assert(cpu.vendor[0] != '\0');

    pl_processor_snapshot snapshot;
    assert(pl_processor_scan(&snapshot) == 0);
    assert(snapshot.architecture[0] != '\0');

    pl_benchmark b;
    assert(pl_run_benchmark(10000, &b) == 0);
    assert(b.iterations == 10000);
    assert(b.elapsed_ms >= 0);
    assert(pl_run_benchmark(0, &b) != 0);
    assert(pl_run_benchmark(UINT64_C(500000001), &b) != 0);

    pl_suite_result suite;
    assert(pl_processor_benchmark_suite(50000, 1, 2, &suite) == 0);
    assert(suite.integrity_ok);
    assert(suite.checksum != 0);
    assert(pl_processor_benchmark_suite(50000, 0, 2, &suite) != 0);

    const char *source_path = "pytyliki_test_input.c";
    FILE *source = fopen(source_path, "wb");
    assert(source);
    fputs("void kernel(void){\n __cpuid(regs, 1);\n __m256 v;\n _mm_prefetch(ptr, 0);\n atomic_thread_fence(memory_order_relaxed);\n}\n", source);
    fclose(source);

    pl_analysis analysis = {0};
    char details[1024];
    assert(pl_analyze_file(source_path, &analysis, details, sizeof(details)) == 0);
    assert(analysis.files_scanned == 1);
    assert(analysis.findings == 5);
    remove(source_path);

    pl_audio_buffer audio = {0}, loaded = {0};
    assert(pl_audio_generate_tone(&audio, 48000, 2, 440.0, 0.25) == 0);
    assert(audio.frames == 12000 && audio.channels == 2);

    pl_audio_stats stats;
    assert(pl_audio_measure(&audio, &stats) == 0);
    assert(stats.peak > 0.65 && stats.peak < 0.71);
    assert(pl_audio_apply_lowpass(&audio, 5000.0) == 0);
    assert(pl_audio_apply_compressor(&audio, -12.0, 4.0) == 0);
    assert(pl_audio_apply_saturation(&audio, 1.5) == 0);
    assert(pl_audio_apply_gain(&audio, -1.0) == 0);
    assert(pl_audio_measure(&audio, &stats) == 0);
    assert(stats.rms > 0.0);

    const char *wav_path = "pytyliki_test_audio.wav";
    assert(pl_audio_write_wav(wav_path, &audio) == 0);
    assert(pl_audio_load_wav(wav_path, &loaded) == 0);
    assert(loaded.frames == audio.frames);
    assert(loaded.channels == 2);
    assert(loaded.sample_rate == 48000);

    remove(wav_path);
    pl_audio_free(&loaded);
    pl_audio_free(&audio);
    assert(pl_audio_generate_tone(&audio, 48000, 2, 24000.0, 1.0) != 0);

    puts("All processor, processor-DSP and core tests passed.");
    return 0;
}
