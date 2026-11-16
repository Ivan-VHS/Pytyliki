#define _POSIX_C_SOURCE 200809L
#include "core.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(_WIN32)
#include <windows.h>
#include <intrin.h>
#else
#include <unistd.h>
#if defined(__x86_64__) || defined(__i386__)
#include <cpuid.h>
#endif
#endif

static void copy_text(char *dst, size_t cap, const char *src) {
    if (cap == 0) return;
    snprintf(dst, cap, "%s", src ? src : "unknown");
}
static void trim_spaces(char *text) {
    size_t n = strlen(text);
    while (n && text[n - 1] == ' ') text[--n] = '\0';
    size_t start = 0;
    while (text[start] == ' ') ++start;
    if (start) memmove(text, text + start, strlen(text + start) + 1u);
}
int pl_cpu_profile(pl_cpu_info *out) {
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
#if defined(_WIN32)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    out->logical_cpus = si.dwNumberOfProcessors ? si.dwNumberOfProcessors : 1u;
#elif defined(_SC_NPROCESSORS_ONLN)
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    out->logical_cpus = n > 0 ? (unsigned)n : 1u;
#else
    out->logical_cpus = 1u;
#endif
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    int r[4] = {0,0,0,0};
    __cpuid(r, 0);
    int max_leaf = r[0];
    char vendor[13];
    memcpy(vendor, &r[1], 4); memcpy(vendor + 4, &r[3], 4); memcpy(vendor + 8, &r[2], 4); vendor[12] = '\0';
    copy_text(out->vendor, sizeof(out->vendor), vendor);
    if (max_leaf >= 1) {
        __cpuidex(r, 1, 0);
        unsigned base_family = ((unsigned)r[0] >> 8) & 15u;
        unsigned base_model = ((unsigned)r[0] >> 4) & 15u;
        unsigned ext_family = ((unsigned)r[0] >> 20) & 255u;
        unsigned ext_model = ((unsigned)r[0] >> 16) & 15u;
        out->family = base_family == 15u ? base_family + ext_family : base_family;
        out->model_id = (base_family == 6u || base_family == 15u) ? base_model + (ext_model << 4) : base_model;
    }
    __cpuid(r, (int)0x80000000u);
    if ((unsigned)r[0] >= 0x80000004u) {
        char brand[49];
        for (int leaf = 0; leaf < 3; ++leaf) {
            __cpuid(r, (int)(0x80000002u + (unsigned)leaf));
            memcpy(brand + leaf * 16, r, 16);
        }
        brand[48] = '\0';
        trim_spaces(brand);
        copy_text(out->model, sizeof(out->model), brand);
    }
#elif (defined(__x86_64__) || defined(__i386__)) && defined(__GNUC__)
    unsigned a = 0, b = 0, c = 0, d = 0;
    if (__get_cpuid(0, &a, &b, &c, &d)) {
        char vendor[13];
        memcpy(vendor, &b, 4); memcpy(vendor + 4, &d, 4); memcpy(vendor + 8, &c, 4); vendor[12] = '\0';
        copy_text(out->vendor, sizeof(out->vendor), vendor);
    }
    if (__get_cpuid(1, &a, &b, &c, &d)) {
        unsigned base_family = (a >> 8) & 15u;
        unsigned base_model = (a >> 4) & 15u;
        unsigned ext_family = (a >> 20) & 255u;
        unsigned ext_model = (a >> 16) & 15u;
        out->family = base_family == 15u ? base_family + ext_family : base_family;
        out->model_id = (base_family == 6u || base_family == 15u) ? base_model + (ext_model << 4) : base_model;
    }
    if (__get_cpuid_max(0x80000000u, NULL) >= 0x80000004u) {
        char brand[49];
        for (unsigned leaf = 0; leaf < 3; ++leaf) {
            if (!__get_cpuid(0x80000002u + leaf, &a, &b, &c, &d)) break;
            memcpy(brand + leaf * 16u, &a, 4);
            memcpy(brand + leaf * 16u + 4u, &b, 4);
            memcpy(brand + leaf * 16u + 8u, &c, 4);
            memcpy(brand + leaf * 16u + 12u, &d, 4);
        }
        brand[48] = '\0';
        trim_spaces(brand);
        copy_text(out->model, sizeof(out->model), brand);
    }
#else
    copy_text(out->vendor, sizeof(out->vendor), "not-available");
#endif
    if (!out->vendor[0]) copy_text(out->vendor, sizeof(out->vendor), "not-available");
    if (!out->model[0]) copy_text(out->model, sizeof(out->model), "CPU brand string unavailable on this build");
    return 0;
}
static double now_ms(void) {
#if defined(_WIN32)
    LARGE_INTEGER frequency, counter;
    if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&counter)) return 0.0;
    return 1000.0 * (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) return (double)clock() / ((double)CLOCKS_PER_SEC / 1000.0);
    return 1000.0 * (double)t.tv_sec + (double)t.tv_nsec / 1000000.0;
#endif
}
int pl_run_benchmark(uint64_t iterations, pl_benchmark *out) {
    if (!out || iterations == 0 || iterations > UINT64_C(500000000)) return -1;
    volatile uint64_t x = UINT64_C(0x9e3779b97f4a7c15);
    double start = now_ms();
    for (uint64_t i = 0; i < iterations; ++i) {
        x ^= x >> 12; x ^= x << 25; x ^= x >> 27; x *= UINT64_C(2685821657736338717);
    }
    double elapsed = now_ms() - start;
    if (x == 0) fputc(' ', stdout);
    out->iterations = iterations; out->elapsed_ms = elapsed;
    out->score = elapsed > 0.0 ? (double)iterations / elapsed : 0.0;
    return 0;
}
static int contains(const char *s, const char *needle) { return strstr(s, needle) != NULL; }
int pl_analyze_file(const char *path, pl_analysis *totals, char *details, size_t cap) {
    static const struct { const char *token; const char *label; } checks[] = {
        {"__cpuid", "CPUID feature discovery; validate leaf bounds and vendor semantics"},
        {"__rdtsc", "timestamp-counter use; validate serialization and invariant-TSC assumptions"},
        {"_mm_", "x86 SIMD intrinsic; verify ISA dispatch and OS vector-state support"},
        {"_mm_prefetch", "cache prefetch hint; benchmark locality and avoid assuming guaranteed benefit"},
        {"__m256", "wide-vector type; verify alignment, feature gating and fallback path"},
        {"__m512", "512-bit vector type; verify hardware and operating-system state support"},
        {"__builtin_prefetch", "compiler prefetch hint; measure workload-specific effect"},
        {"memory_order_relaxed", "relaxed atomic ordering; review synchronization correctness"},
        {"atomic_thread_fence", "memory fence; verify ordering requirements and cost"},
        {"volatile", "volatile access; distinguish MMIO semantics from thread synchronization"},
        {"__asm", "inline assembly; verify ABI, clobbers and target ISA constraints"},
        {"asm volatile", "inline assembly; verify ABI, clobbers and target ISA constraints"},
        {"_mm_stream", "non-temporal store; verify alignment, fences and write-combining assumptions"},
        {"alignas", "explicit data alignment; validate ABI and allocation alignment"}
    };
    if (!path || !totals || !details || cap == 0) return -1;
    FILE *f = fopen(path, "rb");
    if (!f) { totals->read_errors++; return -1; }
    char line[2048]; unsigned line_no = 0; details[0] = '\0'; totals->files_scanned++;
    while (fgets(line, sizeof(line), f)) {
        line_no++;
        for (size_t i = 0; i < sizeof(checks)/sizeof(checks[0]); ++i) {
            if (strcmp(checks[i].token, "_mm_") == 0 &&
                (contains(line, "_mm_prefetch") || contains(line, "_mm_stream"))) continue;
            if (strcmp(checks[i].token, "__asm") == 0 && contains(line, "asm volatile")) continue;
            if (contains(line, checks[i].token)) {
                char item[384];
                snprintf(item, sizeof(item), "line %u: %s\n", line_no, checks[i].label);
                size_t used = strlen(details), need = strlen(item);
                if (used + need + 1u < cap) memcpy(details + used, item, need + 1u);
                totals->findings++;
            }
        }
    }
    if (ferror(f)) { fclose(f); totals->read_errors++; return -1; }
    fclose(f);
    return 0;
}
static void json_string(FILE *f, const char *s) {
    fputc('"', f);
    for (; *s; ++s) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') { fputc('\\', f); fputc(c, f); }
        else if (c < 32) fprintf(f, "\\u%04x", c);
        else fputc(c, f);
    }
    fputc('"', f);
}
int pl_write_json(const char *path, const pl_cpu_info *cpu, const pl_benchmark *bench, const pl_analysis *analysis) {
    if (!path || path[0] == '\0' || !cpu || !bench || !analysis ||
        memchr(cpu->vendor, '\0', sizeof(cpu->vendor)) == NULL ||
        memchr(cpu->model, '\0', sizeof(cpu->model)) == NULL ||
        !isfinite(bench->elapsed_ms) || bench->elapsed_ms < 0.0 ||
        !isfinite(bench->score) || bench->score < 0.0) return -1;
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    fputs("{\n  \"version\": \"0.7.0\",\n  \"cpu\": {\"vendor\":", f);
    json_string(f, cpu->vendor); fputs(",\"model\":", f); json_string(f, cpu->model);
    fprintf(f, ",\"logical_cpus\":%u,\"family\":%u,\"model_id\":%u},\n", cpu->logical_cpus, cpu->family, cpu->model_id);
    fprintf(f, "  \"benchmark\": {\"iterations\":%llu,\"elapsed_ms\":%.3f,\"iterations_per_ms\":%.3f},\n",
            (unsigned long long)bench->iterations, bench->elapsed_ms, bench->score);
    fprintf(f, "  \"analysis\": {\"files_scanned\":%u,\"findings\":%u,\"read_errors\":%u}\n}\n",
            analysis->files_scanned, analysis->findings, analysis->read_errors);
    int bad = ferror(f);
    if (fclose(f) != 0) bad = 1;
    return bad ? -1 : 0;
}
