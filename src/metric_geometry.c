#include "metric_geometry.h"
#include <float.h>
#include <math.h>
#include <stdint.h>

static int valid_vector(const double *values,size_t count) {
    if(values==NULL||count==0u||count>PL_METRIC_MAX_DIMENSION)return 0;
    for(size_t i=0u;i<count;i++)if(!isfinite(values[i]))return 0;
    return 1;
}
static int store_finite(long double value,double *out) {
    if(out==NULL||!isfinite(value)||fabsl(value)>(long double)DBL_MAX)return -1;
    *out=(double)value;return isfinite(*out)?0:-1;
}
int pl_metric_dot(const double *a,const double *b,size_t n,double *out) {
    if(!valid_vector(a,n)||!valid_vector(b,n)||out==NULL)return -1;
    long double total=0.0L;
    for(size_t i=0u;i<n;i++)total+=(long double)a[i]*(long double)b[i];
    return store_finite(total,out);
}
int pl_metric_norm(const double *v,size_t n,double *out) {
    if(!valid_vector(v,n)||out==NULL)return -1;
    double scale=0.0,ssq=1.0;
    for(size_t i=0u;i<n;i++){double x=fabs(v[i]);if(x==0.0)continue;if(scale<x){double r=scale/x;ssq=1.0+ssq*r*r;scale=x;}else{double r=x/scale;ssq+=r*r;}}
    if(scale==0.0){*out=0.0;return 0;}*out=scale*sqrt(ssq);return isfinite(*out)?0:-1;
}
int pl_metric_distance(const double *a,const double *b,size_t n,double *out) {
    if(!valid_vector(a,n)||!valid_vector(b,n)||out==NULL)return -1;
    double scale=0.0,ssq=1.0;
    for(size_t i=0u;i<n;i++){double d=a[i]-b[i];if(!isfinite(d))return -1;double x=fabs(d);if(x==0.0)continue;if(scale<x){double r=scale/x;ssq=1.0+ssq*r*r;scale=x;}else{double r=x/scale;ssq+=r*r;}}
    *out=scale==0.0?0.0:scale*sqrt(ssq);return isfinite(*out)?0:-1;
}
int pl_metric_is_orthogonal(const double *a,const double *b,size_t n,double tol,int *out) {
    if(!valid_vector(a,n)||!valid_vector(b,n)||out==NULL||!isfinite(tol)||tol<0.0||tol>1.0)return -1;
    double ma=0.0,mb=0.0;
    for(size_t i=0u;i<n;i++){if(fabs(a[i])>ma)ma=fabs(a[i]);if(fabs(b[i])>mb)mb=fabs(b[i]);}
    if(ma==0.0||mb==0.0){*out=1;return 0;}
    long double dot=0.0L,aa=0.0L,bb=0.0L;
    for(size_t i=0u;i<n;i++){long double x=(long double)a[i]/ma,y=(long double)b[i]/mb;dot+=x*y;aa+=x*x;bb+=y*y;}
    long double denominator=sqrtl(aa*bb);if(denominator==0.0L){*out=1;return 0;}
    *out=fabsl(dot/denominator)<=(long double)tol;return 0;
}
int pl_metric_quadratic_form(const double *v,const double *m,size_t n,double *out) {
    if(!valid_vector(v,n)||m==NULL||out==NULL||n>PL_METRIC_MAX_MATRIX_DIMENSION||n>SIZE_MAX/n)return -1;
    long double total=0.0L;
    for(size_t i=0u;i<n;i++)for(size_t j=0u;j<n;j++){double a=m[i*n+j];if(!isfinite(a))return -1;total+=(long double)v[i]*(long double)a*(long double)v[j];}
    return store_finite(total,out);
}
int pl_metric_law_of_cosines(double a,double b,double angle,double *out) {
    if(out==NULL||!isfinite(a)||!isfinite(b)||!isfinite(angle)||a<0.0||b<0.0)return -1;
    *out=hypot(a-b*cos(angle),b*sin(angle));return isfinite(*out)?0:-1;
}
int pl_metric_de_gua_area(double a,double b,double c,double *out) {
    if(out==NULL||!isfinite(a)||!isfinite(b)||!isfinite(c)||a<0.0||b<0.0||c<0.0)return -1;
    *out=hypot(hypot(a,b),c);return isfinite(*out)?0:-1;
}
int pl_metric_parseval_energy(const double *v,size_t n,double *out) {
    if(!valid_vector(v,n)||out==NULL)return -1;
    long double energy=0.0L;for(size_t i=0u;i<n;i++){long double x=v[i];energy+=x*x;}
    return store_finite(energy,out);
}
int pl_metric_unit_circle_residual(double angle,double *out) {
    if(out==NULL||!isfinite(angle))return -1;double s=sin(angle),c=cos(angle);
    *out=fabs(fma(s,s,fma(c,c,-1.0)));return isfinite(*out)?0:-1;
}
int pl_metric_spherical_hypotenuse(double a,double b,double *out) {
    if(out==NULL||!isfinite(a)||!isfinite(b)||a<0.0||b<0.0||a>1.57079632679489661923||b>1.57079632679489661923)return -1;
    double p=cos(a)*cos(b);if(p>1.0)p=1.0;if(p< -1.0)p=-1.0;*out=acos(p);return isfinite(*out)?0:-1;
}
int pl_metric_hyperbolic_hypotenuse(double a,double b,double *out) {
    if(out==NULL||!isfinite(a)||!isfinite(b)||a<0.0||b<0.0)return -1;
    double ca=cosh(a),cb=cosh(b);if(!isfinite(ca)||!isfinite(cb)||(ca!=0.0&&cb>DBL_MAX/ca))return -1;
    *out=acosh(ca*cb);return isfinite(*out)?0:-1;
}
int pl_metric_minkowski_interval(double ct,const double *spatial,size_t n,double *out) {
    if(!isfinite(ct)||!valid_vector(spatial,n)||out==NULL)return -1;
    long double result=(long double)ct*(long double)ct;
    for(size_t i=0u;i<n;i++)result-=(long double)spatial[i]*(long double)spatial[i];
    return store_finite(result,out);
}
int pl_metric_combine_uncorrelated_variances(const double *v,size_t n,double *out) {
    if(!valid_vector(v,n)||out==NULL)return -1;long double total=0.0L;
    for(size_t i=0u;i<n;i++){if(v[i]<0.0)return -1;total+=(long double)v[i];}
    return store_finite(total,out);
}
int pl_metric_reciprocal_altitude(double a,double b,double *out) {
    if(out==NULL||!isfinite(a)||!isfinite(b)||a<=0.0||b<=0.0)return -1;
    double small=a<b?a:b,large=a<b?b:a;*out=small/hypot(1.0,small/large);
    return isfinite(*out)&&*out>0.0?0:-1;
}
