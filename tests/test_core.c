#include "core.h"
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "CHECK failed at %s:%d: %s\\n", __FILE__, __LINE__, #expr); \
    return 1; \
} } while (0)

int main(void) {
    pl_cpu_info cpu;
    pl_benchmark b;
    pl_analysis a={0u,0u,0u};
    char details[1024];
    const char *src="pytyliki_test_input.c", *json="pytyliki_test_report.json";
    FILE *f;

    CHECK(pl_cpu_profile(&cpu)==0);
    CHECK(cpu.logical_cpus>=1u);
    CHECK(cpu.vendor[0]!='\\0');
    CHECK(cpu.model[0]!='\\0');

    CHECK(pl_run_benchmark(10000u,&b)==0);
    CHECK(b.iterations==10000u);
    CHECK(b.elapsed_ms>=0.0);
    CHECK(pl_run_benchmark(0u,&b)!=0);
    CHECK(pl_run_benchmark(UINT64_C(500000001),&b)!=0);

    f=fopen(src,"wb");
    CHECK(f!=NULL);
    CHECK(fputs("int main(void) { strcpy(dst, src); system(cmd); }\\n",f)>=0);
    CHECK(fclose(f)==0);

    CHECK(pl_analyze_file(src,&a,details,sizeof(details))==0);
    CHECK(a.files_scanned==1u);
    CHECK(a.findings==2u);
    CHECK(strstr(details,"process execution API")!=NULL);
    CHECK(strstr(details,"unbounded string API")!=NULL);
    (void)remove(src);

    a.files_scanned=0u; a.findings=0u; a.read_errors=0u;
    b.iterations=0u; b.elapsed_ms=0.0; b.score=0.0;
    CHECK(pl_write_json(json,&cpu,&b,&a)==0);
    f=fopen(json,"rb");
    CHECK(f!=NULL);
    CHECK(fgetc(f)=='{');
    CHECK(fclose(f)==0);
    (void)remove(json);

    puts("All core tests passed.");
    return 0;
}
