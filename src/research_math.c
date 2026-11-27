#include "research_math.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#define PL_PI 3.14159265358979323846264338327950288
#define PL_MAX_PLAUSIBLE_PRIME 2147483647u

static const char *const research_catalog[] = {
    "riemann.dirichlet_partial", "riemann.eta_partial", "riemann.critical_residual",
    "complexity.cnf_validate", "complexity.sat_verify", "complexity.sat_find_bounded",
    "collatz.step", "collatz.trajectory", "collatz.stopping_time",
    "navier_stokes.grid_divergence", "navier_stokes.grid_vorticity", "navier_stokes.cfl_dt",
    "bsd.curve_nonsingular_mod_prime", "bsd.curve_point_count_mod_prime", "bsd.frobenius_trace_mod_prime",
    "hodge.type_check", "hodge.cycle_span_rank_mod_prime", "hodge.class_in_cycle_span_mod_prime",
    "yang_mills.u1_plaquette_phase", "yang_mills.u1_wilson_action", "yang_mills.correlator_effective_mass",
    "goldbach.is_prime_u64", "goldbach.verify", "goldbach.find_pair_bounded",
    "twin_primes.verify", "twin_primes.next_bounded", "twin_primes.count_bounded",
    "halting.vm_step", "halting.vm_run_bounded", "halting.vm_trace_verify",
    "hadwiger_nelson.unit_graph_build", "hadwiger_nelson.coloring_verify", "hadwiger_nelson.coloring_search_bounded",
    "inscribed_square.point_on_polyline", "inscribed_square.verify", "inscribed_square.search_sampled",
    "legendre.interval", "legendre.witness_verify", "legendre.find_prime_bounded",
    "beal.power_sum_check", "beal.common_prime_divisor", "beal.counterexample_screen",
    "perfect_cuboid.verify", "perfect_cuboid.face_diagonals", "perfect_cuboid.search_bounded",
    "tate.fixed_vector_verify", "tate.invariant_dimension_mod_prime", "tate.cycle_span_rank_mod_prime",
    "matrix.multiply_f64", "matrix.product_verify_f64", "matrix.scaling_exponent"
};

size_t pl_math_research_catalog_count(void) {
    return sizeof(research_catalog) / sizeof(research_catalog[0]);
}

const char *pl_math_research_catalog_name(size_t index) {
    return index < pl_math_research_catalog_count() ? research_catalog[index] : NULL;
}

static int checked_product_size(size_t a, size_t b, size_t *out) {
    if (out == NULL || (a != 0u && b > SIZE_MAX / a)) return 0;
    *out = a * b;
    return 1;
}

static int finite_double(double value) { return isfinite(value) ? 1 : 0; }

static double wrap_phase(double value) {
    value = remainder(value, 2.0 * PL_PI);
    if (value <= -PL_PI) value += 2.0 * PL_PI;
    if (value > PL_PI) value -= 2.0 * PL_PI;
    return value;
}

/* ----------------------------- Riemann ----------------------------- */

static int riemann_series(double sigma, double t, size_t terms, int alternating,
                          pl_math_complex *out) {
    if (out == NULL || !finite_double(sigma) || !finite_double(t) ||
        terms == 0u || terms > PL_MATH_MAX_TERMS || sigma <= 0.0) return PL_MATH_INVALID;
    if (!alternating && sigma <= 1.0) return PL_MATH_INVALID;
    long double real_sum = 0.0L, imag_sum = 0.0L;
    for (size_t n = 1u; n <= terms; ++n) {
        double nd = (double)n;
        double magnitude = pow(nd, -sigma);
        double angle = -t * log(nd);
        double sign = alternating && (n % 2u == 0u) ? -1.0 : 1.0;
        if (!finite_double(magnitude) || !finite_double(angle)) return PL_MATH_OVERFLOW;
        real_sum += (long double)(sign * magnitude * cos(angle));
        imag_sum += (long double)(sign * magnitude * sin(angle));
    }
    if (fabsl(real_sum) > (long double)DBL_MAX || fabsl(imag_sum) > (long double)DBL_MAX)
        return PL_MATH_OVERFLOW;
    out->real = (double)real_sum;
    out->imag = (double)imag_sum;
    return finite_double(out->real) && finite_double(out->imag) ? PL_MATH_OK : PL_MATH_OVERFLOW;
}

int pl_math_riemann_dirichlet(double sigma, double t, size_t terms, pl_math_complex *out) {
    return riemann_series(sigma, t, terms, 0, out);
}

int pl_math_riemann_eta(double sigma, double t, size_t terms, pl_math_complex *out) {
    return riemann_series(sigma, t, terms, 1, out);
}

int pl_math_riemann_critical_residual(double t, size_t terms, double *out_abs_value) {
    pl_math_complex eta;
    int status;
    double sigma = 0.5;
    double exponent = 0.5;
    double angle = -t * log(2.0);
    double factor_real, factor_imag, denominator_real, denominator_imag, denominator_norm;
    if (out_abs_value == NULL || !finite_double(t) || !finite_double(angle)) return PL_MATH_INVALID;
    status = pl_math_riemann_eta(sigma, t, terms, &eta);
    if (status != PL_MATH_OK) return status;
    factor_real = pow(2.0, exponent) * cos(angle);
    factor_imag = pow(2.0, exponent) * sin(angle);
    denominator_real = 1.0 - factor_real;
    denominator_imag = -factor_imag;
    denominator_norm = denominator_real * denominator_real + denominator_imag * denominator_imag;
    if (!finite_double(denominator_norm) || denominator_norm < 1e-24) return PL_MATH_INVALID;
    {
        double zr = (eta.real * denominator_real + eta.imag * denominator_imag) / denominator_norm;
        double zi = (eta.imag * denominator_real - eta.real * denominator_imag) / denominator_norm;
        *out_abs_value = hypot(zr, zi);
    }
    return finite_double(*out_abs_value) ? PL_MATH_OK : PL_MATH_OVERFLOW;
}

/* ----------------------------- CNF / SAT ----------------------------- */

int pl_math_cnf_validate(size_t variables, const int32_t *literals, size_t literal_count,
                         const size_t *offsets, size_t clause_count) {
    if (variables == 0u || variables > PL_MATH_MAX_CNF_VARIABLES || offsets == NULL ||
        (literal_count != 0u && literals == NULL) || offsets[0] != 0u ||
        offsets[clause_count] != literal_count) return PL_MATH_INVALID;
    for (size_t c = 0u; c < clause_count; ++c) {
        if (offsets[c] > offsets[c + 1u] || offsets[c + 1u] > literal_count) return PL_MATH_INVALID;
    }
    for (size_t i = 0u; i < literal_count; ++i) {
        int64_t literal = literals[i];
        uint64_t variable = (uint64_t)(literal < 0 ? -literal : literal);
        if (literal == 0 || variable == 0u || variable > variables) return PL_MATH_INVALID;
    }
    return PL_MATH_OK;
}

static int cnf_evaluate_valid(size_t variables, const int32_t *literals,
                              const size_t *offsets, size_t clauses,
                              const uint8_t *assignment) {
    (void)variables;
    for (size_t c = 0u; c < clauses; ++c) {
        int clause_true = 0;
        for (size_t i = offsets[c]; i < offsets[c + 1u]; ++i) {
            int32_t literal = literals[i];
            size_t index = (size_t)(literal < 0 ? -(int64_t)literal : literal) - 1u;
            int value = assignment[index] != 0u;
            if ((literal > 0 && value) || (literal < 0 && !value)) { clause_true = 1; break; }
        }
        if (!clause_true) return 0;
    }
    return 1;
}

int pl_math_sat_verify_assignment(size_t variables, const int32_t *literals, size_t literal_count,
                                  const size_t *offsets, size_t clause_count,
                                  const uint8_t *assignment, int *satisfied) {
    int status;
    if (assignment == NULL || satisfied == NULL) return PL_MATH_INVALID;
    status = pl_math_cnf_validate(variables, literals, literal_count, offsets, clause_count);
    if (status != PL_MATH_OK) return status;
    for (size_t i = 0u; i < variables; ++i) if (assignment[i] > 1u) return PL_MATH_INVALID;
    *satisfied = cnf_evaluate_valid(variables, literals, offsets, clause_count, assignment);
    return PL_MATH_OK;
}

int pl_math_sat_find_model_bounded(size_t variables, const int32_t *literals, size_t literal_count,
                                   const size_t *offsets, size_t clause_count,
                                   uint64_t budget, uint8_t *assignment, uint64_t *checked) {
    uint64_t total;
    int status = pl_math_cnf_validate(variables, literals, literal_count, offsets, clause_count);
    if (status != PL_MATH_OK || assignment == NULL || checked == NULL || budget == 0u) return PL_MATH_INVALID;
    total = UINT64_C(1) << variables;
    *checked = 0u;
    for (uint64_t mask = 0u; mask < total && *checked < budget; ++mask) {
        for (size_t bit = 0u; bit < variables; ++bit) assignment[bit] = (uint8_t)((mask >> bit) & 1u);
        ++(*checked);
        if (cnf_evaluate_valid(variables, literals, offsets, clause_count, assignment)) return PL_MATH_OK;
    }
    return *checked == total ? PL_MATH_NOT_FOUND : PL_MATH_LIMIT;
}

/* ----------------------------- Collatz ----------------------------- */

int pl_math_collatz_step(uint64_t value, uint64_t *next) {
    if (value == 0u || next == NULL) return PL_MATH_INVALID;
    if ((value & 1u) == 0u) { *next = value / 2u; return PL_MATH_OK; }
    if (value > (UINT64_MAX - 1u) / 3u) return PL_MATH_OVERFLOW;
    *next = value * 3u + 1u;
    return PL_MATH_OK;
}

int pl_math_collatz_trajectory(uint64_t start, uint64_t budget, uint64_t *sequence,
                               size_t capacity, size_t *count, int *reached_one) {
    uint64_t value = start;
    size_t used = 0u;
    if (start == 0u || sequence == NULL || capacity == 0u || count == NULL || reached_one == NULL)
        return PL_MATH_INVALID;
    sequence[used++] = value;
    *reached_one = value == 1u;
    while (!*reached_one && (uint64_t)(used - 1u) < budget) {
        uint64_t next;
        int status;
        if (used >= capacity) { *count = used; return PL_MATH_LIMIT; }
        status = pl_math_collatz_step(value, &next);
        if (status != PL_MATH_OK) { *count = used; return status; }
        sequence[used++] = next;
        value = next;
        *reached_one = value == 1u;
    }
    *count = used;
    return *reached_one ? PL_MATH_OK : PL_MATH_LIMIT;
}

int pl_math_collatz_stopping_time(uint64_t start, uint64_t budget, uint64_t *steps) {
    uint64_t value = start;
    if (start == 0u || steps == NULL) return PL_MATH_INVALID;
    *steps = 0u;
    while (value != 1u && *steps < budget) {
        uint64_t next;
        int status = pl_math_collatz_step(value, &next);
        if (status != PL_MATH_OK) return status;
        value = next;
        ++(*steps);
    }
    return value == 1u ? PL_MATH_OK : PL_MATH_LIMIT;
}

/* ----------------------------- Navier-Stokes diagnostics ----------------------------- */

static size_t grid_index(size_t x, size_t y, size_t z, size_t nx, size_t ny) {
    return x + nx * (y + ny * z);
}

static double derivative_axis(const double *field, size_t x, size_t y, size_t z,
                              size_t nx, size_t ny, size_t nz, size_t axis, double h) {
    size_t coordinate = axis == 0u ? x : (axis == 1u ? y : z);
    size_t extent = axis == 0u ? nx : (axis == 1u ? ny : nz);
    size_t x0 = x, y0 = y, z0 = z, x1 = x, y1 = y, z1 = z;
    if (coordinate == 0u) {
        if (axis == 0u) ++x1; else if (axis == 1u) ++y1; else ++z1;
        return (field[grid_index(x1,y1,z1,nx,ny)] - field[grid_index(x,y,z,nx,ny)]) / h;
    }
    if (coordinate + 1u == extent) {
        if (axis == 0u) --x0; else if (axis == 1u) --y0; else --z0;
        return (field[grid_index(x,y,z,nx,ny)] - field[grid_index(x0,y0,z0,nx,ny)]) / h;
    }
    if (axis == 0u) { --x0; ++x1; }
    else if (axis == 1u) { --y0; ++y1; }
    else { --z0; ++z1; }
    return (field[grid_index(x1,y1,z1,nx,ny)] - field[grid_index(x0,y0,z0,nx,ny)]) / (2.0 * h);
}

static int validate_grid(const double *u, const double *v, const double *w,
                         size_t nx, size_t ny, size_t nz, double dx, double dy, double dz,
                         size_t *count) {
    if (u == NULL || v == NULL || w == NULL || count == NULL || nx < 2u || ny < 2u || nz < 2u ||
        !finite_double(dx) || !finite_double(dy) || !finite_double(dz) || dx <= 0.0 || dy <= 0.0 || dz <= 0.0)
        return PL_MATH_INVALID;
    if (!checked_product_size(nx, ny, count) || !checked_product_size(*count, nz, count)) return PL_MATH_OVERFLOW;
    for (size_t i = 0u; i < *count; ++i)
        if (!finite_double(u[i]) || !finite_double(v[i]) || !finite_double(w[i])) return PL_MATH_INVALID;
    return PL_MATH_OK;
}

int pl_math_grid3d_divergence(const double *u, const double *v, const double *w,
                              size_t nx, size_t ny, size_t nz, double dx, double dy, double dz, double *out) {
    size_t count;
    int status = validate_grid(u,v,w,nx,ny,nz,dx,dy,dz,&count);
    if (status != PL_MATH_OK || out == NULL) return status == PL_MATH_OK ? PL_MATH_INVALID : status;
    for (size_t z=0u; z<nz; ++z) for(size_t y=0u; y<ny; ++y) for(size_t x=0u; x<nx; ++x) {
        size_t i=grid_index(x,y,z,nx,ny);
        out[i]=derivative_axis(u,x,y,z,nx,ny,nz,0u,dx)+
               derivative_axis(v,x,y,z,nx,ny,nz,1u,dy)+
               derivative_axis(w,x,y,z,nx,ny,nz,2u,dz);
        if (!finite_double(out[i])) return PL_MATH_OVERFLOW;
    }
    return PL_MATH_OK;
}

int pl_math_grid3d_vorticity(const double *u, const double *v, const double *w,
                             size_t nx, size_t ny, size_t nz, double dx, double dy, double dz,
                             double *ox, double *oy, double *oz) {
    size_t count;
    int status = validate_grid(u,v,w,nx,ny,nz,dx,dy,dz,&count);
    if (status != PL_MATH_OK || ox == NULL || oy == NULL || oz == NULL) return status == PL_MATH_OK ? PL_MATH_INVALID : status;
    for (size_t z=0u; z<nz; ++z) for(size_t y=0u; y<ny; ++y) for(size_t x=0u; x<nx; ++x) {
        size_t i=grid_index(x,y,z,nx,ny);
        ox[i]=derivative_axis(w,x,y,z,nx,ny,nz,1u,dy)-derivative_axis(v,x,y,z,nx,ny,nz,2u,dz);
        oy[i]=derivative_axis(u,x,y,z,nx,ny,nz,2u,dz)-derivative_axis(w,x,y,z,nx,ny,nz,0u,dx);
        oz[i]=derivative_axis(v,x,y,z,nx,ny,nz,0u,dx)-derivative_axis(u,x,y,z,nx,ny,nz,1u,dy);
        if (!finite_double(ox[i]) || !finite_double(oy[i]) || !finite_double(oz[i])) return PL_MATH_OVERFLOW;
    }
    (void)count;
    return PL_MATH_OK;
}

int pl_math_navier_stokes_cfl_dt(double speed, double dx, double dy, double dz,
                                 double viscosity, double safety, double *out_dt) {
    double advective, diffusive, rate;
    if (out_dt == NULL || !finite_double(speed) || !finite_double(dx) || !finite_double(dy) ||
        !finite_double(dz) || !finite_double(viscosity) || !finite_double(safety) ||
        speed < 0.0 || dx <= 0.0 || dy <= 0.0 || dz <= 0.0 || viscosity < 0.0 || safety <= 0.0 || safety > 1.0)
        return PL_MATH_INVALID;
    advective = speed * (1.0/dx + 1.0/dy + 1.0/dz);
    diffusive = 2.0 * viscosity * (1.0/(dx*dx) + 1.0/(dy*dy) + 1.0/(dz*dz));
    rate = advective + diffusive;
    if (!finite_double(rate)) return PL_MATH_OVERFLOW;
    *out_dt = rate == 0.0 ? DBL_MAX : safety / rate;
    return finite_double(*out_dt) && *out_dt > 0.0 ? PL_MATH_OK : PL_MATH_OVERFLOW;
}

/* ----------------------------- Finite fields ----------------------------- */

static uint64_t add_mod_u64(uint64_t a, uint64_t b, uint64_t modulus) {
    return a >= modulus - b ? a - (modulus - b) : a + b;
}

static uint64_t mul_mod_u64(uint64_t a, uint64_t b, uint64_t modulus) {
    uint64_t result = 0u;
    a %= modulus;
    while (b != 0u) {
        if ((b & 1u) != 0u) result = add_mod_u64(result, a, modulus);
        b >>= 1u;
        if (b != 0u) a = add_mod_u64(a, a, modulus);
    }
    return result;
}

static uint64_t pow_mod_u64(uint64_t base, uint64_t exponent, uint64_t modulus) {
    uint64_t result = 1u;
    base %= modulus;
    while (exponent != 0u) {
        if ((exponent & 1u) != 0u) result = mul_mod_u64(result, base, modulus);
        exponent >>= 1u;
        if (exponent != 0u) base = mul_mod_u64(base, base, modulus);
    }
    return result;
}

static int is_prime_internal(uint64_t n) {
    static const uint64_t small[] = {2u,3u,5u,7u,11u,13u,17u,19u,23u,29u,31u,37u};
    static const uint64_t bases[] = {2u,325u,9375u,28178u,450775u,9780504u,1795265022u};
    uint64_t d, s = 0u;
    if (n < 2u) return 0;
    for (size_t i=0u; i<sizeof(small)/sizeof(small[0]); ++i) {
        if (n == small[i]) return 1;
        if (n % small[i] == 0u) return 0;
    }
    d = n - 1u;
    while ((d & 1u) == 0u) { d >>= 1u; ++s; }
    for (size_t i=0u; i<sizeof(bases)/sizeof(bases[0]); ++i) {
        uint64_t a = bases[i] % n;
        uint64_t x;
        int witness = 1;
        if (a == 0u) continue;
        x = pow_mod_u64(a,d,n);
        if (x == 1u || x == n-1u) continue;
        for (uint64_t r=1u; r<s; ++r) {
            x = mul_mod_u64(x,x,n);
            if (x == n-1u) { witness = 0; break; }
        }
        if (witness) return 0;
    }
    return 1;
}

int pl_math_is_prime_u64(uint64_t value, int *is_prime) {
    if (is_prime == NULL) return PL_MATH_INVALID;
    *is_prime = is_prime_internal(value);
    return PL_MATH_OK;
}

static int validate_prime32(uint32_t p) {
    return p >= 2u && p <= PL_MAX_PLAUSIBLE_PRIME && is_prime_internal(p);
}

int pl_math_elliptic_curve_nonsingular_mod_prime(uint32_t a, uint32_t b, uint32_t p, int *out) {
    uint64_t aa, bb, discriminant;
    if (out == NULL || p <= 3u || !validate_prime32(p)) return PL_MATH_INVALID;
    aa = (uint64_t)(a % p); bb = (uint64_t)(b % p);
    discriminant = (4u * mul_mod_u64(mul_mod_u64(aa,aa,p),aa,p) +
                    27u * mul_mod_u64(mul_mod_u64(bb,bb,p),bb,p)) % p;
    *out = discriminant != 0u;
    return PL_MATH_OK;
}

int pl_math_elliptic_curve_point_count_mod_prime(uint32_t a, uint32_t b, uint32_t p, uint32_t *count) {
    int nonsingular;
    uint64_t total = 1u;
    if (count == NULL || p <= 3u || p > 1000003u || !validate_prime32(p)) return PL_MATH_INVALID;
    if (pl_math_elliptic_curve_nonsingular_mod_prime(a,b,p,&nonsingular) != PL_MATH_OK || !nonsingular)
        return PL_MATH_INVALID;
    for (uint64_t x=0u; x<p; ++x) {
        uint64_t rhs = (mul_mod_u64(mul_mod_u64(x,x,p),x,p) +
                        mul_mod_u64((uint64_t)(a%p),x,p) + (b%p)) % p;
        if (rhs == 0u) ++total;
        else if (pow_mod_u64(rhs,(p-1u)/2u,p) == 1u) total += 2u;
    }
    *count = (uint32_t)total;
    return PL_MATH_OK;
}

int pl_math_elliptic_curve_frobenius_trace_mod_prime(uint32_t a, uint32_t b, uint32_t p, int64_t *trace) {
    uint32_t count;
    int status;
    if (trace == NULL) return PL_MATH_INVALID;
    status = pl_math_elliptic_curve_point_count_mod_prime(a,b,p,&count);
    if (status != PL_MATH_OK) return status;
    *trace = (int64_t)p + 1 - (int64_t)count;
    return PL_MATH_OK;
}

static int matrix_rank_mod_prime(const uint32_t *matrix, size_t rows, size_t cols,
                                 uint32_t prime, size_t *rank) {
    uint32_t work[PL_MATH_MAX_FINITE_FIELD_DIMENSION][PL_MATH_MAX_FINITE_FIELD_DIMENSION];
    size_t pivot_row = 0u;
    if (matrix == NULL || rank == NULL || rows > PL_MATH_MAX_FINITE_FIELD_DIMENSION ||
        cols > PL_MATH_MAX_FINITE_FIELD_DIMENSION || !validate_prime32(prime)) return PL_MATH_INVALID;
    for (size_t r=0u; r<rows; ++r) for(size_t c=0u; c<cols; ++c)
        work[r][c] = matrix[(r*cols)+c] % prime;
    for (size_t col=0u; col<cols && pivot_row<rows; ++col) {
        size_t found = pivot_row;
        while (found<rows && work[found][col]==0u) ++found;
        if (found==rows) continue;
        if (found!=pivot_row) for(size_t c=col; c<cols; ++c) {
            uint32_t tmp=work[pivot_row][c]; work[pivot_row][c]=work[found][c]; work[found][c]=tmp;
        }
        {
            uint64_t inverse = pow_mod_u64(work[pivot_row][col], prime-2u, prime);
            for(size_t c=col; c<cols; ++c) work[pivot_row][c]=(uint32_t)mul_mod_u64(work[pivot_row][c],inverse,prime);
        }
        for(size_t r=0u; r<rows; ++r) if(r!=pivot_row && work[r][col]!=0u) {
            uint32_t factor=work[r][col];
            for(size_t c=col; c<cols; ++c) {
                uint64_t product=mul_mod_u64(factor,work[pivot_row][c],prime);
                work[r][c]=(uint32_t)((work[r][c]+prime-product)%prime);
            }
        }
        ++pivot_row;
    }
    *rank=pivot_row;
    return PL_MATH_OK;
}

static int vector_span_mod_prime(const uint32_t *vectors, size_t count, size_t dimension,
                                 const uint32_t *candidate, uint32_t prime, int *inside) {
    uint32_t equations[PL_MATH_MAX_FINITE_FIELD_DIMENSION * PL_MATH_MAX_FINITE_FIELD_DIMENSION];
    uint32_t augmented[PL_MATH_MAX_FINITE_FIELD_DIMENSION * PL_MATH_MAX_FINITE_FIELD_DIMENSION];
    size_t r1=0u, r2=0u;
    if (vectors == NULL || candidate == NULL || inside == NULL || count > 63u || dimension == 0u || dimension > 63u ||
        !validate_prime32(prime)) return PL_MATH_INVALID;
    for(size_t r=0u;r<dimension;++r) {
        for(size_t c=0u;c<count;++c) equations[r*count+c]=vectors[c*dimension+r]%prime;
        for(size_t c=0u;c<count;++c) augmented[r*(count+1u)+c]=equations[r*count+c];
        augmented[r*(count+1u)+count]=candidate[r]%prime;
    }
    {
        int s=matrix_rank_mod_prime(equations,dimension,count,prime,&r1);
        if(s!=PL_MATH_OK) return s;
        s=matrix_rank_mod_prime(augmented,dimension,count+1u,prime,&r2);
        if(s!=PL_MATH_OK) return s;
    }
    *inside=r1==r2;
    return PL_MATH_OK;
}

int pl_math_hodge_type_check(unsigned degree, unsigned p, unsigned q, int *is_type) {
    if (is_type == NULL) return PL_MATH_INVALID;
    *is_type = (p == q && p <= UINT_MAX/2u && 2u*p == degree);
    return PL_MATH_OK;
}

int pl_math_hodge_cycle_span_rank_mod_prime(const uint32_t *cycles, size_t count,
                                            size_t dimension, uint32_t prime, size_t *rank) {
    return matrix_rank_mod_prime(cycles,count,dimension,prime,rank);
}

int pl_math_hodge_class_in_cycle_span_mod_prime(const uint32_t *cycles, size_t count,
                                                size_t dimension, const uint32_t *candidate,
                                                uint32_t prime, int *in_span) {
    return vector_span_mod_prime(cycles,count,dimension,candidate,prime,in_span);
}

int pl_math_tate_fixed_vector_verify(const uint32_t *frob, size_t dimension,
                                     const uint32_t *vector, uint32_t prime, int *fixed) {
    if (frob==NULL || vector==NULL || fixed==NULL || dimension==0u ||
        dimension>PL_MATH_MAX_FINITE_FIELD_DIMENSION || !validate_prime32(prime)) return PL_MATH_INVALID;
    *fixed=1;
    for(size_t r=0u;r<dimension;++r) {
        uint64_t total=0u;
        for(size_t c=0u;c<dimension;++c)
            total=(total+mul_mod_u64(frob[r*dimension+c]%prime,vector[c]%prime,prime))%prime;
        if(total!=vector[r]%prime) { *fixed=0; break; }
    }
    return PL_MATH_OK;
}

int pl_math_tate_invariant_dimension_mod_prime(const uint32_t *frob, size_t dimension,
                                               uint32_t prime, size_t *invariant_dimension) {
    uint32_t difference[PL_MATH_MAX_FINITE_FIELD_DIMENSION*PL_MATH_MAX_FINITE_FIELD_DIMENSION];
    size_t rank=0u;
    if(frob==NULL || invariant_dimension==NULL || dimension==0u ||
       dimension>PL_MATH_MAX_FINITE_FIELD_DIMENSION || !validate_prime32(prime)) return PL_MATH_INVALID;
    for(size_t r=0u;r<dimension;++r) for(size_t c=0u;c<dimension;++c) {
        uint32_t value=frob[r*dimension+c]%prime;
        difference[r*dimension+c]=(uint32_t)((value + (r==c ? prime-1u : 0u))%prime);
    }
    {
        int s=matrix_rank_mod_prime(difference,dimension,dimension,prime,&rank);
        if(s!=PL_MATH_OK) return s;
    }
    *invariant_dimension=dimension-rank;
    return PL_MATH_OK;
}

int pl_math_tate_cycle_span_rank_mod_prime(const uint32_t *vectors, size_t count,
                                           size_t dimension, uint32_t prime, size_t *rank) {
    return matrix_rank_mod_prime(vectors,count,dimension,prime,rank);
}

/* ----------------------------- U(1) lattice model ----------------------------- */

int pl_math_u1_plaquette_phase(const double angles[4], const int8_t orientation[4], double *out) {
    double total=0.0;
    if(angles==NULL || orientation==NULL || out==NULL) return PL_MATH_INVALID;
    for(size_t i=0u;i<4u;++i) {
        if(!finite_double(angles[i]) || (orientation[i]!=1 && orientation[i]!=-1)) return PL_MATH_INVALID;
        total += (double)orientation[i]*angles[i];
    }
    *out=wrap_phase(total);
    return finite_double(*out)?PL_MATH_OK:PL_MATH_OVERFLOW;
}

int pl_math_u1_wilson_action(double beta, double phase, double *out) {
    if(out==NULL || !finite_double(beta) || !finite_double(phase) || beta<0.0) return PL_MATH_INVALID;
    *out=beta*(1.0-cos(phase));
    return finite_double(*out)?PL_MATH_OK:PL_MATH_OVERFLOW;
}

int pl_math_correlator_effective_mass(double ct, double next, double spacing, double *out) {
    if(out==NULL || !finite_double(ct) || !finite_double(next) || !finite_double(spacing) ||
       ct<=0.0 || next<=0.0 || spacing<=0.0) return PL_MATH_INVALID;
    *out=log(ct/next)/spacing;
    return finite_double(*out)?PL_MATH_OK:PL_MATH_OVERFLOW;
}

/* ----------------------------- Goldbach / twin primes ----------------------------- */

int pl_math_goldbach_verify(uint64_t n, uint64_t a, int *valid) {
    if(valid==NULL || n<=2u || (n&1u)!=0u || a>n) return PL_MATH_INVALID;
    *valid=is_prime_internal(a) && is_prime_internal(n-a);
    return PL_MATH_OK;
}

int pl_math_goldbach_find_pair_bounded(uint64_t n, uint64_t budget,
                                       uint64_t *a, uint64_t *b, uint64_t *checked) {
    if(a==NULL || b==NULL || checked==NULL || n<=2u || (n&1u)!=0u || budget==0u) return PL_MATH_INVALID;
    *checked=0u;
    for(uint64_t candidate=2u; candidate<=n/2u; ++candidate) {
        if (*checked >= budget) return PL_MATH_LIMIT;
        ++(*checked);
        if(is_prime_internal(candidate) && is_prime_internal(n-candidate)) {
            *a=candidate; *b=n-candidate; return PL_MATH_OK;
        }
    }
    return PL_MATH_NOT_FOUND;
}

int pl_math_twin_prime_verify(uint64_t p, int *valid) {
    if(valid==NULL || p>UINT64_MAX-2u) return PL_MATH_INVALID;
    *valid=is_prime_internal(p) && is_prime_internal(p+2u);
    return PL_MATH_OK;
}

int pl_math_twin_prime_next_bounded(uint64_t lower, uint64_t budget, uint64_t *found, uint64_t *checked) {
    uint64_t candidate=lower<3u?3u:lower;
    if(found==NULL || checked==NULL || budget==0u) return PL_MATH_INVALID;
    if((candidate&1u)==0u) ++candidate;
    *checked=0u;
    while (*checked < budget && candidate <= UINT64_MAX - 2u) {
        ++(*checked);
        if (is_prime_internal(candidate) && is_prime_internal(candidate + 2u)) {
            *found = candidate;
            return PL_MATH_OK;
        }
        if (candidate >= UINT64_MAX - 2u) break;
        candidate += 2u;
    }
    return PL_MATH_LIMIT;
}

int pl_math_twin_prime_count_bounded(uint64_t upper, uint64_t budget, uint64_t *count, uint64_t *checked) {
    if(count==NULL || checked==NULL || upper<5u || budget==0u) return PL_MATH_INVALID;
    *count=0u; *checked=0u;
    for(uint64_t p=3u;p<=upper-2u;p+=2u) {
        if(*checked>=budget) return PL_MATH_LIMIT;
        ++(*checked);
        if (is_prime_internal(p) && is_prime_internal(p + 2u)) ++(*count);
        if (p >= upper - 2u || p > UINT64_MAX - 2u) break;
    }
    return PL_MATH_OK;
}

/* ----------------------------- Bounded halting experiment ----------------------------- */

int pl_math_vm_step(const pl_math_vm_instruction *program, size_t length,
                    const pl_math_vm_state *before, pl_math_vm_state *after) {
    pl_math_vm_state next;
    pl_math_vm_instruction instruction;
    if(program==NULL || before==NULL || after==NULL || length==0u || before->halted>1u) return PL_MATH_INVALID;
    next=*before;
    if(next.halted) { *after=next; return PL_MATH_OK; }
    if(next.pc>=length || next.steps==UINT64_MAX) return PL_MATH_INVALID;
    instruction=program[next.pc];
    switch(instruction.opcode) {
        case PL_MATH_VM_HALT: next.halted=1u; break;
        case PL_MATH_VM_INCREMENT:
            if(instruction.reg>1u || next.registers[instruction.reg]==UINT64_MAX) return PL_MATH_OVERFLOW;
            ++next.registers[instruction.reg]; ++next.pc; break;
        case PL_MATH_VM_DECREMENT_OR_JUMP:
            if(instruction.reg>1u || instruction.target>=length) return PL_MATH_INVALID;
            if(next.registers[instruction.reg]==0u) next.pc=instruction.target;
            else { --next.registers[instruction.reg]; ++next.pc; }
            break;
        case PL_MATH_VM_JUMP:
            if(instruction.target>=length) return PL_MATH_INVALID;
            next.pc=instruction.target; break;
        default: return PL_MATH_INVALID;
    }
    ++next.steps;
    *after=next;
    return PL_MATH_OK;
}

int pl_math_vm_run_bounded(const pl_math_vm_instruction *program, size_t length,
                           pl_math_vm_state *state, uint64_t fuel) {
    if(program==NULL || state==NULL || length==0u) return PL_MATH_INVALID;
    for(uint64_t i=0u;i<fuel && !state->halted;++i) {
        pl_math_vm_state next;
        int status=pl_math_vm_step(program,length,state,&next);
        if(status!=PL_MATH_OK) return status;
        *state=next;
    }
    return state->halted ? PL_MATH_OK : PL_MATH_LIMIT;
}

int pl_math_vm_trace_verify(const pl_math_vm_instruction *program, size_t length,
                            const pl_math_vm_state *trace, size_t trace_length) {
    if(program==NULL || trace==NULL || trace_length==0u || length==0u) return PL_MATH_INVALID;
    for(size_t i=0u;i+1u<trace_length;++i) {
        pl_math_vm_state next;
        int status=pl_math_vm_step(program,length,&trace[i],&next);
        if(status!=PL_MATH_OK) return status;
        if(next.pc!=trace[i+1u].pc || next.registers[0]!=trace[i+1u].registers[0] ||
           next.registers[1]!=trace[i+1u].registers[1] || next.steps!=trace[i+1u].steps ||
           next.halted!=trace[i+1u].halted) return PL_MATH_INVALID;
    }
    return PL_MATH_OK;
}

/* ----------------------------- Finite unit-distance graphs ----------------------------- */

int pl_math_unit_graph_build(const int64_t *xy, size_t n, uint32_t scale,
                             pl_math_edge *edges, size_t capacity, size_t *edge_count) {
    size_t needed=0u;
    long double target=(long double)scale*(long double)scale;
    if(xy==NULL || edges==NULL || edge_count==NULL || n==0u || n>PL_MATH_MAX_GRAPH_VERTICES || scale==0u)
        return PL_MATH_INVALID;
    if (scale > 66000000u) return PL_MATH_INVALID;
    for (size_t i = 0u; i < n; ++i) {
        if (xy[2u*i] < -33000000LL || xy[2u*i] > 33000000LL ||
            xy[2u*i+1u] < -33000000LL || xy[2u*i+1u] > 33000000LL) return PL_MATH_INVALID;
    }
    for(size_t i=0u;i<n;++i) for(size_t j=i+1u;j<n;++j) {
        int64_t dx=xy[2u*i]-xy[2u*j];
        int64_t dy=xy[2u*i+1u]-xy[2u*j+1u];
        uint64_t distance_squared=(uint64_t)(dx*dx)+(uint64_t)(dy*dy);
        if((long double)distance_squared==target) {
            if(needed>=capacity) { *edge_count=needed+1u; return PL_MATH_LIMIT; }
            edges[needed].left=i; edges[needed].right=j; ++needed;
        }
    }
    *edge_count=needed;
    return PL_MATH_OK;
}

static int validate_edges(size_t vertices, const pl_math_edge *edges, size_t count) {
    if((count!=0u && edges==NULL) || vertices==0u || vertices>PL_MATH_MAX_GRAPH_VERTICES) return 0;
    for(size_t i=0u;i<count;++i)
        if(edges[i].left>=vertices || edges[i].right>=vertices || edges[i].left==edges[i].right) return 0;
    return 1;
}

int pl_math_graph_coloring_verify(size_t vertices, const pl_math_edge *edges, size_t edge_count,
                                  const uint8_t *colors, size_t color_count, int *valid) {
    if(!validate_edges(vertices,edges,edge_count) || colors==NULL || valid==NULL || color_count==0u || color_count>255u)
        return PL_MATH_INVALID;
    *valid=1;
    for(size_t i=0u;i<vertices;++i) if(colors[i]>=color_count) return PL_MATH_INVALID;
    for(size_t i=0u;i<edge_count;++i) if(colors[edges[i].left]==colors[edges[i].right]) { *valid=0; break; }
    return PL_MATH_OK;
}

static int coloring_search(size_t vertex, size_t vertices, const pl_math_edge *edges, size_t edge_count,
                           uint8_t colors_count, uint8_t *colors, uint64_t budget, uint64_t *nodes) {
    if(*nodes>=budget) return -1;
    ++(*nodes);
    if(vertex==vertices) return 1;
    for(uint8_t color=0u;color<colors_count;++color) {
        int allowed=1;
        for(size_t e=0u;e<edge_count;++e) {
            size_t other=SIZE_MAX;
            if(edges[e].left==vertex) other=edges[e].right;
            else if(edges[e].right==vertex) other=edges[e].left;
            if(other!=SIZE_MAX && other<vertex && colors[other]==color) { allowed=0; break; }
        }
        if(allowed) {
            colors[vertex]=color;
            { int result=coloring_search(vertex+1u,vertices,edges,edge_count,colors_count,colors,budget,nodes);
              if(result!=0) return result; }
        }
    }
    colors[vertex]=UINT8_MAX;
    return 0;
}

int pl_math_graph_coloring_search_bounded(size_t vertices, const pl_math_edge *edges, size_t edge_count,
                                          uint8_t color_count, uint64_t budget, uint8_t *colors,
                                          uint64_t *nodes) {
    int result;
    if(!validate_edges(vertices,edges,edge_count) || color_count==0u || budget==0u || colors==NULL || nodes==NULL)
        return PL_MATH_INVALID;
    memset(colors,UINT8_MAX,vertices);
    *nodes=0u;
    result=coloring_search(0u,vertices,edges,edge_count,color_count,colors,budget,nodes);
    return result==1 ? PL_MATH_OK : (result<0 ? PL_MATH_LIMIT : PL_MATH_NOT_FOUND);
}

/* ----------------------------- Inscribed square on a polygonal curve ----------------------------- */

static double point_segment_distance(pl_math_point2 p, pl_math_point2 a, pl_math_point2 b) {
    double dx=b.x-a.x, dy=b.y-a.y;
    double denom=dx*dx+dy*dy;
    double t;
    if(denom==0.0) return hypot(p.x-a.x,p.y-a.y);
    t=((p.x-a.x)*dx+(p.y-a.y)*dy)/denom;
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    return hypot(p.x-(a.x+t*dx),p.y-(a.y+t*dy));
}

static int point_on_polyline_internal(const pl_math_point2 *curve,size_t count,pl_math_point2 p,double tol) {
    for(size_t i=0u;i+1u<count;++i)
        if(point_segment_distance(p,curve[i],curve[i+1u])<=tol) return 1;
    return 0;
}

int pl_math_point_on_polyline(const pl_math_point2 *curve, size_t count,
                              pl_math_point2 point, double tolerance, int *on_curve) {
    if(curve==NULL || on_curve==NULL || count<2u || !finite_double(point.x) || !finite_double(point.y) ||
       !finite_double(tolerance) || tolerance<0.0) return PL_MATH_INVALID;
    for(size_t i=0u;i<count;++i) if(!finite_double(curve[i].x)||!finite_double(curve[i].y)) return PL_MATH_INVALID;
    *on_curve=point_on_polyline_internal(curve,count,point,tolerance);
    return PL_MATH_OK;
}

int pl_math_square_on_polyline_verify(const pl_math_point2 *curve, size_t count,
                                      const pl_math_point2 square[4], double tol, int *valid) {
    double side[4], dot;
    if (curve == NULL || square == NULL || valid == NULL || count < 4u || !finite_double(tol) || tol < 0.0)
        return PL_MATH_INVALID;
    for (size_t i = 0u; i < count; ++i)
        if (!finite_double(curve[i].x) || !finite_double(curve[i].y)) return PL_MATH_INVALID;
    if (hypot(curve[0].x - curve[count-1u].x, curve[0].y - curve[count-1u].y) > tol)
        return PL_MATH_INVALID;
    *valid = 0;
    for(size_t i=0u;i<4u;++i) {
        if(!finite_double(square[i].x)||!finite_double(square[i].y)) return PL_MATH_INVALID;
        if(!point_on_polyline_internal(curve,count,square[i],tol)) return PL_MATH_OK;
        side[i]=hypot(square[(i+1u)%4u].x-square[i].x,square[(i+1u)%4u].y-square[i].y);
        if(side[i]<=tol) return PL_MATH_OK;
    }
    for(size_t i=1u;i<4u;++i) if(fabs(side[i]-side[0])>tol*(1.0+side[0])) return PL_MATH_OK;
    dot=(square[1].x-square[0].x)*(square[3].x-square[0].x)+
        (square[1].y-square[0].y)*(square[3].y-square[0].y);
    if(fabs(dot)>tol*(1.0+side[0]*side[0])) return PL_MATH_OK;
    if(hypot(square[2].x-square[0].x,square[2].y-square[0].y) <= tol) return PL_MATH_OK;
    *valid=1;
    return PL_MATH_OK;
}

int pl_math_inscribed_square_search_sampled(const pl_math_point2 *curve,size_t count,
                                            unsigned subdivisions,double tol,pl_math_point2 square[4],
                                            uint64_t *triples) {
    pl_math_point2 samples[PL_MATH_MAX_CURVE_SAMPLES];
    size_t sample_count=0u;
    if (curve == NULL || square == NULL || triples == NULL || count < 4u ||
        count > PL_MATH_MAX_CURVE_SAMPLES || subdivisions == 0u || subdivisions > 8u ||
        !finite_double(tol) || tol < 0.0 || (count - 1u) * subdivisions + 1u > PL_MATH_MAX_CURVE_SAMPLES)
        return PL_MATH_INVALID;
    if (hypot(curve[0].x-curve[count-1u].x, curve[0].y-curve[count-1u].y) > tol) return PL_MATH_INVALID;
    for(size_t i=0u;i+1u<count;++i) {
        if(!finite_double(curve[i].x)||!finite_double(curve[i].y)) return PL_MATH_INVALID;
        for(unsigned k=0u;k<subdivisions;++k) {
            double t=(double)k/(double)subdivisions;
            samples[sample_count].x=curve[i].x+(curve[i+1u].x-curve[i].x)*t;
            samples[sample_count].y=curve[i].y+(curve[i+1u].y-curve[i].y)*t;
            ++sample_count;
        }
    }
    if(!finite_double(curve[count-1u].x)||!finite_double(curve[count-1u].y)) return PL_MATH_INVALID;
    samples[sample_count++]=curve[count-1u];
    *triples=0u;
    for(size_t i=0u;i<sample_count;++i) for(size_t j=0u;j<sample_count;++j) {
        if(j==i) continue;
        for(size_t k=0u;k<sample_count;++k) {
            pl_math_point2 candidate[4];
            int valid=0;
            if(k==i || k==j) continue;
            ++(*triples);
            candidate[0]=samples[i]; candidate[1]=samples[j]; candidate[2]=samples[k];
            candidate[3].x=candidate[0].x+candidate[2].x-candidate[1].x;
            candidate[3].y=candidate[0].y+candidate[2].y-candidate[1].y;
            if(pl_math_square_on_polyline_verify(curve,count,candidate,tol,&valid)==PL_MATH_OK && valid) {
                memcpy(square,candidate,sizeof(candidate));
                return PL_MATH_OK;
            }
        }
    }
    return PL_MATH_NOT_FOUND;
}

/* ----------------------------- Legendre / Beal / cuboid ----------------------------- */

int pl_math_legendre_interval(uint64_t n,uint64_t *lower,uint64_t *upper) {
    if(lower==NULL || upper==NULL || n==0u || n>=UINT32_MAX) return PL_MATH_INVALID;
    *lower=n*n; *upper=(n+1u)*(n+1u);
    return PL_MATH_OK;
}

int pl_math_legendre_witness_verify(uint64_t n,uint64_t candidate,int *valid) {
    uint64_t lower,upper;
    int status;
    if(valid==NULL) return PL_MATH_INVALID;
    status=pl_math_legendre_interval(n,&lower,&upper);
    if(status!=PL_MATH_OK) return status;
    *valid=candidate>lower && candidate<upper && is_prime_internal(candidate);
    return PL_MATH_OK;
}

int pl_math_legendre_find_prime_bounded(uint64_t n,uint64_t budget,uint64_t *found,uint64_t *checked) {
    uint64_t lower,upper;
    int status;
    if(found==NULL || checked==NULL || budget==0u) return PL_MATH_INVALID;
    status=pl_math_legendre_interval(n,&lower,&upper);
    if(status!=PL_MATH_OK) return status;
    *checked=0u;
    for(uint64_t candidate=lower+1u;candidate<upper && *checked<budget;++candidate) {
        ++(*checked);
        if(is_prime_internal(candidate)) { *found=candidate; return PL_MATH_OK; }
    }
    return *checked>=budget && lower+1u<upper ? PL_MATH_LIMIT : PL_MATH_NOT_FOUND;
}

static int checked_power_u64(uint64_t base,uint32_t exponent,uint64_t *out) {
    uint64_t result=1u;
    if(out==NULL || exponent==0u) return PL_MATH_INVALID;
    while(exponent!=0u) {
        if((exponent&1u)!=0u) { if(base!=0u && result>UINT64_MAX/base) return PL_MATH_OVERFLOW; result*=base; }
        exponent>>=1u;
        if(exponent!=0u) { if(base!=0u && base>UINT64_MAX/base) return PL_MATH_OVERFLOW; base*=base; }
    }
    *out=result;
    return PL_MATH_OK;
}

static uint64_t gcd_u64(uint64_t a,uint64_t b) {
    while(b!=0u) { uint64_t t=a%b; a=b; b=t; }
    return a;
}

int pl_math_beal_power_sum_check(uint64_t a,uint32_t x,uint64_t b,uint32_t y,uint64_t c,uint32_t z,int *equal) {
    uint64_t ax,by,cz;
    int s;
    if(equal==NULL || a==0u || b==0u || c==0u || x==0u || y==0u || z==0u) return PL_MATH_INVALID;
    s=checked_power_u64(a,x,&ax); if(s!=PL_MATH_OK)return s;
    s=checked_power_u64(b,y,&by); if(s!=PL_MATH_OK)return s;
    s=checked_power_u64(c,z,&cz); if(s!=PL_MATH_OK)return s;
    if(ax>UINT64_MAX-by)return PL_MATH_OVERFLOW;
    *equal=ax+by==cz;
    return PL_MATH_OK;
}

int pl_math_beal_common_prime_divisor(uint64_t a,uint64_t b,uint64_t c,uint64_t *factor) {
    uint64_t common;
    if(factor==NULL || a==0u || b==0u || c==0u) return PL_MATH_INVALID;
    common=gcd_u64(a,gcd_u64(b,c));
    *factor=0u;
    if(common<=1u) return PL_MATH_OK;
    if((common&1u)==0u) { *factor=2u; return PL_MATH_OK; }
    for(uint64_t p=3u;p<=common/p;p+=2u) if(common%p==0u) { *factor=p; return PL_MATH_OK; }
    *factor=common;
    return PL_MATH_OK;
}

int pl_math_beal_counterexample_screen(uint64_t a,uint32_t x,uint64_t b,uint32_t y,
                                       uint64_t c,uint32_t z,int *candidate) {
    int equal;
    uint64_t factor;
    int status;
    if(candidate==NULL || x<=2u || y<=2u || z<=2u) return PL_MATH_INVALID;
    status=pl_math_beal_power_sum_check(a,x,b,y,c,z,&equal); if(status!=PL_MATH_OK)return status;
    if(!equal) { *candidate=0; return PL_MATH_OK; }
    status=pl_math_beal_common_prime_divisor(a,b,c,&factor); if(status!=PL_MATH_OK)return status;
    *candidate=factor==0u;
    return PL_MATH_OK;
}

static int square_u64(uint64_t n,uint64_t *root) {
    long double approx=sqrtl((long double)n);
    uint64_t r=(uint64_t)approx;
    while(r>0u && r>n/r) --r;
    while(r<UINT64_MAX && (r+1u)<=n/(r+1u)) ++r;
    if(root!=NULL)*root=r;
    return r*r==n;
}

static int sum_squares(uint64_t a,uint64_t b,uint64_t *sum) {
    if(a!=0u && a>UINT64_MAX/a) return 0;
    if(b!=0u && b>UINT64_MAX/b) return 0;
    { uint64_t aa=a*a,bb=b*b; if(aa>UINT64_MAX-bb)return 0; *sum=aa+bb; }
    return 1;
}

int pl_math_cuboid_face_diagonals(uint64_t a,uint64_t b,uint64_t c,uint64_t diagonals[3],int *all_integer) {
    uint64_t sums[3],roots[3];
    if(diagonals==NULL || all_integer==NULL || a==0u || b==0u || c==0u) return PL_MATH_INVALID;
    if(!sum_squares(a,b,&sums[0]) || !sum_squares(a,c,&sums[1]) || !sum_squares(b,c,&sums[2])) return PL_MATH_OVERFLOW;
    *all_integer=1;
    for(size_t i=0u;i<3u;++i) { if(!square_u64(sums[i],&roots[i])) { *all_integer=0; roots[i]=0u; } diagonals[i]=roots[i]; }
    return PL_MATH_OK;
}

int pl_math_perfect_cuboid_verify(uint64_t a, uint64_t b, uint64_t c, int *perfect) {
    uint64_t ab, ac, bc, aa, bb, cc, space;
    if (perfect == NULL || a == 0u || b == 0u || c == 0u) return PL_MATH_INVALID;
    if (!sum_squares(a, b, &ab) || !sum_squares(a, c, &ac) || !sum_squares(b, c, &bc))
        return PL_MATH_OVERFLOW;
    if (!square_u64(ab, NULL) || !square_u64(ac, NULL) || !square_u64(bc, NULL)) {
        *perfect = 0;
        return PL_MATH_OK;
    }
    if (a != 0u && a > UINT64_MAX / a) return PL_MATH_OVERFLOW;
    if (b != 0u && b > UINT64_MAX / b) return PL_MATH_OVERFLOW;
    if (c != 0u && c > UINT64_MAX / c) return PL_MATH_OVERFLOW;
    aa = a * a; bb = b * b; cc = c * c;
    if (aa > UINT64_MAX - bb || aa + bb > UINT64_MAX - cc) return PL_MATH_OVERFLOW;
    space = aa + bb + cc;
    *perfect = square_u64(space, NULL);
    return PL_MATH_OK;
}

int pl_math_perfect_cuboid_search_bounded(uint64_t max_edge,uint64_t budget,uint64_t edges[3],uint64_t *nodes) {
    if(max_edge<1u || max_edge>100000u || budget==0u || edges==NULL || nodes==NULL) return PL_MATH_INVALID;
    *nodes=0u;
    for(uint64_t a=1u;a<=max_edge;++a) for(uint64_t b=a;b<=max_edge;++b) {
        uint64_t ab;
        if(!sum_squares(a,b,&ab)) continue;
        if(!square_u64(ab,NULL)) continue;
        for(uint64_t c=b;c<=max_edge;++c) {
            int perfect=0;
            if(*nodes>=budget)return PL_MATH_LIMIT;
            ++(*nodes);
            if(pl_math_perfect_cuboid_verify(a,b,c,&perfect)!=PL_MATH_OK) continue;
            if(perfect) { edges[0]=a; edges[1]=b; edges[2]=c; return PL_MATH_OK; }
        }
    }
    return PL_MATH_NOT_FOUND;
}

/* ----------------------------- Matrix kernels / empirical exponent ----------------------------- */

int pl_math_matrix_multiply_f64(const double *a,const double *b,size_t n,double *out) {
    size_t count;
    double result[PL_MATH_MAX_MATRIX_DIMENSION * PL_MATH_MAX_MATRIX_DIMENSION];
    if(a==NULL || b==NULL || out==NULL || n==0u || n>PL_MATH_MAX_MATRIX_DIMENSION || !checked_product_size(n,n,&count)) return PL_MATH_INVALID;
    for(size_t i=0u;i<count;++i)if(!finite_double(a[i])||!finite_double(b[i]))return PL_MATH_INVALID;
    for(size_t r=0u;r<n;++r)for(size_t c=0u;c<n;++c) {
        long double sum=0.0L;
        for(size_t k=0u;k<n;++k)sum+=(long double)a[r*n+k]*(long double)b[k*n+c];
        if(fabsl(sum)>(long double)DBL_MAX)return PL_MATH_OVERFLOW;
        result[r*n+c]=(double)sum;
        if(!finite_double(result[r*n+c]))return PL_MATH_OVERFLOW;
    }
    memcpy(out,result,count*sizeof(*out));
    return PL_MATH_OK;
}

int pl_math_matrix_product_verify_f64(const double *a,const double *b,const double *candidate,
                                      size_t n,double tolerance,int *valid) {
    size_t count;
    if(a==NULL||b==NULL||candidate==NULL||valid==NULL||n==0u||n>PL_MATH_MAX_MATRIX_DIMENSION||
       !finite_double(tolerance)||tolerance<0.0||!checked_product_size(n,n,&count))return PL_MATH_INVALID;
    *valid=1;
    for(size_t r=0u;r<n;++r)for(size_t c=0u;c<n;++c) {
        long double expected=0.0L;
        double actual=candidate[r*n+c];
        if(!finite_double(actual))return PL_MATH_INVALID;
        for(size_t k=0u;k<n;++k) {
            if(!finite_double(a[r*n+k])||!finite_double(b[k*n+c]))return PL_MATH_INVALID;
            expected+=(long double)a[r*n+k]*(long double)b[k*n+c];
        }
        if(fabsl(expected-(long double)actual)>(long double)tolerance*(1.0L+fabsl(expected))) { *valid=0; return PL_MATH_OK; }
    }
    (void)count;
    return PL_MATH_OK;
}

int pl_math_matrix_scaling_exponent(size_t n1,double t1,size_t n2,double t2,double *exponent) {
    if(exponent==NULL||n1==0u||n2<=n1||!finite_double(t1)||!finite_double(t2)||t1<=0.0||t2<=0.0)
        return PL_MATH_INVALID;
    *exponent=log(t2/t1)/log((double)n2/(double)n1);
    return finite_double(*exponent)?PL_MATH_OK:PL_MATH_OVERFLOW;
}
