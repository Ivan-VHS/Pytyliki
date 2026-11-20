#include "metric_geometry.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
int main(void) {
 const double x[]={3.0,4.0},zero[]={0.0,0.0},orth[]={0.0,2.0};
 const double coeff[]={1.0,2.0,2.0},vars[]={4.0,9.0,3.0},spatial[]={3.0,4.0,0.0};
 const double matrix[]={2.0,0.0,0.0,3.0};double v=0.0;int yes=0;
 assert(pl_metric_norm(x,2,&v)==0&&fabs(v-5.0)<1e-12);
 assert(pl_metric_distance(x,zero,2,&v)==0&&fabs(v-5.0)<1e-12);
 assert(pl_metric_dot(x,orth,2,&v)==0&&fabs(v-8.0)<1e-12);
 assert(pl_metric_is_orthogonal((double[]){1.0,0.0},orth,2,1e-12,&yes)==0&&yes);
 assert(pl_metric_quadratic_form(x,matrix,2,&v)==0&&fabs(v-66.0)<1e-12);
 assert(pl_metric_law_of_cosines(3,4,1.5707963267948966,&v)==0&&fabs(v-5.0)<1e-12);
 assert(pl_metric_de_gua_area(3,4,12,&v)==0&&fabs(v-13.0)<1e-12);
 assert(pl_metric_parseval_energy(coeff,3,&v)==0&&fabs(v-9.0)<1e-12);
 assert(pl_metric_unit_circle_residual(0.73,&v)==0&&v<1e-12);
 assert(pl_metric_spherical_hypotenuse(0.2,0.3,&v)==0&&v>0.0);
 assert(pl_metric_hyperbolic_hypotenuse(0.2,0.3,&v)==0&&v>0.0);
 assert(pl_metric_minkowski_interval(10,spatial,3,&v)==0&&fabs(v-75.0)<1e-12);
 assert(pl_metric_combine_uncorrelated_variances(vars,3,&v)==0&&fabs(v-16.0)<1e-12);
 assert(pl_metric_reciprocal_altitude(3,4,&v)==0&&fabs(v-2.4)<1e-12);
 assert(pl_metric_norm(NULL,1,&v)!=0);
 assert(pl_metric_reciprocal_altitude(0,2,&v)!=0);
 assert(pl_metric_combine_uncorrelated_variances((double[]){1.0,-1.0},2,&v)!=0);
 assert(pl_metric_spherical_hypotenuse(2.0,0.1,&v)!=0);
 return 0;
}
