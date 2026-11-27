#ifndef PYTYLIKI_PROCESSOR_THREAD_RECOVERY_ARCH_H
#define PYTYLIKI_PROCESSOR_THREAD_RECOVERY_ARCH_H
#ifdef __cplusplus
extern "C" {
#endif
/* Architecture-specific low-power spin hint; never waits or touches privileged state. */
void pl_tr_cpu_relax(void);
#ifdef __cplusplus
}
#endif
#endif
