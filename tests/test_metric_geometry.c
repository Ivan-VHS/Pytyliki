#include "metric_geometry.h"
#include "metric_registry.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
int main(void){
 const double x[]={3,4},zero[]={0,0},orth[]={0,2},coeff[]={1,2,2},vars[]={4,9,3},spatial[]={3,4,0};
 const double matrix[]={2,0,0,3},weights[]={1,4},p[]={1,0},q[]={0,1},r[]={1,1};
 double v=0,mean=0,variance=0,covariance=0,projected[2],remainder[2];int yes=0;
 assert(pl_metric_norm(x,2,&v)==0&&fabs(v-5)<1e-12);
 assert(pl_metric_distance(x,zero,2,&v)==0&&fabs(v-5)<1e-12);
 assert(pl_metric_dot(x,orth,2,&v)==0&&fabs(v-8)<1e-12);
 assert(pl_metric_is_orthogonal((double[]){1,0},orth,2,1e-12,&yes)==0&&yes);
 assert(pl_metric_quadratic_form(x,matrix,2,&v)==0&&fabs(v-66)<1e-12);
 assert(pl_metric_law_of_cosines(3,4,1.5707963267948966,&v)==0&&fabs(v-5)<1e-12);
 assert(pl_metric_de_gua_area(3,4,12,&v)==0&&fabs(v-13)<1e-12);
 assert(pl_metric_parseval_energy(coeff,3,&v)==0&&fabs(v-9)<1e-12);
 assert(pl_metric_unit_circle_residual(0.73,&v)==0&&v<1e-12);
 assert(pl_metric_spherical_hypotenuse(0.2,0.3,&v)==0&&v>0);
 assert(pl_metric_hyperbolic_hypotenuse(0.2,0.3,&v)==0&&v>0);
 assert(pl_metric_minkowski_interval(10,spatial,3,&v)==0&&fabs(v-75)<1e-12);
 assert(pl_metric_combine_uncorrelated_variances(vars,3,&v)==0&&fabs(v-16)<1e-12);
 assert(pl_metric_reciprocal_altitude(3,4,&v)==0&&fabs(v-2.4)<1e-12);
 assert(pl_metric_l1_norm(x,2,&v)==0&&fabs(v-7)<1e-12);
 assert(pl_metric_linf_norm(x,2,&v)==0&&fabs(v-4)<1e-12);
 assert(pl_metric_weighted_l2_norm(x,weights,2,&v)==0&&fabs(v-sqrt(73))<1e-12);
 assert(pl_metric_weighted_dot(x,x,weights,2,&v)==0&&fabs(v-73)<1e-12);
 assert(pl_metric_squared_distance(x,zero,2,&v)==0&&fabs(v-25)<1e-12);
 assert(pl_metric_l1_distance(x,zero,2,&v)==0&&fabs(v-7)<1e-12);
 assert(pl_metric_linf_distance(x,zero,2,&v)==0&&fabs(v-4)<1e-12);
 assert(pl_metric_weighted_distance(x,zero,weights,2,&v)==0&&fabs(v-sqrt(73))<1e-12);
 assert(pl_metric_project(x,(double[]){1,0},2,projected)==0&&projected[0]==3&&projected[1]==0);
 assert(pl_metric_orthogonal_component(x,(double[]){1,0},2,remainder)==0&&remainder[0]==0&&remainder[1]==4);
 assert(pl_metric_cosine_similarity((double[]){1,0},(double[]){1,1},2,&v)==0&&fabs(v-sqrt(0.5))<1e-12);
 assert(pl_metric_mean_variance((double[]){1,2,3},3,0,&mean,&variance)==0&&mean==2&&fabs(variance-2.0/3.0)<1e-12);
 assert(pl_metric_mean_variance((double[]){1,2,3},3,1,&mean,&variance)==0&&fabs(variance-1)<1e-12);
 assert(pl_metric_rms((double[]){3,4},2,&v)==0&&fabs(v-sqrt(12.5))<1e-12);
 assert(pl_metric_covariance((double[]){1,2,3},(double[]){2,4,6},3,1,&covariance)==0&&fabs(covariance-2)<1e-12);
 assert(pl_metric_pythagorean_check(p,q,2,1e-12,1e-12,&yes)==0&&yes);
 assert(pl_metric_pythagorean_check(p,r,2,1e-12,1e-12,&yes)==0&&!yes);
 assert(pl_metric_triangle_check(p,q,r,2,1e-12,1e-12,&yes)==0&&yes);
 assert(pl_metric_weighted_l2_norm(x,(double[]){1,-1},2,&v)!=0);
 assert(pl_metric_cosine_similarity(zero,x,2,&v)!=0);
 assert(pl_metric_registry_count()>=15u&&pl_metric_registry_find_id(1001u)!=NULL);
 assert(pl_metric_registry_find_id(9999u)==NULL);
 const pl_metric_operation_t *found[8];
 assert(pl_metric_registry_search("variance",NULL,PL_METRIC_CAP_DETERMINISTIC,found,8u)>=1u);
 assert(pl_metric_registry_search(NULL,"memory",0u,found,8u)>=3u);
 assert(pl_metric_norm(NULL,1,&v)!=0&&pl_metric_reciprocal_altitude(0,2,&v)!=0);
 assert(pl_metric_combine_uncorrelated_variances((double[]){1,-1},2,&v)!=0);
 assert(pl_metric_spherical_hypotenuse(2,0.1,&v)!=0);
 return 0;
}
