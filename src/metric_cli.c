#include "metric_geometry.h"
#include "metric_registry.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int print_list(const char *query,const char *category){
 const pl_metric_operation_t *matches[64];size_t total=pl_metric_registry_search(query,category,0u,matches,64u);
 size_t shown=total<64u?total:64u;
 for(size_t i=0u;i<shown;i++)printf("%u  %-30s  [%s]  %s\n",matches[i]->id,matches[i]->name,matches[i]->category,matches[i]->terms);
 printf("Operations: %zu",total);if(total>shown)printf(" (first %zu shown)",shown);putchar('\n');return total==0u?1:0;
}
static int print_description(const char *text){
 char *end=NULL;errno=0;unsigned long id=strtoul(text,&end,10);
 if(errno==0&&end!=text&&*end=='\0'&&id<=0xfffffffful){
  const pl_metric_operation_t *op=pl_metric_registry_find_id((uint32_t)id);if(op==NULL)return 1;
  printf("ID: %u\nName: %s\nCategory: %s\nTerms: %s\nDescription: %s\nComplexity: %s\nType mask: 0x%x\nCapabilities: 0x%x\n",op->id,op->name,op->category,op->terms,op->description,op->complexity,op->type_mask,op->capabilities);
  return 0;
 }return print_list(text,NULL);
}
static int run_demo(void){
 const double a[]={3.0,4.0},b[]={0.0,0.0},orth[]={0.0,1.0},spectrum[]={0.5,0.25,0.125};
 const double spatial[]={3.0,4.0,0.0},variances[]={0.04,0.09,0.01};
 double value=0.0,mean=0.0,variance=0.0;int is_orthogonal=0;
 if(pl_metric_norm(a,2u,&value)!=0)return 1;printf("2D/ND Euclidean norm: %.9g\n",value);
 if(pl_metric_distance(a,b,2u,&value)!=0)return 1;printf("Euclidean distance from origin: %.9g\n",value);
 if(pl_metric_is_orthogonal((double[]){1.0,0.0},orth,2u,1e-12,&is_orthogonal)!=0)return 1;
 printf("Orthogonality check: %s\n",is_orthogonal?"orthogonal":"not orthogonal");
 if(pl_metric_parseval_energy(spectrum,3u,&value)!=0)return 1;printf("Parseval coefficient energy: %.9g\n",value);
 if(pl_metric_minkowski_interval(10.0,spatial,3u,&value)!=0)return 1;printf("Minkowski interval (+---): %.9g\n",value);
 if(pl_metric_combine_uncorrelated_variances(variances,3u,&value)!=0)return 1;printf("Combined uncorrelated variance: %.9g\n",value);
 if(pl_metric_mean_variance(a,2u,0,&mean,&variance)!=0)return 1;printf("Population statistics: mean=%.9g variance=%.9g\n",mean,variance);
 if(pl_metric_l1_norm(a,2u,&value)!=0)return 1;printf("L1 norm: %.9g\n",value);
 puts("Metric Lab is a local numerical tool; no hardware settings are changed.");return 0;
}
int main(int argc,char **argv){
 if(argc==2&&strcmp(argv[1],"--demo")==0)return run_demo();
 if(argc==2&&strcmp(argv[1],"--list")==0)return print_list(NULL,NULL);
 if(argc==3&&strcmp(argv[1],"--search")==0)return print_list(argv[2],NULL);
 if(argc==3&&strcmp(argv[1],"--category")==0)return print_list(NULL,argv[2]);
 if(argc==3&&strcmp(argv[1],"--describe")==0)return print_description(argv[2]);
 if(argc==2&&strcmp(argv[1],"--help")==0){puts("Pytyliki Metric Lab\nUsage: pytyliki-metrics --demo | --list | --search TERM | --category NAME | --describe ID");return 0;}
 puts("Pytyliki Metric Lab\nUsage: pytyliki-metrics --demo | --list | --search TERM | --category NAME | --describe ID");return 2;
}
