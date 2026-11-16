#define _POSIX_C_SOURCE 200809L
#include "core.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#if defined(_WIN32)
#include <windows.h>
#include <intrin.h>
#else
#include <unistd.h>
#if defined(__x86_64__) || defined(__i386__)
#include <cpuid.h>
#endif
#endif

static void copy_text(char *dst, size_t cap, const char *src) {
    if (cap) (void)snprintf(dst, cap, "%s", src ? src : "unknown");
}
static void decode_cpu(const unsigned int r[4], pl_cpu_info *out) {
    unsigned int bf=(r[0]>>8)&15u, bm=(r[0]>>4)&15u, ef=(r[0]>>20)&255u, em=(r[0]>>16)&15u;
    out->family = bf==15u ? bf+ef : bf;
    out->model_id = (bf==6u || bf==15u) ? (em<<4)|bm : bm;
}
int pl_cpu_profile(pl_cpu_info *out) {
    if (!out) return -1;
    memset(out,0,sizeof(*out));
#if defined(_WIN32)
    SYSTEM_INFO si; GetSystemInfo(&si); out->logical_cpus=si.dwNumberOfProcessors;
#elif defined(_SC_NPROCESSORS_ONLN)
    { long n=sysconf(_SC_NPROCESSORS_ONLN); out->logical_cpus=n>0?(unsigned)n:1u; }
#else
    out->logical_cpus=1u;
#endif
    if (!out->logical_cpus) out->logical_cpus=1u;
#if defined(_MSC_VER) && (defined(_M_X64)||defined(_M_IX86))
    {
        int r[4]; __cpuid(r,0);
        char v[13]; memcpy(v,&r[1],4); memcpy(v+4,&r[3],4); memcpy(v+8,&r[2],4); v[12]='\0';
        copy_text(out->vendor,sizeof(out->vendor),v);
        __cpuid(r,1); decode_cpu((const unsigned int*)r,out);
        __cpuid(r,(int)0x80000000u);
        if ((unsigned)r[0]>=0x80000004u) {
            char brand[49]; unsigned i;
            for(i=0;i<3u;i++){__cpuid(r,(int)(0x80000002u+i));memcpy(brand+i*16u,r,16u);}
            brand[48]='\0'; copy_text(out->model,sizeof(out->model),brand);
        }
    }
#elif (defined(__x86_64__)||defined(__i386__)) && defined(__GNUC__)
    {
        unsigned a,b,c,d;
        if(__get_cpuid(0,&a,&b,&c,&d)){
            char v[13]; memcpy(v,&b,4);memcpy(v+4,&d,4);memcpy(v+8,&c,4);v[12]='\0';
            copy_text(out->vendor,sizeof(out->vendor),v);
        }
        if(__get_cpuid(1,&a,&b,&c,&d)){const unsigned r[4]={a,b,c,d};decode_cpu(r,out);}
        if(__get_cpuid(0x80000000u,&a,&b,&c,&d)&&a>=0x80000004u){
            char brand[49]; unsigned i;
            for(i=0;i<3u;i++){
                if(!__get_cpuid(0x80000002u+i,&a,&b,&c,&d)) break;
                memcpy(brand+i*16u,&a,4);memcpy(brand+i*16u+4,&b,4);
                memcpy(brand+i*16u+8,&c,4);memcpy(brand+i*16u+12,&d,4);
            }
            if(i==3u){brand[48]='\0';copy_text(out->model,sizeof(out->model),brand);}
        }
    }
#else
    copy_text(out->vendor,sizeof(out->vendor),"not-available");
#endif
    if(!out->vendor[0])copy_text(out->vendor,sizeof(out->vendor),"not-available");
    if(!out->model[0])copy_text(out->model,sizeof(out->model),"CPU model string unavailable on this architecture");
    return 0;
}
static double now_ms(void) {
#if defined(_WIN32)
    LARGE_INTEGER f,t;
    if(!QueryPerformanceFrequency(&f)||!QueryPerformanceCounter(&t))
        return (double)clock()*1000.0/(double)CLOCKS_PER_SEC;
    return 1000.0*(double)t.QuadPart/(double)f.QuadPart;
#else
    struct timespec t;
    if(clock_gettime(CLOCK_MONOTONIC,&t)!=0)
        return (double)clock()*1000.0/(double)CLOCKS_PER_SEC;
    return 1000.0*(double)t.tv_sec+(double)t.tv_nsec/1000000.0;
#endif
}
int pl_run_benchmark(uint64_t iterations, pl_benchmark *out) {
    volatile uint64_t x=UINT64_C(0x9e3779b97f4a7c15);
    if(!out||iterations==0u||iterations>UINT64_C(500000000))return -1;
    double start=now_ms();
    for(uint64_t i=0;i<iterations;i++){x^=x>>12;x^=x<<25;x^=x>>27;x*=UINT64_C(2685821657736338717);}
    double elapsed=now_ms()-start;
    if(x==0u)(void)fputc(' ',stdout);
    out->iterations=iterations;out->elapsed_ms=elapsed;out->score=elapsed>0.0?(double)iterations/elapsed:0.0;
    return 0;
}
static void add_finding(char *details,size_t cap,unsigned line,const char *description) {
    char item[256]; size_t used,need;
    (void)snprintf(item,sizeof(item),"line %u: %s\n",line,description);
    used=strlen(details);need=strlen(item);
    if(used+need+1u<cap)memcpy(details+used,item,need+1u);
}
int pl_analyze_file(const char *path,pl_analysis *totals,char *details,size_t cap) {
    if(!path||!totals||!details||cap==0u)return -1;
    FILE *f=fopen(path,"rb");
    if(!f){totals->read_errors++;return -1;}
    char line[2048];unsigned number=0;details[0]='\0';totals->files_scanned++;
    while(fgets(line,sizeof(line),f)){
        number++;
        if(strstr(line,"system(")||strstr(line,"popen(")){add_finding(details,cap,number,"process execution API");totals->findings++;}
        if(strstr(line,"strcpy(")||strstr(line,"strcat(")||strstr(line,"sprintf(")){add_finding(details,cap,number,"unbounded string API");totals->findings++;}
        if(strstr(line,"VirtualAlloc(")||strstr(line,"mmap(")){add_finding(details,cap,number,"manual memory mapping; review bounds and permissions");totals->findings++;}
        if(strstr(line,"__asm")||strstr(line,"asm volatile")){add_finding(details,cap,number,"inline assembly; architecture-specific review required");totals->findings++;}
    }
    if(ferror(f)){(void)fclose(f);totals->read_errors++;return -1;}
    (void)fclose(f);return 0;
}
static void json_string(FILE *f,const char *s) {
    const unsigned char *p=(const unsigned char*)s;
    (void)fputc('"',f);
    while(*p){if(*p=='"'||*p=='\\'){(void)fputc('\\',f);(void)fputc(*p,f);}
    else if(*p<32u)(void)fprintf(f,"\\u%04x",(unsigned)*p);else (void)fputc(*p,f);p++;}
    (void)fputc('"',f);
}
int pl_write_json(const char *path,const pl_cpu_info *cpu,const pl_benchmark *b,const pl_analysis *a) {
    if(!path||!cpu||!b||!a)return -1;
    FILE *f=fopen(path,"wb");if(!f)return -1;
    (void)fputs("{\n  \"version\": \"0.4.0\",\n  \"cpu\": {\"vendor\":",f);json_string(f,cpu->vendor);
    (void)fputs(",\"model\":",f);json_string(f,cpu->model);
    (void)fprintf(f,",\"logical_cpus\":%u,\"family\":%u,\"model_id\":%u},\n",cpu->logical_cpus,cpu->family,cpu->model_id);
    (void)fprintf(f,"  \"benchmark\": {\"iterations\":%llu,\"elapsed_ms\":%.3f,\"iterations_per_ms\":%.3f},\n",(unsigned long long)b->iterations,b->elapsed_ms,b->score);
    (void)fprintf(f,"  \"analysis\": {\"files_scanned\":%u,\"findings\":%u,\"read_errors\":%u}\n}\n",a->files_scanned,a->findings,a->read_errors);
    int bad=ferror(f);if(fclose(f)!=0)bad=1;return bad?-1:0;
}
