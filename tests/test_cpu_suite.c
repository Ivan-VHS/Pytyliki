#include "cpu_suite.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
 const pl_cpu_suite_module *items=NULL; size_t count=pl_cpu_suite_catalog(&items);
 assert(items!=NULL&&count==PL_CPU_SUITE_MODULE_COUNT&&count==78u);
 assert(pl_cpu_suite_catalog(NULL)==0u);
 unsigned domains_seen=0u;
 for(size_t i=0u;i<count;++i){
  pl_cpu_suite_module a,b,c;pl_cpu_suite_plan p;
  assert(items[i].id==(uint16_t)(i+1u));assert(items[i].name&&items[i].name[0]);assert(items[i].description&&items[i].description[0]);
  assert((unsigned)items[i].domain<6u);domains_seen|=1u<<(unsigned)items[i].domain;
  assert(pl_cpu_suite_get(i,&a)==0);assert(pl_cpu_suite_find_id(items[i].id,&b)==0);assert(pl_cpu_suite_find_name(items[i].name,&c)==0);
  assert(strcmp(a.name,b.name)==0&&strcmp(a.name,c.name)==0);assert(pl_cpu_suite_plan_module(items[i].id,&p)==0);
  assert(strcmp(p.execution_mode,"ADVISORY")==0);assert(strstr(p.safety_note,"not performed")!=NULL);
  for(size_t j=i+1u;j<count;++j){assert(items[i].id!=items[j].id);assert(strcmp(items[i].name,items[j].name)!=0);}
 }
 assert(domains_seen==((1u<<6u)-1u));
 {pl_cpu_suite_module m;pl_cpu_suite_plan p;assert(pl_cpu_suite_get(count,&m)!=0);assert(pl_cpu_suite_find_id(0u,&m)!=0);assert(pl_cpu_suite_find_id(79u,&m)!=0);assert(pl_cpu_suite_find_name(NULL,&m)!=0);assert(pl_cpu_suite_find_name("",&m)!=0);assert(pl_cpu_suite_plan_module(79u,&p)!=0);assert(pl_cpu_suite_domain_name((pl_cpu_suite_domain)99)!=NULL);}
 puts("CPU Suite Pro 78 catalogue tests passed.");return 0;
}
