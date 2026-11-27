#ifndef PYTYLIKI_PROCESSOR_AFFINITY_H
#define PYTYLIKI_PROCESSOR_AFFINITY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_TR_AFFINITY_TOKEN_BYTES 256u
typedef union {
    max_align_t alignment;
    unsigned char bytes[PL_TR_AFFINITY_TOKEN_BYTES];
} pl_tr_affinity_storage;
typedef struct {
    uint32_t magic;
    uint32_t platform;
    uint32_t active;
    pl_tr_affinity_storage storage;
} pl_tr_affinity_lease;

/* These APIs affect only the calling thread, never the system-wide CPU set. */
int pl_tr_affinity_preview(uint32_t processor_group,
                           uint32_t logical_processor);
int pl_tr_affinity_apply_current(uint32_t processor_group,
                                 uint32_t logical_processor,
                                 pl_tr_affinity_lease *lease);
int pl_tr_affinity_restore_current(pl_tr_affinity_lease *lease);

#ifdef __cplusplus
}
#endif
#endif
