#ifndef PYTYLIKI_PROCESSOR_H
#define PYTYLIKI_PROCESSOR_H
#include "core.h"
#include <stdint.h>
#define PL_ARCH_MAX 32
typedef struct { pl_cpu_info base; char architecture[PL_ARCH_MAX]; uint64_t physical_memory_mib; uint64_t available_memory_mib; unsigned sse2:1; unsigned sse42:1; unsigned avx:1; unsigned avx2:1; unsigned avx512f:1; unsigned aes:1; unsigned fma:1; unsigned virtualization:1; } pl_processor_snapshot;
typedef struct { uint64_t iterations; unsigned memory_mib; unsigned passes; double integer_ms; double memory_ms; double matrix_ms; uint64_t checksum; int integrity_ok; } pl_suite_result;
int pl_processor_scan(pl_processor_snapshot *out);
int pl_processor_benchmark_suite(uint64_t iterations,unsigned memory_mib,unsigned passes,pl_suite_result *out);
#endif
