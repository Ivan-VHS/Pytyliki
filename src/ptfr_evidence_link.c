#include "ptfr_evidence_link.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static const char zero_hash[] =
    "0000000000000000000000000000000000000000000000000000000000000000";

static int hash_link(const pl_tr_worker_evidence_link *link,
                     char output[PL_TR_SHA256_HEX_SIZE]) {
    char canonical[512];
    int length=snprintf(canonical,sizeof canonical,
        "pytyliki.ptfr-evidence-link.v1|%" PRIu64 "|%" PRIu64
        "|%" PRIu64 "|%" PRIu64 "|%" PRIu64 "|%" PRIu64
        "|%u|%u|%u|%s",
        link->sequence,link->evidence_sequence,link->worker_id,
        link->worker_fault_monotonic_ns,link->mapped_evidence_monotonic_ns,
        link->allowed_delta_ns,link->cpu_id,link->fault_code,
        link->fault_kind,link->evidence_sha256);
    if(length<0 || (size_t)length>=sizeof canonical) return -1;
    return pl_tr_sha256_hex(canonical,(size_t)length,output);
}
static int hash_chain(const pl_tr_worker_evidence_link *link,
                      const char *previous,
                      char output[PL_TR_SHA256_HEX_SIZE]) {
    char canonical[192];
    int length=snprintf(canonical,sizeof canonical,
        "pytyliki.ptfr-link-chain.v1|%" PRIu64 "|%s|%s",
        link->sequence,previous,link->link_sha256);
    if(length<0 || (size_t)length>=sizeof canonical) return -1;
    return pl_tr_sha256_hex(canonical,(size_t)length,output);
}

void pl_tr_worker_links_init(pl_tr_worker_link_ledger *ledger) {
    if(ledger==NULL) return;
    memset(ledger,0,sizeof(*ledger));
    memcpy(ledger->chain_head,zero_hash,sizeof zero_hash);
}

int pl_tr_worker_links_verify(const pl_tr_worker_link_ledger *ledger) {
    char previous[PL_TR_SHA256_HEX_SIZE];
    size_t i;
    if(ledger==NULL || ledger->count>PL_TR_LINK_CAPACITY)
        return PL_TR_LINK_INVALID;
    memcpy(previous,zero_hash,sizeof previous);
    for(i=0u;i<ledger->count;++i) {
        const pl_tr_worker_evidence_link *entry=&ledger->entries[i];
        char link_hash[PL_TR_SHA256_HEX_SIZE];
        char chain_hash[PL_TR_SHA256_HEX_SIZE];
        if(entry->sequence!=(uint64_t)i+1u ||
           entry->evidence_sequence==0u || entry->worker_id==0u ||
           entry->worker_fault_monotonic_ns==0u ||
           entry->mapped_evidence_monotonic_ns==0u ||
           entry->allowed_delta_ns==0u ||
           entry->cpu_id==PL_TR_UNKNOWN_CPU ||
           entry->fault_code==0u ||
           entry->fault_kind<PL_TR_FAULT_CORRECTED_HARDWARE ||
           entry->fault_kind>PL_TR_FAULT_FATAL ||
           hash_link(entry,link_hash)!=0 ||
           strcmp(link_hash,entry->link_sha256)!=0 ||
           hash_chain(entry,previous,chain_hash)!=0 ||
           strcmp(chain_hash,entry->chain_sha256)!=0)
            return PL_TR_LINK_INTEGRITY;
        memcpy(previous,chain_hash,sizeof previous);
    }
    return strcmp(previous,ledger->chain_head)==0
        ? PL_TR_LINK_OK : PL_TR_LINK_INTEGRITY;
}

int pl_tr_evidence_ingest_worker_fault(
    const pl_tr_evidence_ledger *evidence,
    pl_tr_worker_link_ledger *links,
    pl_tr_monitor *monitor,
    uint64_t evidence_sequence,
    uint64_t worker_id,
    uint32_t worker_cpu_id,
    uint64_t worker_fault_monotonic_ns,
    uint64_t mapped_evidence_monotonic_ns,
    uint64_t allowed_delta_ns,
    int attribution_verified) {
    const pl_tr_evidence_entry *source;
    pl_tr_worker_evidence_link candidate;
    pl_tr_observation observation;
    uint64_t delta;
    pl_tr_fault_kind kind;

    if(evidence==NULL || links==NULL || monitor==NULL ||
       evidence_sequence==0u || evidence_sequence>evidence->count ||
       worker_id==0u || worker_cpu_id==PL_TR_UNKNOWN_CPU ||
       worker_fault_monotonic_ns==0u || mapped_evidence_monotonic_ns==0u ||
       allowed_delta_ns==0u || attribution_verified!=1)
        return PL_TR_LINK_INVALID;
    if(pl_tr_evidence_verify(evidence)!=PL_TR_EVIDENCE_OK ||
       pl_tr_worker_links_verify(links)!=PL_TR_LINK_OK)
        return PL_TR_LINK_INTEGRITY;
    if(links->count>=PL_TR_LINK_CAPACITY) return PL_TR_LINK_CAPACITY;

    source=&evidence->entries[(size_t)evidence_sequence-1u];
    if(source->sequence!=evidence_sequence) return PL_TR_LINK_INVALID;
    if(source->event.cpu_id==PL_TR_UNKNOWN_CPU ||
       source->event.cpu_id!=worker_cpu_id)
        return PL_TR_LINK_UNATTRIBUTED;

    delta=worker_fault_monotonic_ns>=mapped_evidence_monotonic_ns
        ? worker_fault_monotonic_ns-mapped_evidence_monotonic_ns
        : mapped_evidence_monotonic_ns-worker_fault_monotonic_ns;
    if(delta>allowed_delta_ns) return PL_TR_LINK_TIME_MISMATCH;

    switch(source->event.kind) {
    case PL_TR_EVIDENCE_CORRECTED: kind=PL_TR_FAULT_CORRECTED_HARDWARE; break;
    case PL_TR_EVIDENCE_UNCORRECTED: kind=PL_TR_FAULT_UNCORRECTED_HARDWARE; break;
    case PL_TR_EVIDENCE_FATAL: kind=PL_TR_FAULT_FATAL; break;
    default: return PL_TR_LINK_UNATTRIBUTED;
    }
    if(source->event.event_code==0u) return PL_TR_LINK_INVALID;

    memset(&candidate,0,sizeof candidate);
    candidate.sequence=(uint64_t)links->count+1u;
    candidate.evidence_sequence=evidence_sequence;
    candidate.worker_id=worker_id;
    candidate.worker_fault_monotonic_ns=worker_fault_monotonic_ns;
    candidate.mapped_evidence_monotonic_ns=mapped_evidence_monotonic_ns;
    candidate.allowed_delta_ns=allowed_delta_ns;
    candidate.cpu_id=worker_cpu_id;
    candidate.fault_code=source->event.event_code;
    candidate.fault_kind=(uint32_t)kind;
    memcpy(candidate.evidence_sha256,source->event_sha256,
           sizeof candidate.evidence_sha256);
    if(hash_link(&candidate,candidate.link_sha256)!=0 ||
       hash_chain(&candidate,links->chain_head,candidate.chain_sha256)!=0)
        return PL_TR_LINK_INVALID;

    observation.worker_id=worker_id;
    observation.timestamp_ns=worker_fault_monotonic_ns;
    observation.cpu_id=worker_cpu_id;
    observation.fault_code=source->event.event_code;
    observation.kind=kind;
    {
        int record_status=pl_tr_record(monitor,&observation);
        if(record_status!=PL_TR_OK) return PL_TR_LINK_MONITOR_ERROR;
    }
    links->entries[links->count]=candidate;
    memcpy(links->chain_head,candidate.chain_sha256,
           sizeof links->chain_head);
    links->count++;
    return PL_TR_LINK_OK;
}

int pl_tr_worker_links_export_jsonl(
    const pl_tr_worker_link_ledger *ledger,FILE *output) {
    size_t i;
    if(ledger==NULL || output==NULL ||
       pl_tr_worker_links_verify(ledger)!=PL_TR_LINK_OK)
        return PL_TR_LINK_INVALID;
    for(i=0u;i<ledger->count;++i) {
        const pl_tr_worker_evidence_link *entry=&ledger->entries[i];
        if(fprintf(output,
            "{\"schema\":\"pytyliki.ptfr-evidence-link.v1\","
            "\"sequence\":\"%" PRIu64 "\","
            "\"evidence_sequence\":\"%" PRIu64 "\","
            "\"worker_id\":\"%" PRIu64 "\","
            "\"worker_fault_monotonic_ns\":\"%" PRIu64 "\","
            "\"mapped_evidence_monotonic_ns\":\"%" PRIu64 "\","
            "\"allowed_delta_ns\":\"%" PRIu64 "\","
            "\"cpu_id\":%u,\"fault_code\":%u,\"fault_kind\":%u,"
            "\"evidence_sha256\":\"%s\",\"link_sha256\":\"%s\","
            "\"chain_sha256\":\"%s\"}\n",
            entry->sequence,entry->evidence_sequence,entry->worker_id,
            entry->worker_fault_monotonic_ns,entry->mapped_evidence_monotonic_ns,
            entry->allowed_delta_ns,entry->cpu_id,entry->fault_code,
            entry->fault_kind,entry->evidence_sha256,entry->link_sha256,
            entry->chain_sha256)<0) return PL_TR_LINK_INVALID;
    }
    return fflush(output)==0 ? PL_TR_LINK_OK : PL_TR_LINK_INVALID;
}
