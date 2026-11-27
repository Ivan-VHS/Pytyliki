#include "processor_evidence.h"
#include "processor_affinity.h"
#include "processor_arch.h"
#include "platform_adapters.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

static pl_tr_platform_event make_event(uint64_t timestamp,uint32_t cpu,uint32_t code) {
    pl_tr_platform_event event={0};
    event.timestamp_epoch_ns=timestamp;
    event.source_record_id=code;
    event.provider=PL_TR_PROVIDER_LINUX_EDAC;
    event.event_code=code;
    event.cpu_id=cpu;
    event.severity=PL_TR_SEVERITY_WARNING;
    event.kind=PL_TR_EVIDENCE_CORRECTED;
    memcpy(event.source_identity,"mc0/ce_count",sizeof "mc0/ce_count");
    return event;
}
static void test_sha256(void) {
    char digest[PL_TR_SHA256_HEX_SIZE];
    assert(pl_tr_sha256_hex("abc",3u,digest)==0);
    assert(strcmp(digest,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")==0);
}
static void test_append_verify_export_and_tamper(void) {
    pl_tr_evidence_ledger ledger;
    pl_tr_platform_event one=make_event(1000u,2u,1u);
    pl_tr_platform_event two=make_event(2000u,2u,1u);
    uint64_t matches[4]={0};
    FILE *file;
    char line[2048];
    pl_tr_evidence_init(&ledger);
    assert(pl_tr_evidence_append(&ledger,&one)==PL_TR_EVIDENCE_OK);
    assert(pl_tr_evidence_append(&ledger,&two)==PL_TR_EVIDENCE_OK);
    assert(pl_tr_evidence_verify(&ledger)==PL_TR_EVIDENCE_OK);
    assert(pl_tr_evidence_find_cpu_window(&ledger,2u,1000u,1500u,matches,4u)==1u);
    assert(matches[0]==1u);
    assert(pl_tr_evidence_find_cpu_window(&ledger,PL_TR_UNKNOWN_CPU,1u,3000u,NULL,0u)==0u);
#if defined(_WIN32)
    assert(tmpfile_s(&file)==0);
#else
    file=tmpfile();
#endif
    assert(file!=NULL);
    assert(pl_tr_evidence_export_jsonl(&ledger,file)==PL_TR_EVIDENCE_OK);
    rewind(file);
    assert(fgets(line,sizeof line,file)!=NULL);
    assert(strstr(line,"pytyliki.platform-evidence.v1")!=NULL);
    fclose(file);
    ledger.entries[0].event.counter_total++;
    assert(pl_tr_evidence_verify(&ledger)==PL_TR_EVIDENCE_INTEGRITY);
}
static void test_invalid_and_full(void) {
    pl_tr_evidence_ledger ledger;
    pl_tr_platform_event event=make_event(10u,PL_TR_UNKNOWN_CPU,1u);
    pl_tr_evidence_init(&ledger);
    event.source_identity[0]='m';
    event.source_identity[1]='c';
    event.source_identity[2]='0';
    event.source_identity[3]='/';
    event.source_identity[4]='c';
    event.source_identity[5]='e';
    event.source_identity[6]='_';
    event.source_identity[7]='c';
    event.source_identity[8]='o';
    event.source_identity[9]='u';
    event.source_identity[10]='n';
    event.source_identity[11]='t';
    event.source_identity[12]='\0';
    assert(pl_tr_evidence_append(&ledger,&event)==PL_TR_EVIDENCE_OK);
    event=make_event(20u,0u,1u);
    for(size_t i=1u;i<PL_TR_EVIDENCE_CAPACITY;++i) {
        event.timestamp_epoch_ns=(uint64_t)i+20u;
        assert(pl_tr_evidence_append(&ledger,&event)==PL_TR_EVIDENCE_OK);
    }
    event.timestamp_epoch_ns=999u;
    assert(pl_tr_evidence_append(&ledger,&event)==PL_TR_EVIDENCE_FULL);
    assert(pl_tr_evidence_verify(&ledger)==PL_TR_EVIDENCE_OK);
}
static void test_affinity_rejects_invalid_requests(void) {
    pl_tr_affinity_lease lease;
    pl_tr_affinity_lease_init(&lease);
    assert(pl_tr_affinity_preview(UINT32_MAX,UINT32_MAX)<0);
    assert(pl_tr_affinity_apply_current(UINT32_MAX,UINT32_MAX,&lease)<0);
    assert(pl_tr_affinity_restore_current(&lease)<0);
}
static void test_whea_wait_bounds(void) {
#if defined(_WIN32)
    pl_tr_platform_event event;
    assert(pl_tr_windows_whea_wait_next(0u,&event)==-1);
    assert(pl_tr_windows_whea_wait_next(60001u,&event)==-1);
#endif
}
static void test_assembly_cpuid(void) {
#if defined(PYTYLIKI_HAVE_ASM_CPUID)
    uint32_t signature=0u,apic=0u;
    assert(pl_tr_arch_cpuid_leaf1(&signature,&apic)==0);
    (void)signature;
    (void)apic;
#endif
}
int main(void) {
    test_sha256();
    test_append_verify_export_and_tamper();
    test_invalid_and_full();
    test_affinity_rejects_invalid_requests();
    test_whea_wait_bounds();
    test_assembly_cpuid();
    puts("Pytyliki platform evidence tests passed.");
    return 0;
}
