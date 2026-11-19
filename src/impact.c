#include "impact.h"

#include <string.h>

static const pl_impact_profile profiles[PL_IMPACT_PROFILE_COUNT] = {
    { 1u, "Грохот", "Массивный", "Баллистический", 5u, 5u, 4u, 3u, 4u },
    { 2u, "Громыхание", "Громоздкий", "Ракетный", 5u, 5u, 5u, 4u, 5u },
    { 3u, "Раскат", "Объёмный", "Снарядный", 4u, 4u, 4u, 3u, 4u },
    { 4u, "Рокот", "Крупный", "Артиллерийский", 5u, 4u, 4u, 4u, 4u },
    { 5u, "Гул", "Гигантский", "Гиперзвуковой", 5u, 5u, 5u, 5u, 5u },
    { 6u, "Рёв", "Исполинский", "Сверхзвуковой", 5u, 5u, 5u, 4u, 5u },
    { 7u, "Рев", "Огромный", "Ударный", 4u, 5u, 4u, 3u, 5u },
    { 8u, "Бум", "Могучий", "Импульсный", 4u, 4u, 5u, 3u, 5u },
    { 9u, "Удар", "Мощный", "Кинетический", 4u, 4u, 5u, 3u, 5u },
    { 10u, "Треск", "Тяжеловесный", "Реактивный", 4u, 4u, 5u, 3u, 4u },
    { 11u, "Трещание", "Основательный", "Траекторный", 3u, 4u, 4u, 4u, 3u },
    { 12u, "Бах", "Монументальный", "Динамический", 5u, 5u, 5u, 4u, 5u },
    { 13u, "Грохотание", "Колоссальный", "Разгонный", 5u, 5u, 5u, 4u, 5u },
    { 14u, "Раскатистость", "Внушительный", "Ускоренный", 4u, 4u, 5u, 3u, 4u },
    { 15u, "Громогласие", "Габаритный", "Снарядоподобный", 5u, 5u, 4u, 4u, 4u },
    { 16u, "Громоподобие", "Тяжёлый", "Пробивной", 5u, 5u, 4u, 3u, 5u },
    { 17u, "Шум", "Дородный", "Разрушительный", 3u, 4u, 3u, 2u, 3u },
    { 18u, "Резонанс", "Крепкий", "Мощностной", 4u, 4u, 3u, 3u, 4u },
    { 19u, "Рокотание", "Масштабный", "Дальнобойный", 4u, 5u, 4u, 5u, 4u },
    { 20u, "Оглушительность", "Великолепный", "Гиперскоростной", 5u, 5u, 5u, 5u, 5u },
};

size_t pl_impact_catalog(const pl_impact_profile **out) {
    if (out == NULL) return 0u;
    *out = profiles;
    return PL_IMPACT_PROFILE_COUNT;
}

int pl_impact_find(unsigned id, const pl_impact_profile **out) {
    size_t i;
    if (out == NULL || id == 0u || id > PL_IMPACT_PROFILE_COUNT) return -1;
    for (i = 0u; i < PL_IMPACT_PROFILE_COUNT; ++i) {
        if (profiles[i].id == id) {
            *out = &profiles[i];
            return 0;
        }
    }
    return -1;
}

int pl_impact_find_term(const char *term, const pl_impact_profile **out) {
    size_t i;
    if (term == NULL || term[0] == '\0' || out == NULL) return -1;
    for (i = 0u; i < PL_IMPACT_PROFILE_COUNT; ++i) {
        if (strcmp(term, profiles[i].sound) == 0 ||
            strcmp(term, profiles[i].scale_word) == 0 ||
            strcmp(term, profiles[i].motion_word) == 0) {
            *out = &profiles[i];
            return 0;
        }
    }
    return -1;
}

const char *pl_impact_dimension_name(unsigned dimension) {
    static const char *const names[PL_IMPACT_DIMENSION_COUNT] = {
        "acoustic", "scale", "kinetic", "reach", "impulse"
    };
    if (dimension >= PL_IMPACT_DIMENSION_COUNT) return "invalid";
    return names[dimension];
}

unsigned pl_impact_profile_rank(const pl_impact_profile *profile, unsigned dimension) {
    if (profile == NULL) return 0u;
    switch (dimension) {
        case PL_IMPACT_ACOUSTIC: return profile->acoustic_rank;
        case PL_IMPACT_SCALE: return profile->scale_rank;
        case PL_IMPACT_KINETIC: return profile->kinetic_rank;
        case PL_IMPACT_REACH: return profile->reach_rank;
        case PL_IMPACT_IMPULSE: return profile->impulse_rank;
        default: return 0u;
    }
}

int pl_impact_profile_validate(const pl_impact_profile *profile) {
    unsigned dimension;
    if (profile == NULL || profile->id == 0u ||
        profile->id > PL_IMPACT_PROFILE_COUNT ||
        profile->sound == NULL || profile->sound[0] == '\0' ||
        profile->scale_word == NULL || profile->scale_word[0] == '\0' ||
        profile->motion_word == NULL || profile->motion_word[0] == '\0') return -1;
    for (dimension = 0u; dimension < PL_IMPACT_DIMENSION_COUNT; ++dimension) {
        unsigned rank = pl_impact_profile_rank(profile, dimension);
        if (rank < 1u || rank > 5u) return -1;
    }
    return 0;
}
