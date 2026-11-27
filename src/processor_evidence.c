#include "processor_evidence.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static const char zero_hash[] =
    "0000000000000000000000000000000000000000000000000000000000000000";

static int valid_source_identity(const char *value) {
    size_t i;
    if (value==NULL || value[0]=='\0' || strstr(value,"..")!=NULL) return 0;
    for (i=0u;i<PL_TR_SOURCE_ID_MAX && value[i]!='\0';++i) {
        const unsigned char ch=(unsigned char)value[i];
        if (!(isalnum(ch) || ch=='_' || ch=='-' || ch=='.' || ch=='/')) return 0;
    }
    return i>0u && i<PL_TR_SOURCE_ID_MAX && value[i]=='\0';
}
static int valid_sha256(const char *value, int allow_empty) {
    size_t i;
    if (value==NULL) return 0;
    if (allow_empty && value[0]=='\0') return 1;
    if (value[64]!='\0') return 0;
    for (i=0u;i<64u;++i)
        if (!((value[i]>='0'&&value[i]<='9') ||
              (value[i]>='a'&&value[i]<='f'))) return 0;
    return 1;
}
static int event_valid(const pl_tr_platform_event *event) {
    return event!=NULL && event->timestamp_epoch_ns>0u &&
        (event->provider==PL_TR_PROVIDER_WHEA ||
         event->provider==PL_TR_PROVIDER_LINUX_EDAC) &&
        event->kind<=PL_TR_EVIDENCE_COUNTER_SNAPSHOT &&
        event->severity<=PL_TR_SEVERITY_CRITICAL &&
        valid_source_identity(event->source_identity) &&
        valid_sha256(event->payload_sha256,1);
}
const char *pl_tr_evidence_provider_name(uint32_t provider) {
    switch(provider) {
    case PL_TR_PROVIDER_WHEA: return "windows-whea";
    case PL_TR_PROVIDER_LINUX_EDAC: return "linux-edac";
    default: return "unknown";
    }
}
const char *pl_tr_evidence_kind_name(uint32_t kind) {
    switch(kind) {
    case PL_TR_EVIDENCE_CORRECTED: return "corrected";
    case PL_TR_EVIDENCE_UNCORRECTED: return "uncorrected";
    case PL_TR_EVIDENCE_FATAL: return "fatal";
    case PL_TR_EVIDENCE_COUNTER_SNAPSHOT: return "counter_snapshot";
    default: return "unknown";
    }
}
const char *pl_tr_evidence_severity_name(uint32_t severity) {
    switch(severity) {
    case PL_TR_SEVERITY_INFO: return "info";
    case PL_TR_SEVERITY_WARNING: return "warning";
    case PL_TR_SEVERITY_ERROR: return "error";
    case PL_TR_SEVERITY_CRITICAL: return "critical";
    default: return "unknown";
    }
}
static int hash_event(const pl_tr_platform_event *event,
                      char output[PL_TR_SHA256_HEX_SIZE]) {
    char canonical[512];
    const int n=snprintf(canonical,sizeof canonical,
        "pytyliki.platform-evidence.v1|%" PRIu64 "|%" PRIu64 "|%" PRIu64
        "|%u|%u|%u|%u|%u|%s|%s",
        event->timestamp_epoch_ns,event->source_record_id,event->counter_total,
        event->provider,event->event_code,event->cpu_id,event->severity,event->kind,
        event->source_identity,event->payload_sha256);
    if(n<0 || (size_t)n>=sizeof canonical) return -1;
    return pl_tr_sha256_hex(canonical,(size_t)n,output);
}
static int hash_chain(uint64_t sequence,const char *previous,
                      const char *event_hash,
                      char output[PL_TR_SHA256_HEX_SIZE]) {
    char canonical[192];
    const int n=snprintf(canonical,sizeof canonical,"pytyliki.evidence-chain.v1|%" PRIu64 "|%s|%s",
                         sequence,previous,event_hash);
    if(n<0 || (size_t)n>=sizeof canonical) return -1;
    return pl_tr_sha256_hex(canonical,(size_t)n,output);
}
void pl_tr_evidence_init(pl_tr_evidence_ledger *ledger) {
    if(ledger==NULL) return;
    memset(ledger,0,sizeof(*ledger));
    memcpy(ledger->chain_head,zero_hash,sizeof zero_hash);
}
int pl_tr_evidence_append(pl_tr_evidence_ledger *ledger,
                          const pl_tr_platform_event *event) {
    pl_tr_evidence_entry *entry;
    pl_tr_platform_event normalized;
    char payload_seed[256];
    int n;
    if(ledger==NULL || !event_valid(event) ||
       pl_tr_evidence_verify(ledger)!=PL_TR_EVIDENCE_OK)
        return PL_TR_EVIDENCE_INVALID;
    if(ledger->count>=PL_TR_EVIDENCE_CAPACITY) return PL_TR_EVIDENCE_FULL;
    normalized=*event;
    if(normalized.payload_sha256[0]=='\0') {
        n=snprintf(payload_seed,sizeof payload_seed,
            "%s|%" PRIu64 "|%u|%u|%" PRIu64,
            normalized.source_identity,normalized.timestamp_epoch_ns,
            normalized.event_code,normalized.kind,normalized.counter_total);
        if(n<0 || (size_t)n>=sizeof payload_seed ||
           pl_tr_sha256_hex(payload_seed,(size_t)n,normalized.payload_sha256)!=0)
            return PL_TR_EVIDENCE_INVALID;
    }
    entry=&ledger->entries[ledger->count];
    memset(entry,0,sizeof(*entry));
    entry->sequence=(uint64_t)ledger->count+1u;
    entry->event=normalized;
    if(hash_event(&entry->event,entry->event_sha256)!=0 ||
       hash_chain(entry->sequence,ledger->chain_head,
                  entry->event_sha256,entry->chain_sha256)!=0)
        return PL_TR_EVIDENCE_INVALID;
    memcpy(ledger->chain_head,entry->chain_sha256,sizeof ledger->chain_head);
    ledger->count++;
    return PL_TR_EVIDENCE_OK;
}
int pl_tr_evidence_verify(const pl_tr_evidence_ledger *ledger) {
    char previous[PL_TR_SHA256_HEX_SIZE];
    size_t i;
    if(ledger==NULL || ledger->count>PL_TR_EVIDENCE_CAPACITY) return PL_TR_EVIDENCE_INVALID;
    memcpy(previous,zero_hash,sizeof previous);
    for(i=0u;i<ledger->count;++i) {
        char event_hash[PL_TR_SHA256_HEX_SIZE];
        char chain_hash[PL_TR_SHA256_HEX_SIZE];
        const pl_tr_evidence_entry *entry=&ledger->entries[i];
        if(entry->sequence!=(uint64_t)i+1u || !event_valid(&entry->event) ||
           hash_event(&entry->event,event_hash)!=0 ||
           strcmp(event_hash,entry->event_sha256)!=0 ||
           hash_chain(entry->sequence,previous,event_hash,chain_hash)!=0 ||
           strcmp(chain_hash,entry->chain_sha256)!=0)
            return PL_TR_EVIDENCE_INTEGRITY;
        memcpy(previous,chain_hash,sizeof previous);
    }
    return strcmp(previous,ledger->chain_head)==0
        ? PL_TR_EVIDENCE_OK : PL_TR_EVIDENCE_INTEGRITY;
}
size_t pl_tr_evidence_find_cpu_window(const pl_tr_evidence_ledger *ledger,
                                      uint32_t cpu_id,
                                      uint64_t start_epoch_ns,
                                      uint64_t end_epoch_ns,
                                      uint64_t *sequences,
                                      size_t sequence_capacity) {
    size_t i,count=0u;
    if(ledger==NULL || cpu_id==PL_TR_UNKNOWN_CPU ||
       start_epoch_ns>end_epoch_ns || ledger->count>PL_TR_EVIDENCE_CAPACITY)
        return 0u;
    for(i=0u;i<ledger->count;++i) {
        const pl_tr_evidence_entry *entry=&ledger->entries[i];
        if(entry->event.cpu_id==cpu_id &&
           entry->event.timestamp_epoch_ns>=start_epoch_ns &&
           entry->event.timestamp_epoch_ns<=end_epoch_ns) {
            if(sequences!=NULL && count<sequence_capacity)
                sequences[count]=entry->sequence;
            count++;
        }
    }
    return count;
}
int pl_tr_evidence_export_jsonl(const pl_tr_evidence_ledger *ledger,FILE *output) {
    size_t i;
    if(ledger==NULL || output==NULL ||
       pl_tr_evidence_verify(ledger)!=PL_TR_EVIDENCE_OK) return PL_TR_EVIDENCE_INVALID;
    for(i=0u;i<ledger->count;++i) {
        const pl_tr_evidence_entry *e=&ledger->entries[i];
        const pl_tr_platform_event *v=&e->event;
        if(fprintf(output,
            "{\"schema\":\"pytyliki.platform-evidence.v1\","
            "\"sequence\":\"%" PRIu64 "\",\"timestamp_epoch_ns\":\"%" PRIu64
            "\",\"provider\":\"%s\",\"event_code\":%u,\"event_kind\":\"%s\","
            "\"severity\":\"%s\",\"cpu_id\":",
            e->sequence,v->timestamp_epoch_ns,pl_tr_evidence_provider_name(v->provider),
            v->event_code,pl_tr_evidence_kind_name(v->kind),
            pl_tr_evidence_severity_name(v->severity))<0) return PL_TR_EVIDENCE_IO;
        if(v->cpu_id==PL_TR_UNKNOWN_CPU) {
            if(fputs("null",output)==EOF) return PL_TR_EVIDENCE_IO;
        } else if(fprintf(output,"%u",v->cpu_id)<0) return PL_TR_EVIDENCE_IO;
        if(fprintf(output,
            ",\"source_record_id\":\"%" PRIu64 "\",\"counter_total\":\"%" PRIu64
            "\",\"source_identity\":\"%s\",\"payload_sha256\":\"%s\","
            "\"event_sha256\":\"%s\",\"chain_sha256\":\"%s\"}\n",
            v->source_record_id,v->counter_total,v->source_identity,v->payload_sha256,
            e->event_sha256,e->chain_sha256)<0) return PL_TR_EVIDENCE_IO;
    }
    return fflush(output)==0 ? PL_TR_EVIDENCE_OK : PL_TR_EVIDENCE_IO;
}
