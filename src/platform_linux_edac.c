#if defined(__linux__) && defined(PYTYLIKI_HAVE_RUST_EDAC)
#include "platform_adapters.h"

#include <stdint.h>

typedef int32_t (*pl_tr_rust_event_sink)(const pl_tr_platform_event *event,
                                         void *context);
extern int32_t pytyliki_edac_scan(const char *root,
                                  pl_tr_rust_event_sink sink,
                                  void *context);

static int32_t append_event(const pl_tr_platform_event *event,void *context) {
    pl_tr_evidence_ledger *ledger=(pl_tr_evidence_ledger *)context;
    return pl_tr_evidence_append(ledger,event)==PL_TR_EVIDENCE_OK ? 0 : -1;
}

int pl_tr_linux_edac_collect(const char *sysfs_root,
                             pl_tr_evidence_ledger *ledger,
                             size_t *events_added) {
    int32_t result;
    size_t before;
    if(sysfs_root==NULL || ledger==NULL || events_added==NULL) return -1;
    before=ledger->count;
    result=pytyliki_edac_scan(sysfs_root,append_event,ledger);
    if(result<0) return (int)result;
    *events_added=ledger->count-before;
    return 0;
}
#endif
