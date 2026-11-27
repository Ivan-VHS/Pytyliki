.text
.globl pl_tr_asm_cpuid_leaf1
.type pl_tr_asm_cpuid_leaf1,@function
pl_tr_asm_cpuid_leaf1:
    pushq %rbx
    xorl %eax, %eax
    cpuid
    cmpl $1, %eax
    jb .Lno_leaf
    movl $1, %eax
    xorl %ecx, %ecx
    cpuid
    movl %eax, (%rdi)
    shrl $24, %ebx
    movl %ebx, (%rsi)
    xorl %eax, %eax
    popq %rbx
    ret
.Lno_leaf:
    movl $0, (%rdi)
    movl $0, (%rsi)
    movl $-1, %eax
    popq %rbx
    ret
.size pl_tr_asm_cpuid_leaf1, .-pl_tr_asm_cpuid_leaf1
.section .note.GNU-stack,"",@progbits
