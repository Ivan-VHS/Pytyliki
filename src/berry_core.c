#include "berry_core.h"
#include "processor.h"

#include <stdio.h>
#include <string.h>

static const pl_berry_core_command commands[PL_BERRY_CORE_COMMAND_COUNT] = {
    {PL_BERRY_MALINA, "Malina", "Heroism", "Core invocation", "Create a validated invocation plan for a logical processor."},
    {PL_BERRY_YAGODKA, "Yagodka", "Courage", "Core addressing", "Resolve a logical-processor target without changing affinity."},
    {PL_BERRY_YAGODINA, "Yagodina", "Valor", "Core query", "Prepare a read-only processor-target query."},
    {PL_BERRY_KOSTYANIKA, "Kostyanika", "Daring", "Core command", "Validate a command envelope and target index."},
    {PL_BERRY_EZHEVIKA, "Ezhevika", "Bravery", "Core instruction", "Describe a bounded instruction-dispatch request; do not execute inspected code."},
    {PL_BERRY_KUMANIKA, "Kumanika", "Fearlessness", "Core initialization", "Initialize request metadata and validate processor availability."},
    {PL_BERRY_BRUSNIKA, "Brusnika", "Feat", "Core activation", "Mark a target as eligible for a future approved workload."},
    {PL_BERRY_KLYUKVA, "Klyukva", "Selflessness", "Core wake-up", "Plan a wake-up notification without issuing an interrupt."},
    {PL_BERRY_ZEMLYANIKA, "Zemlyanika", "Valor", "Core synchronization", "Prepare a synchronization checkpoint for a workload."},
    {PL_BERRY_KLUBNIKA, "Klubnika", "Nobility", "Core management", "Produce a read-only management plan for a logical processor."},
    {PL_BERRY_CHERNIKA, "Chernika", "Resilience", "Core interaction", "Validate a caller-to-core interaction descriptor."},
    {PL_BERRY_GOLUBIKA, "Golubika", "Self-sacrifice", "Control handoff", "Prepare a cooperative ownership handoff descriptor."},
    {PL_BERRY_SMORODINA, "Smorodina", "Resolve", "Processor-core invocation", "Resolve and validate an explicit logical-core invocation."},
    {PL_BERRY_KRYZHOVNIK, "Kryzhovnik", "Determination", "Core dispatch", "Build a deterministic dispatch plan for a selected core."},
    {PL_BERRY_MOROSHKA, "Moroshka", "Fearlessness", "Core interrupt", "Describe an interrupt request; never send an interrupt."},
    {PL_BERRY_SHELKOVITSA, "Shelkovitsa", "Perseverance", "Core processing", "Prepare a bounded processing-workload descriptor."},
    {PL_BERRY_BOYARYSHNIK, "Boyaryshnik", "Duty", "Core communication", "Prepare a message envelope for a processor subsystem."},
    {PL_BERRY_BARBARIS, "Barbaris", "Valor", "Command routing", "Route a command descriptor to a validated logical-core index."},
    {PL_BERRY_KNYAZHENIKA, "Knyazhenika", "Heroism", "Instruction execution plan", "Validate an execution plan without executing arbitrary instructions."},
    {PL_BERRY_VIKTORIA, "Viktoria", "Generosity", "Direct core control plan", "Describe permitted software-level control; hardware control remains unavailable."}
};

size_t pl_berry_core_catalog(const pl_berry_core_command **out) {
    if (out == NULL) return 0u;
    *out = commands;
    return PL_BERRY_CORE_COMMAND_COUNT;
}

int pl_berry_core_dispatch(const pl_berry_core_request *request,
                           pl_berry_core_result *result) {
    pl_processor_snapshot snapshot;
    const pl_berry_core_command *command;
    if (request == NULL || result == NULL ||
        (unsigned)request->command >= PL_BERRY_CORE_COMMAND_COUNT) return -1;
    memset(result, 0, sizeof(*result));
    if (pl_processor_scan(&snapshot) != 0 || snapshot.base.logical_cpus == 0u)
        return -2;
    if (request->target_logical_core >= snapshot.base.logical_cpus) {
        (void)snprintf(result->detail, sizeof(result->detail),
                       "Target core is outside the detected range 0..%u.",
                       snapshot.base.logical_cpus - 1u);
        return -3;
    }
    command = &commands[(unsigned)request->command];
    result->accepted = 1;
    result->planned_only = 1;
    result->hardware_modified = 0;
    result->target_logical_core = request->target_logical_core;
    (void)snprintf(result->command_name, sizeof(result->command_name), "%s", command->name);
    (void)snprintf(result->operation, sizeof(result->operation), "%s", command->operation);
    (void)snprintf(result->detail, sizeof(result->detail), "%s", command->description);
    return 0;
}
