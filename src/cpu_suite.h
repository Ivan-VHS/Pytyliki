#ifndef PYTYLIKI_CPU_SUITE_H
#define PYTYLIKI_CPU_SUITE_H
#include <stddef.h>
#include <stdint.h>
#define PL_CPU_SUITE_MODULE_COUNT 78u
#define PL_CPU_SUITE_NAME_MAX 40u
#define PL_CPU_SUITE_DETAIL_MAX 160u
typedef enum { PL_CPU_SUITE_HUB = 0, PL_CPU_SUITE_ARCHITECTURE, PL_CPU_SUITE_THERMAL_POWER, PL_CPU_SUITE_MEMORY_CACHE, PL_CPU_SUITE_SCHEDULER, PL_CPU_SUITE_DIAGNOSTICS_SECURITY, PL_CPU_SUITE_BENCHMARK_LAB } pl_cpu_suite_domain;
typedef struct { uint16_t id; const char *name; pl_cpu_suite_domain domain; const char *description; } pl_cpu_suite_module;
typedef struct { uint16_t module_id; char module_name[PL_CPU_SUITE_NAME_MAX]; char domain_name[24]; char execution_mode[24]; char safety_note[PL_CPU_SUITE_DETAIL_MAX]; } pl_cpu_suite_plan;
/* Immutable 78-entry catalogue; enumeration is deterministic and allocation-free. */
size_t pl_cpu_suite_catalog(const pl_cpu_suite_module **items);
int pl_cpu_suite_get(size_t index, pl_cpu_suite_module *out);
int pl_cpu_suite_find_id(uint16_t id, pl_cpu_suite_module *out);
int pl_cpu_suite_find_name(const char *name, pl_cpu_suite_module *out);
const char *pl_cpu_suite_domain_name(pl_cpu_suite_domain domain);
/* Produces an advisory execution plan only; never writes registers or device settings. */
int pl_cpu_suite_plan_module(uint16_t id, pl_cpu_suite_plan *out);
#endif
