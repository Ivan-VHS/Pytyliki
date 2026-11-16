#include "core.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    pl_cpu_info cpu; pl_benchmark b; pl_analysis a={0u,0u,0u};
    char details[1024]; const char *src="pytyliki_test_input.c", *json="pytyliki_test_report.json";
    FILE *f;
    assert(pl_cpu_profile(&cpu)==0); assert(cpu.logical_cpus>=1u);
    assert(cpu.vendor[0]!='\\0'); assert(cpu.model[0]!='\\0');
    assert(pl_run_benchmark(10000u,&b)==0); assert(b.iterations==10000u); assert(b.elapsed_ms>=0.0);
    assert(pl_run_benchmark(0u,&b)!=0); assert(pl_run_benchmark(UINT64_C(500000001),&b)!=0);
    f=fopen(src,"wb"); assert(f); assert(fputs("int main(void) { strcpy(dst, src); system(cmd); }\\n",f)>=0); assert(fclose(f)==0);
    assert(pl_analyze_file(src,&a,details,sizeof(details))==0); assert(a.files_scanned==1u);
    assert(a.findings==2u); assert(strstr(details,"process execution API")!=NULL);
    assert(strstr(details,"unbounded string API")!=NULL); (void)remove(src);
    a.files_scanned=0u;a.findings=0u;a.read_errors=0u;b.iterations=0u;b.elapsed_ms=0.0;b.score=0.0;
    assert(pl_write_json(json,&cpu,&b,&a)==0);f=fopen(json,"rb");assert(f);assert(fgetc(f)=='{');assert(fclose(f)==0);(void)remove(json);
    puts("All core tests passed.");return 0;
}
