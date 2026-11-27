#include "ptfr_evidence_link.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

static pl_tr_platform_event event_make(uint64_t timestamp) {
    pl_tr_platform_event event={0};
    event.timestamp_epoch_ns=timestamp;
    event.source_record_id=timestamp;
    event.provider=PL_TR_PROVIDER_WHEA;
    event.event_code=0x41u;
    event.cpu_id=2u;
    event.severity=PL_TR_SEVERITY_WARNING;
    event.kind=PL_TR_EVIDENCE_CORRECTED;
    memcpy(event.source_identity,"Microsoft-Windows-WHEA-Logger",
           sizeof "Microsoft-Windows-WHEA-Logger");
    return event;
}
static void test_verified_link_reaches_ptfr(void) {
    pl_tr_evidence_ledger evidence;
    pl_tr_worker_link_ledger links;
    pl_tr_monitor monitor;
    pl_tr_platform_event first=event_make(1000u);
    pl_tr_platform_event second=event_make(2000u);
    FILE *file;
    char line[2048];

    pl_tr_evidence_init(&evidence);
    pl_tr_worker_links_init(&links);
    pl_tr_monitor_init(&monitor);
    assert(pl_tr_evidence_append(&evidence,&first)==PL_TR_EVIDENCE_OK);
    assert(pl_tr_evidence_append(&evidence,&second)==PL_TR_EVIDENCE_OK);

    assert(pl_tr_evidence_ingest_worker_fault(
        &evidence,&links,&monitor,1u,77u,2u,5000u,5030u,50u,1)
        ==PL_TR_LINK_OK);
    assert(pl_tr_evidence_ingest_worker_fault(
        &evidence,&links,&monitor,2u,77u,2u,6000u,6020u,50u,1)
        ==PL_TR_LINK_OK);
    assert(links.count==2u);
    assert(pl_tr_worker_links_verify(&links)==PL_TR_LINK_OK);
    {
        pl_tr_observation event={77u,7000u,2u,0x41u,PL_TR_FAULT_CORRECTED_HARDWARE};
        assert(pl_tr_record(&monitor,&event)==PL_TR_OK);
    }
    {
        pl_tr_policy policy;
        pl_tr_analysis analysis;
        pl_tr_decision decision;
        pl_tr_policy_default(&policy);
        assert(pl_tr_analyze(&monitor,77u,0x41u,
            PL_TR_FAULT_CORRECTED_HARDWARE,&policy,&analysis)==PL_TR_OK);
        assert(analysis.observations==3u);
        assert(pl_tr_plan_recovery(&analysis,PL_TR_TASK_MANAGED_POOL,
            &policy,&decision)==PL_TR_OK);
        assert(decision.action==PL_TR_ACTION_OBSERVE);
    }
#if defined(_WIN32)
    assert(tmpfile_s(&file)==0);
#else
    file=tmpfile();
#endif
    assert(file!=NULL);
    assert(pl_tr_worker_links_export_jsonl(&links,file)==PL_TR_LINK_OK);
    rewind(file);
    assert(fgets(line,sizeof line,file)!=NULL);
    assert(strstr(line,"pytyliki.ptfr-evidence-link.v1")!=NULL);
    fclose(file);
    links.entries[0].worker_id++;
    assert(pl_tr_worker_links_verify(&links)==PL_TR_LINK_INTEGRITY);
}
static void test_unknown_cpu_and_counter_snapshot_are_not_attributed(void) {
    pl_tr_evidence_ledger evidence;
    pl_tr_worker_link_ledger links;
    pl_tr_monitor monitor;
    pl_tr_platform_event unknown=event_make(1000u);
    pl_tr_platform_event snapshot=event_make(2000u);
    pl_tr_evidence_init(&evidence);
    pl_tr_worker_links_init(&links);
    pl_tr_monitor_init(&monitor);
    unknown.cpu_id=PL_TR_UNKNOWN_CPU;
    assert(pl_tr_evidence_append(&evidence,&unknown)==PL_TR_EVIDENCE_OK);
    snapshot.kind=PL_TR_EVIDENCE_COUNTER_SNAPSHOT;
    assert(pl_tr_evidence_append(&evidence,&snapshot)==PL_TR_EVIDENCE_OK);
    assert(pl_tr_evidence_ingest_worker_fault(
        &evidence,&links,&monitor,1u,77u,2u,100u,100u,10u,1)
        ==PL_TR_LINK_UNATTRIBUTED);
    assert(pl_tr_evidence_ingest_worker_fault(
        &evidence,&links,&monitor,2u,77u,2u,100u,100u,10u,1)
        ==PL_TR_LINK_UNATTRIBUTED);
    assert(links.count==0u);
}
static void test_time_and_verification_gates(void) {
    pl_tr_evidence_ledger evidence;
    pl_tr_worker_link_ledger links;
    pl_tr_monitor monitor;
    pl_tr_platform_event event=event_make(1000u);
    pl_tr_evidence_init(&evidence);
    pl_tr_worker_links_init(&links);
    pl_tr_monitor_init(&monitor);
    assert(pl_tr_evidence_append(&evidence,&event)==PL_TR_EVIDENCE_OK);
    assert(pl_tr_evidence_ingest_worker_fault(
        &evidence,&links,&monitor,1u,1u,2u,100u,200u,10u,1)
        ==PL_TR_LINK_TIME_MISMATCH);
    assert(pl_tr_evidence_ingest_worker_fault(
        &evidence,&links,&monitor,1u,1u,2u,100u,100u,10u,0)
        ==PL_TR_LINK_INVALID);
}
int main(void) {
    test_verified_link_reaches_ptfr();
    test_unknown_cpu_and_counter_snapshot_are_not_attributed();
    test_time_and_verification_gates();
    puts("Pytyliki PTFR evidence bridge tests passed.");
    return 0;
}
