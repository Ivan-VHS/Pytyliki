#if defined(_WIN32)
#include "platform_adapters.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winevt.h>

#include <stdlib.h>
#include <string.h>

#define PL_TR_WHEA_MAX_EVENTS 128u
#define PL_TR_WHEA_MAX_XML_BYTES (1024u * 1024u)
#define PL_TR_FILETIME_EPOCH_DELTA UINT64_C(116444736000000000)

static uint64_t filetime_to_epoch_ns(ULONGLONG filetime) {
    if(filetime<PL_TR_FILETIME_EPOCH_DELTA) return 0u;
    return (filetime-PL_TR_FILETIME_EPOCH_DELTA)*UINT64_C(100);
}
static uint32_t event_level_to_severity(BYTE level) {
    switch(level) {
    case 1u: return PL_TR_SEVERITY_CRITICAL;
    case 2u: return PL_TR_SEVERITY_ERROR;
    case 3u: return PL_TR_SEVERITY_WARNING;
    case 4u: return PL_TR_SEVERITY_INFO;
    default: return PL_TR_SEVERITY_UNKNOWN;
    }
}
static int render_event(EVT_HANDLE handle,pl_tr_platform_event *out) {
    EVT_VARIANT values[EvtSystemPropertyIdEND];
    DWORD buffer_used=0u,property_count=0u;
    DWORD xml_bytes=0u,xml_props=0u;
    wchar_t *xml=NULL;
    DWORD xml_chars;
    ULONGLONG record_id=0u,system_time=0u;
    WORD event_id=0u;
    BYTE level=0u;
    int status=0;

    memset(values,0,sizeof values);
    if(!EvtRender(NULL,handle,EvtRenderEventValues,sizeof values,
                  values,&buffer_used,&property_count) ||
       property_count<=EvtSystemEventRecordId) return -1;

    if(values[EvtSystemEventRecordId].Type==EvtVarTypeUInt64)
        record_id=values[EvtSystemEventRecordId].UInt64Val;
    else if(values[EvtSystemEventRecordId].Type==EvtVarTypeInt64 &&
            values[EvtSystemEventRecordId].Int64Val>=0)
        record_id=(ULONGLONG)values[EvtSystemEventRecordId].Int64Val;
    if(values[EvtSystemEventID].Type==EvtVarTypeUInt16)
        event_id=values[EvtSystemEventID].UInt16Val;
    if(values[EvtSystemLevel].Type==EvtVarTypeByte)
        level=values[EvtSystemLevel].ByteVal;
    if(values[EvtSystemTimeCreated].Type==EvtVarTypeFileTime)
        system_time=values[EvtSystemTimeCreated].FileTimeVal;

    if(EvtRender(NULL,handle,EvtRenderEventXml,0,NULL,&xml_bytes,&xml_props) ||
       GetLastError()!=ERROR_INSUFFICIENT_BUFFER || xml_bytes==0u ||
       xml_bytes>PL_TR_WHEA_MAX_XML_BYTES || (xml_bytes%sizeof(wchar_t))!=0u)
        return -2;
    xml=(wchar_t *)malloc(xml_bytes);
    if(xml==NULL) return -3;
    if(!EvtRender(NULL,handle,EvtRenderEventXml,xml_bytes,xml,&xml_bytes,&xml_props)) {
        free(xml);
        return -4;
    }
    xml_chars=xml_bytes/(DWORD)sizeof(wchar_t);
    if(xml_chars>0u && xml[xml_chars-1u]==L'\0') --xml_chars;

    memset(out,0,sizeof(*out));
    out->timestamp_epoch_ns=filetime_to_epoch_ns(system_time);
    out->source_record_id=(uint64_t)record_id;
    out->provider=PL_TR_PROVIDER_WHEA;
    out->event_code=(uint32_t)event_id;
    out->cpu_id=PL_TR_UNKNOWN_CPU;
    out->severity=event_level_to_severity(level);
    out->kind=PL_TR_EVIDENCE_UNKNOWN;
    memcpy(out->source_identity,"Microsoft-Windows-WHEA-Logger",
           sizeof "Microsoft-Windows-WHEA-Logger");
    status=pl_tr_sha256_hex(xml,(size_t)xml_chars*sizeof(wchar_t),
                            out->payload_sha256);
    free(xml);
    return status;
}

int pl_tr_windows_whea_collect(pl_tr_evidence_ledger *ledger,
                               uint64_t after_record_id,
                               size_t maximum_events,
                               size_t *events_added) {
    static const wchar_t query_text[]=
        L"*[System[Provider[@Name='Microsoft-Windows-WHEA-Logger']]]";
    EVT_HANDLE query=NULL;
    pl_tr_platform_event pending[PL_TR_WHEA_MAX_EVENTS];
    size_t count=0u,i,added=0u;
    int result=0;
    if(ledger==NULL || events_added==NULL || maximum_events==0u ||
       maximum_events>PL_TR_WHEA_MAX_EVENTS) return -1;
    *events_added=0u;
    query=EvtQuery(NULL,L"System",query_text,
                   EvtQueryChannelPath|EvtQueryReverseDirection);
    if(query==NULL) return -2;

    while(count<maximum_events) {
        EVT_HANDLE event=NULL;
        DWORD returned=0u;
        pl_tr_platform_event normalized;
        if(!EvtNext(query,1u,&event,0u,0u,&returned)) {
            const DWORD error=GetLastError();
            if(error!=ERROR_NO_MORE_ITEMS) result=-3;
            break;
        }
        if(render_event(event,&normalized)!=0) {
            EvtClose(event);
            result=-4;
            break;
        }
        EvtClose(event);
        if(normalized.source_record_id<=after_record_id) break;
        if(normalized.timestamp_epoch_ns==0u) continue;
        pending[count++]=normalized;
    }
    EvtClose(query);
    if(result!=0) return result;

    /* EvtQuery is reverse-ordered; append oldest first for deterministic chains. */
    for(i=count;i>0u;--i) {
        const int append_status=pl_tr_evidence_append(ledger,&pending[i-1u]);
        if(append_status!=PL_TR_EVIDENCE_OK) return append_status;
        ++added;
    }
    *events_added=added;
    return 0;
}
#endif
