#if defined(__x86_64__)
    .text
    .globl pl_tr_cpu_relax
    .type pl_tr_cpu_relax,@function
pl_tr_cpu_relax:
    pause
    ret
    .size pl_tr_cpu_relax, .-pl_tr_cpu_relax
#endif
