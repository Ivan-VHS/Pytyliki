#ifndef PYTYLIKI_METRIC_GEOMETRY_H
#define PYTYLIKI_METRIC_GEOMETRY_H
#include <stddef.h>
#define PL_METRIC_MAX_DIMENSION 4096u
#define PL_METRIC_MAX_MATRIX_DIMENSION 512u
/* Return 0 on success, -1 for invalid, incompatible or non-finite input. */
int pl_metric_dot(const double *left,const double *right,size_t count,double *out);
int pl_metric_norm(const double *vector,size_t count,double *out);
int pl_metric_distance(const double *left,const double *right,size_t count,double *out);
int pl_metric_is_orthogonal(const double *left,const double *right,size_t count,double tolerance,int *out);
int pl_metric_quadratic_form(const double *vector,const double *matrix,size_t dimension,double *out);
int pl_metric_law_of_cosines(double side_a,double side_b,double included_angle,double *opposite_side);
int pl_metric_de_gua_area(double face_a,double face_b,double face_c,double *opposite_face_area);
int pl_metric_parseval_energy(const double *coefficients,size_t count,double *out);
int pl_metric_unit_circle_residual(double angle,double *out);
int pl_metric_spherical_hypotenuse(double side_a,double side_b,double *out);
int pl_metric_hyperbolic_hypotenuse(double side_a,double side_b,double *out);
int pl_metric_minkowski_interval(double ct,const double *spatial,size_t spatial_dimension,double *out);
int pl_metric_combine_uncorrelated_variances(const double *variances,size_t count,double *out);
int pl_metric_reciprocal_altitude(double side_a,double side_b,double *out);
/* General-purpose vector metrics. Weights must be finite and non-negative. */
int pl_metric_l1_norm(const double *vector,size_t count,double *out);
int pl_metric_linf_norm(const double *vector,size_t count,double *out);
int pl_metric_weighted_l2_norm(const double *vector,const double *weights,size_t count,double *out);
int pl_metric_weighted_dot(const double *left,const double *right,const double *weights,size_t count,double *out);
int pl_metric_squared_distance(const double *left,const double *right,size_t count,double *out);
int pl_metric_l1_distance(const double *left,const double *right,size_t count,double *out);
int pl_metric_linf_distance(const double *left,const double *right,size_t count,double *out);
int pl_metric_weighted_distance(const double *left,const double *right,const double *weights,size_t count,double *out);
int pl_metric_project(const double *vector,const double *onto,size_t count,double *out);
int pl_metric_orthogonal_component(const double *vector,const double *onto,size_t count,double *out);
int pl_metric_cosine_similarity(const double *left,const double *right,size_t count,double *out);
/* sample != 0 selects sample variance/covariance (n - 1 denominator). */
int pl_metric_mean_variance(const double *values,size_t count,int sample,double *mean,double *variance);
int pl_metric_rms(const double *values,size_t count,double *out);
int pl_metric_covariance(const double *left,const double *right,size_t count,int sample,double *out);
int pl_metric_pythagorean_check(const double *left,const double *right,size_t count,double absolute_tolerance,double relative_tolerance,int *out);
int pl_metric_triangle_check(const double *a,const double *b,const double *c,size_t count,double absolute_tolerance,double relative_tolerance,int *out);
#endif
