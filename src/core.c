#define _POSIX_C_SOURCE 200809L
#include "core.h"
#include <stdio.h>
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
    if (cap != 0) (void)snprintf(dst, cap, "%s", src != NULL ? src : "unknown");
}

static void set_family_model(pl_cpu_info *out, unsigned eax) {
    unsigned base_family = (eax >> 8) & 0x0fu;
    unsigned base_model = (eax >> 4) & 0x0fu;
    unsigned extended_family = (eax >> 20) & 0xffu;
    unsigned extended_model = (eax >> 16) & 0x0fu;

    out->family = base_family;
    out->model_id = base_model;
    if (base_family == 0x0fu) out->family += extended_family;
    if (base_family == 0x06u || base_family == 0x0fu)
        out->model_id += extended_model << 4;
}

static void detect_brand_string(char *dst, size_t cap) {
    char brand[49];
    unsigned i;

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    int regs[4];
    __cpuid(regs, (int)0x80000000u);
    if ((unsigned)regs[0] < 0x80000004u) {
        copy_text(dst, cap, "CPU model string unavailable on this processor");
        return;
    }
    for (i = 0; i < 3u; ++i) {
        __cpuid(regs, (int)(0x80000002u + i));
        memcpy(brand + i * 16u, &regs[0], 4);
        memcpy(brand + i * 16u + 4u, &regs[1], 4);
        memcpy(brand + i * 16u + 8u, &regs[2], 4);
        memcpy(brand + i * 16u + 12u, &regs[3], 4);
    }
#elif (defined(__x86_64__) || defined(__i386__)) && defined(__GNUC__)
    unsigned a, b, c, d;
    if (__get_cpuid_max(0x80000000u, NULL) < 0x80000004u) {
        copy_text(dst, cap, "CPU model string unavailable on this processor");
        return;
    }
    for (i = 0; i < 3u; ++i) {
        if (!__get_cpuid(0x80000002u + i, &a, &b, &c, &d)) {
            copy_text(dst, cap, "CPU model string unavailable on this build");
            return;
        }
        memcpy(brand + i * 16u, &a, 4);
        memcpy(brand + i * 16u + 4u, &b, 4);
        memcpy(brand + i * 16u + 8u, &c, 4);
        memcpy(brand + i * 16u + 12u, &d, 4);
    }
#else
    (void)i;
    copy_text(dst, cap, "CPU model string unavailable on this build");
    return;
#endif

    brand[48] = '\0';
    {
        size_t start = 0;
        size_t end = strlen(brand);
        while (start < end && brand[start] == ' ') ++start;
        while (end > start && brand[end - 1u] == ' ') --end;
        brand[end] = '\0';
        copy_text(dst, cap, brand + start);
    }
}

int pl_cpu_profile(pl_cpu_info *out) {
    if (out == NULL) return -1;
    memset(out, 0, sizeof(*out));

#if defined(_WIN32)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        out->logical_cpus = si.dwNumberOfProcessors != 0u ? si.dwNumberOfProcessors : 1u;
    }
#elif defined(_SC_NPROCESSORS_ONLN)
    {
        long n = sysconf(_SC_NPROCESSORS_ONLN);
        out->logical_cpus = n > 0 ? (unsigned)n : 1u;
    }
#else
    out->logical_cpus = 1u;
#endif

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    {
        int r[4];
        __cpuid(r, 0);
        {
            char vendor[13];
            memcpy(vendor, &r[1], 4);
            memcpy(vendor + 4, &r[3], 4);
            memcpy(vendor + 8, &r[2], 4);
            vendor[12] = '\0';
            copy_text(out->vendor, sizeof(out->vendor), vendor);
        }
        __cpuid(r, 1);
        set_family_model(out, (unsigned)r[0]);
    }
#elif (defined(__x86_64__) || defined(__i386__)) && defined(__GNUC__)
    {
        unsigned a, b, c, d;
        if (__get_cpuid(0, &a, &b, &c, &d)) {
            char vendor[13];
            memcpy(vendor, &b, 4);
            memcpy(vendor + 4, &d, 4);
            memcpy(vendor + 8, &c, 4);
            vendor[12] = '\0';
            copy_text(out->vendor, sizeof(out->vendor), vendor);
        }
        if (__get_cpuid(1, &a, &b, &c, &d)) set_family_model(out, a);
    }
#else
    copy_text(out->vendor, sizeof(out->vendor), "not-available");
#endif

    if (out->vendor[0] == '\0')
        copy_text(out->vendor, sizeof(out->vendor), "not-available");
    detect_brand_string(out->model, sizeof(out->model));
    return 0;
}

static double now_ms(void) {
#if defined(_WIN32)
    LARGE_INTEGER frequency, counter;
    if (!QueryPerformanceFrequency(&frequency) || frequency.QuadPart == 0 ||
        !QueryPerformanceCounter(&counter)) return (double)clock() * 1000.0 / CLOCKS_PER_SEC;
    return 1000.0 * (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0)
        return (double)clock() * 1000.0 / CLOCKS_PER_SEC;
    return 1000.0 * (double)t.tv_sec + (double)t.tv_nsec / 1000000.0;
#endif
}

int pl_run_benchmark(uint64_t iterations, pl_benchmark *out) {
    volatile uint64_t x = UINT64_C(0x9e3779b97f4a7c15);
    double start, elapsed;
    uint64_t i;

    if (out == NULL || iterations == 0 ||
        iterations > UINT64_C(500000000)) return -1;

    start = now_ms();
    for (i = 0; i < iterations; ++i) {
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        x *= UINT64_C(2685821657736338717);
    }
    elapsed = now_ms() - start;
    if (x == 0) fputc(' ', stdout);
    out->iterations = iterations;
    out->elapsed_ms = elapsed;
    out->score = elapsed > 0.0 ? (double)iterations / elapsed : 0.0;
    return 0;
}

static int contains(const char *s, const char *needle) {
    return strstr(s, needle) != NULL;
}

int pl_analyze_file(const char *path, pl_analysis *totals,
                    char *details, size_t cap) {
    FILE *f;
    char line[2048];
    unsigned line_no = 0;

    if (path == NULL || totals == NULL || details == NULL || cap == 0) return -1;
    f = fopen(path, "rb");
    if (f == NULL) {
        totals->read_errors++;
        return -1;
    }
    details[0] = '\0';
    totals->files_scanned++;

    while (fgets(line, sizeof(line), f) != NULL) {
        const char *finding = NULL;
        char item[256];
        size_t used, need;
        ++line_no;

        if (contains(line, "system(") || contains(line, "popen("))
            finding = "process execution API";
        else if (contains(line, "strcpy(") || contains(line, "strcat(") ||
                 contains(line, "sprintf("))
            finding = "unbounded string API";
        else if (contains(line, "VirtualAlloc(") || contains(line, "mmap("))
            finding = "manual memory mapping; review bounds and permissions";
        else if (contains(line, "__asm") || contains(line, "asm volatile"))
            finding = "inline assembly; architecture-specific review required";

        if (finding != NULL) {
            (void)snprintf(item, sizeof(item), "line %u: %s\n", line_no, finding);
            used = strlen(details);
            need = strlen(item);
            if (used + need + 1u < cap) memcpy(details + used, item, need + 1u);
            totals->findings++;
        }
    }

    if (ferror(f)) {
        (void)fclose(f);
        totals->read_errors++;
        return -1;
    }
    (void)fclose(f);
    return 0;
}

static void json_string(FILE *f, const char *s) {
    const unsigned char *p = (const unsigned char *)(s != NULL ? s : "");
    fputc('"', f);
    for (; *p != 0u; ++p) {
        if (*p == '"' || *p == '\\') {
            fputc('\\', f);
            fputc(*p, f);
        } else if (*p < 32u) {
            (void)fprintf(f, "\\u%04x", (unsigned)*p);
        } else {
            fputc(*p, f);
        }
    }
    fputc('"', f);
}

int pl_write_json(const char *path, const pl_cpu_info *cpu,
                  const pl_benchmark *bench, const pl_analysis *analysis) {
    FILE *f;
    int bad;

    if (path == NULL || cpu == NULL || bench == NULL || analysis == NULL) return -1;
    f = fopen(path, "wb");
    if (f == NULL) return -1;

    fputs("{\n  \"version\": \"0.5.0\",\n  \"cpu\": {\"vendor\":", f);
    json_string(f, cpu->vendor);
    fputs(",\"model\":", f);
    json_string(f, cpu->model);
    (void)fprintf(f, ",\"logical_cpus\":%u,\"family\":%u,\"model_id\":%u},\n",
                  cpu->logical_cpus, cpu->family, cpu->model_id);
    (void)fprintf(f,
                  "  \"benchmark\": {\"iterations\":%llu,\"elapsed_ms\":%.3f,\"iterations_per_ms\":%.3f},\n",
                  (unsigned long long)bench->iterations, bench->elapsed_ms, bench->score);
    (void)fprintf(f,
                  "  \"analysis\": {\"files_scanned\":%u,\"findings\":%u,\"read_errors\":%u}\n}\n",
                  analysis->files_scanned, analysis->findings, analysis->read_errors);

    bad = ferror(f);
    if (fclose(f) != 0) bad = 1;
    return bad ? -1 : 0;
}
