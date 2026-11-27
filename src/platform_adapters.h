#ifndef PYTYLIKI_PLATFORM_ADAPTERS_H
#define PYTYLIKI_PLATFORM_ADAPTERS_H

#include "processor_evidence.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Read-only pull of the latest WHEA Logger event records from the local System log. */
#if defined(_WIN32)
int pl_tr_windows_whea_collect(pl_tr_evidence_ledger *ledger,
                               uint64_t after_record_id,
                               size_t maximum_events,
                               size_t *events_added);
#endif

/* Optional Rust static adapter; reads EDAC sysfs counters without writing to sysfs. */
#if defined(__linux__)
int pl_tr_linux_edac_collect(const char *sysfs_root,
                             pl_tr_evidence_ledger *ledger,
                             size_t *events_added);
#endif

#ifdef __cplusplus
}
#endif
#endif
