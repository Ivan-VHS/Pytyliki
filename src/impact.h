#ifndef PYTYLIKI_IMPACT_H
#define PYTYLIKI_IMPACT_H

#include <stddef.h>

#define PL_IMPACT_PROFILE_COUNT 20u

typedef enum {
    PL_IMPACT_ACOUSTIC = 0u,
    PL_IMPACT_SCALE,
    PL_IMPACT_KINETIC,
    PL_IMPACT_REACH,
    PL_IMPACT_IMPULSE,
    PL_IMPACT_DIMENSION_COUNT
} pl_impact_dimension;

typedef struct {
    unsigned id;
    const char *sound;
    const char *scale_word;
    const char *motion_word;
    unsigned acoustic_rank;
    unsigned scale_rank;
    unsigned kinetic_rank;
    unsigned reach_rank;
    unsigned impulse_rank;
} pl_impact_profile;

/* Ranks are ordinal lexical weights (1..5), not physical measurements. */
size_t pl_impact_catalog(const pl_impact_profile **out);
int pl_impact_find(unsigned id, const pl_impact_profile **out);
int pl_impact_find_term(const char *term, const pl_impact_profile **out);
const char *pl_impact_dimension_name(unsigned dimension);
unsigned pl_impact_profile_rank(const pl_impact_profile *profile, unsigned dimension);
int pl_impact_profile_validate(const pl_impact_profile *profile);

#endif
