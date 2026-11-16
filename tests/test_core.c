#include "core.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
  pl_cpu_info cpu; assert(pl_cpu_profile(&cpu)==0); assert(cpu.logical_cpus>=1); assert(cpu.vendor[0]!='\0');
  pl_benchmark b; assert(pl_run_benchmark(10000,&b)==0); assert(b.iterations==10000); assert(b.elapsed_ms>=0);
  assert(pl_run_benchmark(0,&b)!=0); assert(pl_run_benchmark(UINT64_C(500000001),&b)!=0);
  const char *p="pytyliki_test_input.c"; FILE *f=fopen(p,"wb"); assert(f); fputs("int main(){ strcpy(dst, src); system(cmd); }\n",f); fclose(f);
  pl_analysis a={0}; char details[1024]; assert(pl_analyze_file(p,&a,details,sizeof(details))==0); assert(a.files_scanned==1); assert(a.findings==2); remove(p);
  puts("All core tests passed."); return 0;
}
