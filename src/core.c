#define _POSIX_C_SOURCE 200809L
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
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
static void copy_text(char *dst,size_t cap,const char *src){if(cap)snprintf(dst,cap,"%s",src?src:"unknown");}
int pl_cpu_profile(pl_cpu_info *out){
 if(!out)return -1;memset(out,0,sizeof(*out));
#if defined(_WIN32)
 SYSTEM_INFO si;GetSystemInfo(&si);out->logical_cpus=si.dwNumberOfProcessors;
#elif defined(_SC_NPROCESSORS_ONLN)
 long n=sysconf(_SC_NPROCESSORS_ONLN);out->logical_cpus=n>0?(unsigned)n:1u;
#else
 out->logical_cpus=1;
#endif
#if defined(_MSC_VER) && (defined(_M_X64)||defined(_M_IX86))
 int r[4];__cpuid(r,0);char v[13];memcpy(v,&r[1],4);memcpy(v+4,&r[3],4);memcpy(v+8,&r[2],4);v[12]=0;copy_text(out->vendor,sizeof(out->vendor),v);__cpuid(r,1);out->family=(unsigned)((r[0]>>8)&15);out->model_id=(unsigned)((r[0]>>4)&15);
#elif (defined(__x86_64__)||defined(__i386__))&&defined(__GNUC__)
 unsigned a,b,c,d;if(__get_cpuid(0,&a,&b,&c,&d)){char v[13];memcpy(v,&b,4);memcpy(v+4,&d,4);memcpy(v+8,&c,4);v[12]=0;copy_text(out->vendor,sizeof(out->vendor),v);}if(__get_cpuid(1,&a,&b,&c,&d)){out->family=(a>>8)&15;out->model_id=(a>>4)&15;}
#else
 copy_text(out->vendor,sizeof(out->vendor),"not-available");
#endif
 if(!out->vendor[0])copy_text(out->vendor,sizeof(out->vendor),"not-available");
 copy_text(out->model,sizeof(out->model),"CPU model string unavailable on this build");return 0;
}
static double now_ms(void){
#if defined(_WIN32)
 LARGE_INTEGER f,t;QueryPerformanceFrequency(&f);QueryPerformanceCounter(&t);return 1000.0*(double)t.QuadPart/(double)f.QuadPart;
#else
 struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t)!=0)return (double)clock()/((double)CLOCKS_PER_SEC/1000.0);return 1000.0*(double)t.tv_sec+(double)t.tv_nsec/1000000.0;
#endif
}
int pl_run_benchmark(uint64_t iterations,pl_benchmark *out){
 if(!out||iterations==0||iterations>UINT64_C(500000000))return -1;volatile uint64_t x=UINT64_C(0x9e3779b97f4a7c15);double start=now_ms();for(uint64_t i=0;i<iterations;i++){x^=x>>12;x^=x<<25;x^=x>>27;x*=UINT64_C(2685821657736338717);}double elapsed=now_ms()-start;if(x==0)fputc(' ',stdout);out->iterations=iterations;out->elapsed_ms=elapsed;out->score=elapsed>0?(double)iterations/elapsed:0.0;return 0;
}
static int contains(const char*s,const char*n){return strstr(s,n)!=NULL;}
static void append_finding(
    pl_analysis *totals, char *details, size_t cap,
    unsigned line_no, const char *finding
) {
    char item[256];
    int written = snprintf(item, sizeof(item), "line %u: %s\n", line_no, finding);
    if (written > 0) {
        size_t used = strlen(details);
        size_t need = (size_t)written;
        if (need < sizeof(item) && used + need + 1u <= cap) {
            memcpy(details + used, item, need + 1u);
        }
        totals->findings++;
    }
}

int pl_analyze_file(const char *path, pl_analysis *totals, char *details, size_t cap) {
    FILE *f;
    char line[2048];
    unsigned line_no = 0u;

    if (path == NULL || totals == NULL || details == NULL || cap == 0u) return -1;
    f = fopen(path, "rb");
    if (f == NULL) {
        totals->read_errors++;
        return -1;
    }

    details[0] = '\0';
    totals->files_scanned++;
    while (fgets(line, sizeof(line), f) != NULL) {
        line_no++;
        /* Evaluate each category independently: one source line may contain
           multiple distinct risky patterns and must not hide later matches. */
        if (contains(line, "system(") || contains(line, "popen("))
            append_finding(totals, details, cap, line_no, "process execution API");
        if (contains(line, "strcpy(") || contains(line, "strcat(") || contains(line, "sprintf("))
            append_finding(totals, details, cap, line_no, "unbounded string API");
        if (contains(line, "VirtualAlloc(") || contains(line, "mmap("))
            append_finding(totals, details, cap, line_no,
                           "manual memory mapping; review bounds and permissions");
        if (contains(line, "__asm") || contains(line, "asm volatile"))
            append_finding(totals, details, cap, line_no,
                           "inline assembly; architecture-specific review required");
    }

    if (ferror(f)) {
        fclose(f);
        totals->read_errors++;
        return -1;
    }
    fclose(f);
    return 0;
}
static void json_string(FILE*f,const char*s){fputc('"',f);for(;*s;s++){unsigned char c=(unsigned char)*s;if(c=='"'||c=='\\'){fputc('\\',f);fputc(c,f);}else if(c<32)fprintf(f,"\\u%04x",c);else fputc(c,f);}fputc('"',f);}
int pl_write_json(const char*path,const pl_cpu_info*cpu,const pl_benchmark*b,const pl_analysis*a){
 if(!path||!cpu||!b||!a)return -1;FILE*f=fopen(path,"wb");if(!f)return -1;fputs("{\n  \"version\": \"0.4.0\",\n  \"cpu\": {\"vendor\":",f);json_string(f,cpu->vendor);fputs(",\"model\":",f);json_string(f,cpu->model);fprintf(f,",\"logical_cpus\":%u,\"family\":%u,\"model_id\":%u},\n",cpu->logical_cpus,cpu->family,cpu->model_id);fprintf(f,"  \"benchmark\": {\"iterations\":%llu,\"elapsed_ms\":%.3f,\"iterations_per_ms\":%.3f},\n",(unsigned long long)b->iterations,b->elapsed_ms,b->score);fprintf(f,"  \"analysis\": {\"files_scanned\":%u,\"findings\":%u,\"read_errors\":%u}\n}\n",a->files_scanned,a->findings,a->read_errors);int bad=ferror(f);if(fclose(f)!=0)bad=1;return bad?-1:0;
}
