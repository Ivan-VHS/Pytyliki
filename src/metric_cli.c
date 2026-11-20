#include "metric_geometry.h"
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv) {
    const double a[]={3.0,4.0},b[]={0.0,0.0},orthogonal[]={0.0,1.0};
    const double spectrum[]={0.5,0.25,0.125};
    const double spatial[]={3.0,4.0,0.0},variances[]={0.04,0.09,0.01};
    double value=0.0;int is_orthogonal=0;
    if(argc!=2||strcmp(argv[1],"--demo")!=0) {
        puts("Pytyliki Metric Lab\nUsage: pytyliki-metrics --demo");
        return argc==2&&strcmp(argv[1],"--help")==0?0:2;
    }
    if(pl_metric_norm(a,2u,&value)!=0)return 1;
    printf("2D/ND Euclidean norm: %.9g\n",value);
    if(pl_metric_distance(a,b,2u,&value)!=0)return 1;
    printf("Euclidean distance from origin: %.9g\n",value);
    if(pl_metric_is_orthogonal((double[]){1.0,0.0},orthogonal,2u,1e-12,&is_orthogonal)!=0)return 1;
    printf("Orthogonality check: %s\n",is_orthogonal?"orthogonal":"not orthogonal");
    if(pl_metric_parseval_energy(spectrum,3u,&value)!=0)return 1;
    printf("Parseval coefficient energy: %.9g\n",value);
    if(pl_metric_minkowski_interval(10.0,spatial,3u,&value)!=0)return 1;
    printf("Minkowski interval (+---): %.9g\n",value);
    if(pl_metric_combine_uncorrelated_variances(variances,3u,&value)!=0)return 1;
    printf("Combined uncorrelated variance: %.9g\n",value);
    puts("Metric Lab is a local numerical tool; no hardware settings are changed.");
    return 0;
}
