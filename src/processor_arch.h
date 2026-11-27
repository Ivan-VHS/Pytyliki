#ifndef PYTYLIKI_PROCESSOR_ARCH_H
#define PYTYLIKI_PROCESSOR_ARCH_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Read-only CPUID leaf 1 values on supported x86-64 Unix builds. */
int pl_tr_arch_cpuid_leaf1(uint32_t *signature, uint32_t *initial_apic_id);
#ifdef __cplusplus
}
#endif
#endif
