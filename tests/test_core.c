#include "core.h"
#include "processor.h"
#include "dsp.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

int main(void) {
    pl_cpu_info cpu;
    assert(pl_cpu_profile(&cpu) == 0);
    assert(cpu.logical_cpus >= 1);
    assert(cpu.vendor[0] != '\0');

    pl_processor_snapshot snapshot;
    assert(pl_processor_scan(&snapshot) == 0);
    assert(snapshot.architecture[0] != '\0');
    /* AVX usability includes hardware and OS-managed vector-state prerequisites. */
    assert(!snapshot.avx || (snapshot.avx_hw && snapshot.osxsave && snapshot.avx_os_state));
    assert(!snapshot.avx2 || snapshot.avx);
    assert(!snapshot.fma || snapshot.avx);
    assert(!snapshot.avx512f || (snapshot.avx && snapshot.avx512_os_state));
    assert(!snapshot.avx512_os_state || snapshot.avx_os_state);

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
    assert(strstr(details, "x86 SIMD intrinsic") == NULL);
    assert(strstr(details, "CPUID feature discovery") != NULL);
    assert(strstr(details, "cache prefetch hint") != NULL);
    remove(source_path);


    {
        float channel_samples[] = {
            0.5f, -0.25f, -0.5f, 0.25f, 1.0f, -1.0f, 0.0f, 0.0f
        };
        pl_audio_buffer stereo = {channel_samples, 4u, 48000u, 2u};
        pl_audio_stats channel_stats[2] = {{0}};
        assert(pl_audio_measure_channels(&stereo, channel_stats, 2u) == 0);
        assert(channel_stats[0].peak == 1.0 && channel_stats[1].peak == 1.0);
        assert(channel_stats[0].rms > 0.61 && channel_stats[0].rms < 0.62);
        assert(channel_stats[1].rms > 0.53 && channel_stats[1].rms < 0.54);
        assert(channel_stats[0].dc == 0.25 && channel_stats[1].dc == -0.25);
        assert(channel_stats[0].clipped_samples == 1u && channel_stats[1].clipped_samples == 1u);
        assert(pl_audio_measure_channels(&stereo, channel_stats, 1u) != 0);
        assert(pl_audio_measure_channels(&stereo, NULL, 2u) != 0);
        stereo.samples = NULL;
        assert(pl_audio_measure_channels(&stereo, channel_stats, 2u) != 0);
    }

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
    assert(pl_audio_generate_tone(&audio, 48000, 2, NAN, 1.0) != 0);
    assert(pl_audio_generate_tone(&audio, 48000, 2, 440.0, NAN) != 0);
    assert(pl_audio_generate_tone(&audio, 48000, 2, INFINITY, 1.0) != 0);
    assert(pl_audio_generate_tone(&audio, 48000, 2, 440.0, INFINITY) != 0);
    assert(pl_audio_generate_tone(&audio, 48000, 2, 440.0, 0.25) == 0);
    float original_sample = audio.samples[0];
    audio.samples[0] = NAN;
    assert(pl_audio_measure(&audio, &stats) != 0);
    assert(pl_audio_apply_gain(&audio, 0.0) != 0);
    assert(pl_audio_apply_lowpass(&audio, 5000.0) != 0);
    assert(pl_audio_apply_compressor(&audio, -12.0, 4.0) != 0);
    assert(pl_audio_apply_saturation(&audio, 1.5) != 0);
    assert(pl_audio_write_wav("pytyliki_invalid_audio.wav", &audio) != 0);
    audio.samples[0] = original_sample;
    assert(pl_audio_measure(&audio, &stats) == 0);
    pl_audio_free(&audio);

    pl_benchmark invalid_benchmark = b;
    invalid_benchmark.elapsed_ms = NAN;
    pl_analysis empty_analysis = {0};
    assert(pl_write_json("pytyliki_invalid_report.json", &cpu, &invalid_benchmark, &empty_analysis) != 0);
    const char *json_path = "pytyliki_test_report.json";
    assert(pl_write_json(json_path, &cpu, &b, &analysis) == 0);
    FILE *json_file = fopen(json_path, "rb");
    assert(json_file != NULL);
    char json_text[2048] = {0};
    size_t json_size = fread(json_text, 1, sizeof(json_text) - 1u, json_file);
    assert(!ferror(json_file));
    fclose(json_file);
    assert(json_size > 0u);
    assert(strstr(json_text, "\"version\": \"0.7.0\"") != NULL);
    remove(json_path);

    puts("All processor, DSP and core tests passed.");
    return 0;
}
