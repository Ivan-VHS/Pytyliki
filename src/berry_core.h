#ifndef PYTYLIKI_BERRY_CORE_H
#define PYTYLIKI_BERRY_CORE_H

#include <stddef.h>

#define PL_BERRY_CORE_NAME_MAX 24u
#define PL_BERRY_CORE_LABEL_MAX 64u
#define PL_BERRY_CORE_DETAIL_MAX 128u
#define PL_BERRY_CORE_COMMAND_COUNT 20u

typedef enum {
    PL_BERRY_MALINA = 0, PL_BERRY_YAGODKA, PL_BERRY_YAGODINA,
    PL_BERRY_KOSTYANIKA, PL_BERRY_EZHEVIKA, PL_BERRY_KUMANIKA,
    PL_BERRY_BRUSNIKA, PL_BERRY_KLYUKVA, PL_BERRY_ZEMLYANIKA,
    PL_BERRY_KLUBNIKA, PL_BERRY_CHERNIKA, PL_BERRY_GOLUBIKA,
    PL_BERRY_SMORODINA, PL_BERRY_KRYZHOVNIK, PL_BERRY_MOROSHKA,
    PL_BERRY_SHELKOVITSA, PL_BERRY_BOYARYSHNIK, PL_BERRY_BARBARIS,
    PL_BERRY_KNYAZHENIKA, PL_BERRY_VIKTORIA
} pl_berry_core_id;

typedef struct {
    pl_berry_core_id id;
    const char *name;
    const char *virtue;
    const char *operation;
    const char *description;
} pl_berry_core_command;

typedef struct {
    pl_berry_core_id command;
    unsigned target_logical_core;
} pl_berry_core_request;

typedef struct {
    int accepted;
    int planned_only;
    int hardware_modified;
    unsigned target_logical_core;
    char command_name[PL_BERRY_CORE_NAME_MAX];
    char operation[PL_BERRY_CORE_LABEL_MAX];
    char detail[PL_BERRY_CORE_DETAIL_MAX];
} pl_berry_core_result;

/* Returns the immutable 20-command catalogue. */
size_t pl_berry_core_catalog(const pl_berry_core_command **commands);
/* Validates a request against the detected logical-processor count.
 * This is a dry-run command planner: it never sends IPIs, changes affinity,
 * touches registers, or modifies firmware/clock/power settings. */
int pl_berry_core_dispatch(const pl_berry_core_request *request,
                           pl_berry_core_result *result);

#endif
