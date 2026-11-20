#include "metric_geometry.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

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

static int valid_weights(const double *weights,size_t count) {
 if(!valid_vector(weights,count))return 0;
 for(size_t i=0u;i<count;i++)if(weights[i]<0.0)return 0;
 return 1;
}
int pl_metric_l1_norm(const double *v,size_t n,double *out) {
 if(!valid_vector(v,n)||out==NULL)return -1;
 long double total=0.0L;
 for(size_t i=0u;i<n;i++)total+=fabsl((long double)v[i]);
 return store_finite(total,out);
}
int pl_metric_linf_norm(const double *v,size_t n,double *out) {
 if(!valid_vector(v,n)||out==NULL)return -1;
 double maximum=0.0;
 for(size_t i=0u;i<n;i++)if(fabs(v[i])>maximum)maximum=fabs(v[i]);
 *out=maximum;return 0;
}
int pl_metric_weighted_l2_norm(const double *v,const double *w,size_t n,double *out) {
 if(!valid_vector(v,n)||!valid_weights(w,n)||out==NULL)return -1;
 long double scale=0.0L,ssq=1.0L;
 for(size_t i=0u;i<n;i++){
  long double term=fabsl((long double)v[i])*sqrtl((long double)w[i]);
  if(term==0.0L)continue;
  if(scale<term){long double ratio=scale/term;ssq=1.0L+ssq*ratio*ratio;scale=term;}
  else{long double ratio=term/scale;ssq+=ratio*ratio;}
 }
 return store_finite(scale==0.0L?0.0L:scale*sqrtl(ssq),out);
}
int pl_metric_weighted_dot(const double *a,const double *b,const double *w,size_t n,double *out) {
 if(!valid_vector(a,n)||!valid_vector(b,n)||!valid_weights(w,n)||out==NULL)return -1;
 long double total=0.0L;
 for(size_t i=0u;i<n;i++)total+=(long double)w[i]*(long double)a[i]*(long double)b[i];
 return store_finite(total,out);
}
int pl_metric_squared_distance(const double *a,const double *b,size_t n,double *out) {
 if(!valid_vector(a,n)||!valid_vector(b,n)||out==NULL)return -1;
 long double total=0.0L;
 for(size_t i=0u;i<n;i++){long double d=(long double)a[i]-(long double)b[i];total+=d*d;}
 return store_finite(total,out);
}
int pl_metric_l1_distance(const double *a,const double *b,size_t n,double *out) {
 if(!valid_vector(a,n)||!valid_vector(b,n)||out==NULL)return -1;
 long double total=0.0L;
 for(size_t i=0u;i<n;i++)total+=fabsl((long double)a[i]-(long double)b[i]);
 return store_finite(total,out);
}
int pl_metric_linf_distance(const double *a,const double *b,size_t n,double *out) {
 if(!valid_vector(a,n)||!valid_vector(b,n)||out==NULL)return -1;
 long double maximum=0.0L;
 for(size_t i=0u;i<n;i++){long double d=fabsl((long double)a[i]-(long double)b[i]);if(d>maximum)maximum=d;}
 return store_finite(maximum,out);
}
int pl_metric_weighted_distance(const double *a,const double *b,const double *w,size_t n,double *out) {
 if(!valid_vector(a,n)||!valid_vector(b,n)||!valid_weights(w,n)||out==NULL)return -1;
 long double scale=0.0L,ssq=1.0L;
 for(size_t i=0u;i<n;i++){
  long double d=fabsl((long double)a[i]-(long double)b[i]);
  long double term=d*sqrtl((long double)w[i]);if(term==0.0L)continue;
  if(scale<term){long double ratio=scale/term;ssq=1.0L+ssq*ratio*ratio;scale=term;}
  else{long double ratio=term/scale;ssq+=ratio*ratio;}
 }
 return store_finite(scale==0.0L?0.0L:scale*sqrtl(ssq),out);
}
int pl_metric_project(const double *v,const double *onto,size_t n,double *out) {
 if(!valid_vector(v,n)||!valid_vector(onto,n)||out==NULL)return -1;
 long double scale=0.0L,denominator=0.0L,numerator=0.0L;
 for(size_t i=0u;i<n;i++)if(fabsl((long double)onto[i])>scale)scale=fabsl((long double)onto[i]);
 if(scale==0.0L)return -1;
 for(size_t i=0u;i<n;i++){long double b=(long double)onto[i]/scale;denominator+=b*b;numerator+=(long double)v[i]*b;}
 if(denominator==0.0L)return -1;
 long double beta=numerator/denominator;
 for(size_t i=0u;i<n;i++){long double value=beta*((long double)onto[i]/scale);if(!isfinite(value)||fabsl(value)>(long double)DBL_MAX)return -1;out[i]=(double)value;}
 return 0;
}
int pl_metric_orthogonal_component(const double *v,const double *onto,size_t n,double *out) {
 if(!valid_vector(v,n)||!valid_vector(onto,n)||out==NULL)return -1;
 long double scale=0.0L,denominator=0.0L,numerator=0.0L;
 for(size_t i=0u;i<n;i++)if(fabsl((long double)onto[i])>scale)scale=fabsl((long double)onto[i]);
 if(scale==0.0L)return -1;
 for(size_t i=0u;i<n;i++){long double b=(long double)onto[i]/scale;denominator+=b*b;numerator+=(long double)v[i]*b;}
 if(denominator==0.0L)return -1;
 long double beta=numerator/denominator;
 for(size_t i=0u;i<n;i++){
  long double projection=beta*((long double)onto[i]/scale);
  long double value=(long double)v[i]-projection;
  if(!isfinite(value)||fabsl(value)>(long double)DBL_MAX)return -1;
  out[i]=(double)value;
 }
 return 0;
}
int pl_metric_cosine_similarity(const double *a,const double *b,size_t n,double *out) {
 if(!valid_vector(a,n)||!valid_vector(b,n)||out==NULL)return -1;
 double sa=0.0,sb=0.0;
 for(size_t i=0u;i<n;i++){if(fabs(a[i])>sa)sa=fabs(a[i]);if(fabs(b[i])>sb)sb=fabs(b[i]);}
 if(sa==0.0||sb==0.0)return -1;
 long double dot=0.0L,aa=0.0L,bb=0.0L;
 for(size_t i=0u;i<n;i++){long double x=(long double)a[i]/sa,y=(long double)b[i]/sb;dot+=x*y;aa+=x*x;bb+=y*y;}
 long double result=dot/sqrtl(aa*bb);if(result>1.0L)result=1.0L;if(result< -1.0L)result=-1.0L;
 return store_finite(result,out);
}
int pl_metric_mean_variance(const double *v,size_t n,int sample,double *mean,double *variance) {
 if(!valid_vector(v,n)||mean==NULL||variance==NULL||(sample!=0&&sample!=1)||(sample&&n<2u))return -1;
 long double m=0.0L,m2=0.0L;
 for(size_t i=0u;i<n;i++){long double x=v[i],delta=x-m;m+=delta/(long double)(i+1u);m2+=delta*(x-m);}
 long double divisor=(long double)(sample?n-1u:n);
 if(store_finite(m,mean)!=0||store_finite(m2/divisor,variance)!=0)return -1;
 if(*variance<0.0&&*variance>-DBL_EPSILON)*variance=0.0;
 return *variance>=0.0?0:-1;
}
int pl_metric_rms(const double *v,size_t n,double *out) {
 if(!valid_vector(v,n)||out==NULL)return -1;
 double scale=0.0,ssq=1.0;
 for(size_t i=0u;i<n;i++){double x=fabs(v[i]);if(x==0.0)continue;
  if(scale<x){double ratio=scale/x;ssq=1.0+ssq*ratio*ratio;scale=x;}
  else{double ratio=x/scale;ssq+=ratio*ratio;}}
 return store_finite(scale==0.0?0.0L:(long double)scale*sqrtl((long double)ssq/(long double)n),out);
}
int pl_metric_covariance(const double *a,const double *b,size_t n,int sample,double *out) {
 if(!valid_vector(a,n)||!valid_vector(b,n)||out==NULL||(sample!=0&&sample!=1)||(sample&&n<2u))return -1;
 long double ma=0.0L,mb=0.0L,c=0.0L;
 for(size_t i=0u;i<n;i++){long double x=a[i],y=b[i],dx=x-ma;ma+=dx/(long double)(i+1u);mb+=(y-mb)/(long double)(i+1u);c+=dx*(y-mb);}
 return store_finite(c/(long double)(sample?n-1u:n),out);
}
static int valid_tolerances(double at,double rt){return isfinite(at)&&isfinite(rt)&&at>=0.0&&rt>=0.0;}
int pl_metric_pythagorean_check(const double *a,const double *b,size_t n,double at,double rt,int *out) {
 if(!valid_vector(a,n)||!valid_vector(b,n)||out==NULL||!valid_tolerances(at,rt))return -1;
 long double aa=0.0L,bb=0.0L,sum=0.0L;
 for(size_t i=0u;i<n;i++){long double x=a[i],y=b[i],z=x+y;aa+=x*x;bb+=y*y;sum+=z*z;}
 long double expected=aa+bb;*out=fabsl(sum-expected)<=(long double)at+(long double)rt*fabsl(expected);return 0;
}
int pl_metric_triangle_check(const double *a,const double *b,const double *c,size_t n,double at,double rt,int *out) {
 if(!valid_vector(a,n)||!valid_vector(b,n)||!valid_vector(c,n)||out==NULL||!valid_tolerances(at,rt))return -1;
 long double ab=0.0L,bc=0.0L,ac=0.0L;
 for(size_t i=0u;i<n;i++){long double x=(long double)a[i]-b[i],y=(long double)b[i]-c[i],z=(long double)a[i]-c[i];ab+=x*x;bc+=y*y;ac+=z*z;}
 long double lhs=sqrtl(ac),rhs=sqrtl(ab)+sqrtl(bc);
 *out=lhs<=rhs+(long double)at+(long double)rt*rhs;return 0;
}
