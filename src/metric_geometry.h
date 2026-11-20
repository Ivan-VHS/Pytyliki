#ifndef PYTYLIKI_METRIC_GEOMETRY_H
#define PYTYLIKI_METRIC_GEOMETRY_H
#include <stddef.h>
#define PL_METRIC_MAX_DIMENSION 4096u
#define PL_METRIC_MAX_MATRIX_DIMENSION 512u
/* Return 0 on success, -1 for invalid or non-finite input. */
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
#endif
