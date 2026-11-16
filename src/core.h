#ifndef PYTYLIKI_CORE_H
#define PYTYLIKI_CORE_H
#include <stddef.h>
#include <stdint.h>
#define PL_VENDOR_MAX 32
#define PL_MODEL_MAX 96
#define PL_PATH_MAX 1024
typedef struct { char vendor[PL_VENDOR_MAX]; char model[PL_MODEL_MAX]; unsigned logical_cpus; unsigned family; unsigned model_id; } pl_cpu_info;
typedef struct { uint64_t iterations; double elapsed_ms; double score; } pl_benchmark;
typedef struct { unsigned files_scanned; unsigned findings; unsigned read_errors; } pl_analysis;
int pl_cpu_profile(pl_cpu_info *out);
int pl_run_benchmark(uint64_t iterations, pl_benchmark *out);
int pl_analyze_file(const char *path, pl_analysis *totals, char *details, size_t details_cap);
int pl_write_json(const char *path, const pl_cpu_info *cpu, const pl_benchmark *bench, const pl_analysis *analysis);
#endif
