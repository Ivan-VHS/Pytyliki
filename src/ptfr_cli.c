#include "platform_adapters.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
static void usage(void) {
    puts("Usage: pytyliki-ptfr --whea [after-record-id] --jsonl FILE [--max-events N]
       pytyliki-ptfr --whea-wait-ms 1..60000 --jsonl FILE");
    puts("Queries recent WHEA Logger records or waits for one Kernel-WHEA CPER notification.");
}
#else
static void usage(void) {
    puts("Usage: pytyliki-ptfr --edac-root SYSFS_ROOT --jsonl FILE");
    puts("Reads Linux EDAC counters from sysfs through the optional Rust adapter.");
}
#endif

static int parse_u64(const char *text,uint64_t *value) {
    char *end=NULL;
    unsigned long long parsed;
    if(text==NULL || value==NULL || text[0]=='-' || text[0]=='\0') return -1;
    errno=0;
    parsed=strtoull(text,&end,10);
    if(errno!=0 || end==text || *end!='\0') return -1;
    *value=(uint64_t)parsed;
    return 0;
}
int main(int argc,char **argv) {
    pl_tr_evidence_ledger ledger;
    const char *output_path=NULL;
    FILE *output=NULL;
    size_t added=0u;
    int status=-1;
    pl_tr_evidence_init(&ledger);

    if(argc==2 && strcmp(argv[1],"--help")==0) {
        usage();
        return 0;
    }
#if defined(_WIN32)
    {
        uint64_t watermark=0u;
        size_t maximum=64u;
        int i;
        int selected=0;
        uint64_t wait_ms=0u;
        for(i=1;i<argc;++i) {
            if(strcmp(argv[i],"--whea")==0) selected=1;
            else if(strcmp(argv[i],"--whea-wait-ms")==0 && i+1<argc) {
                if(parse_u64(argv[++i],&wait_ms)!=0 || wait_ms<1u || wait_ms>60000u) return 2;
                selected=2;
            }
            else if(strcmp(argv[i],"--jsonl")==0 && i+1<argc) output_path=argv[++i];
            else if(strcmp(argv[i],"--max-events")==0 && i+1<argc) {
                uint64_t parsed;
                if(parse_u64(argv[++i],&parsed)!=0 || parsed<1u || parsed>128u) return 2;
                maximum=(size_t)parsed;
            } else if(argv[i][0]!='-' && parse_u64(argv[i],&watermark)==0) {
                /* Optional last-seen WHEA event-record ID. */
            } else {
                usage(); return 2;
            }
        }
        if(!selected || output_path==NULL) { usage(); return 2; }
        if(selected==2) {
            pl_tr_platform_event event;
            status=pl_tr_windows_whea_wait_next((uint32_t)wait_ms,&event);
            if(status==0) {
                status=pl_tr_evidence_append(&ledger,&event);
                if(status==PL_TR_EVIDENCE_OK) added=1u;
            }
        } else {
            status=pl_tr_windows_whea_collect(&ledger,watermark,maximum,&added);
        }
    }
#else
    {
        const char *sysfs_root=NULL;
        int i;
        for(i=1;i<argc;++i) {
            if(strcmp(argv[i],"--edac-root")==0 && i+1<argc) sysfs_root=argv[++i];
            else if(strcmp(argv[i],"--jsonl")==0 && i+1<argc) output_path=argv[++i];
            else { usage(); return 2; }
        }
        if(sysfs_root==NULL || output_path==NULL) { usage(); return 2; }
        status=pl_tr_linux_edac_collect(sysfs_root,&ledger,&added);
    }
#endif
    if(status!=0) {
        fprintf(stderr,"PTFR platform collection failed: %d\n",status);
        return 1;
    }
#if defined(_WIN32)
    if(fopen_s(&output,output_path,"wb")!=0) output=NULL;
#else
    output=fopen(output_path,"wb");
#endif
    if(output==NULL) {
        fprintf(stderr,"Cannot open evidence output: %s\n",output_path);
        return 1;
    }
    status=pl_tr_evidence_export_jsonl(&ledger,output);
    if(fclose(output)!=0 && status==PL_TR_EVIDENCE_OK)
        status=PL_TR_EVIDENCE_IO;
    if(status!=PL_TR_EVIDENCE_OK) {
        fprintf(stderr,"Evidence export failed: %d\n",status);
        return 1;
    }
    printf("PTFR evidence: %zu record(s), SHA-256 chain verified, output=%s\n",
           added,output_path);
    puts("Collection is read-only; CPU placement and hardware configuration were not changed.");
    return 0;
}
