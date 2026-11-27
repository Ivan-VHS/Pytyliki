#include "processor_arch.h"
#if defined(PYTYLIKI_HAVE_ASM_CPUID)
extern int pl_tr_asm_cpuid_leaf1(uint32_t *signature,uint32_t *initial_apic_id);
#endif
int pl_tr_arch_cpuid_leaf1(uint32_t *signature,uint32_t *initial_apic_id) {
    if(signature==0 || initial_apic_id==0) return -1;
#if defined(PYTYLIKI_HAVE_ASM_CPUID)
    return pl_tr_asm_cpuid_leaf1(signature,initial_apic_id);
#else
    *signature=0u;
    *initial_apic_id=0u;
    return -2;
#endif
}
