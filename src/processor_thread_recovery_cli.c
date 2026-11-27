#include "processor_thread_recovery_cli.h"

#include "processor_thread_recovery.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PL_TR_CLI_MAX_EVENTS 4096u
#define PL_TR_CLI_LINE_BYTES 256u
#define PL_TR_CLI_VALID_FLAGS (PL_TR_TASK_IDEMPOTENT | PL_TR_TASK_CHECKPOINTED | PL_TR_TASK_MANAGED_POOL)

static int parse_kind(const char *text, pl_tr_fault_kind *kind) {
    if (strcmp(text, "transient") == 0) *kind = PL_TR_FAULT_TRANSIENT;
    else if (strcmp(text, "corrected_hardware") == 0) *kind = PL_TR_FAULT_CORRECTED_HARDWARE;
    else if (strcmp(text, "uncorrected_hardware") == 0) *kind = PL_TR_FAULT_UNCORRECTED_HARDWARE;
    else if (strcmp(text, "watchdog") == 0) *kind = PL_TR_FAULT_WATCHDOG;
    else if (strcmp(text, "fatal") == 0) *kind = PL_TR_FAULT_FATAL;
    else return 0;
    return 1;
}

static char *trim_field(char *field) {
    char *end;
    while (*field == ' ' || *field == '\\t' || *field == '\\r') ++field;
    end = field + strlen(field);
    while (end > field &&
           (end[-1] == ' ' || end[-1] == '\\t' || end[-1] == '\\r' ||
            end[-1] == '\\n')) {
        *--end = '\\0';
    }
    return field;
}

static int parse_u64_field(const char *text, uint64_t maximum, uint64_t *out) {
    char *end = NULL;
    unsigned long long value;
    if (text == NULL || text[0] == '\\0' || out == NULL) return 0;
    if (text[0] == '+' || text[0] == '-' ||
        (text[0] < '0' || text[0] > '9')) return 0;
    errno = 0;
    value = strtoull(text, &end, 10);
    if (errno == ERANGE || end == text || end == NULL || *end != '\\0' ||
        value > maximum) return 0;
    *out = (uint64_t)value;
    return 1;
}

static int parse_event_line(char *line, pl_tr_observation *event, uint32_t *task_flags) {
    char *fields[6];
    char *cursor;
    uint64_t values[5] = {0u, 0u, 0u, 0u, 0u};
    size_t field_count = 0u;

    if (line == NULL || event == NULL || task_flags == NULL) return 0;
    cursor = line;
    while (field_count < 5u) {
        char *separator = strchr(cursor, ',');
        if (separator == NULL) return 0;
        *separator = '\\0';
        fields[field_count++] = cursor;
        cursor = separator + 1;
    }
    if (strchr(cursor, ',') != NULL) return 0;
    fields[field_count++] = cursor;
    if (field_count != 6u) return 0;
    for (size_t i = 0u; i < 6u; ++i) fields[i] = trim_field(fields[i]);

    if (!parse_u64_field(fields[0], UINT64_MAX, &values[0]) ||
        !parse_u64_field(fields[1], UINT64_MAX, &values[1]) ||
        !parse_u64_field(fields[2], UINT32_MAX, &values[2]) ||
        !parse_u64_field(fields[3], UINT32_MAX, &values[3]) ||
        !parse_u64_field(fields[5], PL_TR_CLI_VALID_FLAGS, &values[4]) ||
        values[0] == 0u || values[1] == 0u || values[3] == 0u) {
        return 0;
    }

    event->worker_id = values[0];
    event->timestamp_ns = values[1];
    event->cpu_id = (uint32_t)values[2];
    event->fault_code = (uint32_t)values[3];
    if (!parse_kind(fields[4], &event->kind)) return 0;
    *task_flags = (uint32_t)values[4];
    return 1;
}

static size_t find_series(const pl_tr_monitor *monitor,
                          const pl_tr_observation *event) {
    for (size_t i = 0u; i < PL_TR_MAX_SERIES; ++i) {
        const pl_tr_series *series = &monitor->series[i];
        if (series->used != 0u &&
            series->worker_id == event->worker_id &&
            series->fault_code == event->fault_code &&
            series->kind == event->kind) return i;
    }
    return PL_TR_MAX_SERIES;
}

static int report_monitor(const pl_tr_monitor *monitor,
                          const uint32_t *task_flags,
                          const uint8_t *flags_present,
                          size_t event_count,
                          const char *source) {
    pl_tr_policy policy;
    size_t series_count = 0u;
    pl_tr_policy_default(&policy);
    printf("Pytyliki PTFR v1 | source=%s | events=%zu\n", source, event_count);
    for (size_t i = 0u; i < PL_TR_MAX_SERIES; ++i) {
        const pl_tr_series *series = &monitor->series[i];
        pl_tr_analysis analysis;
        pl_tr_decision decision;
        if (series->used == 0u) continue;
        if (flags_present[i] == 0u ||
            pl_tr_analyze(monitor, series->worker_id, series->fault_code,
                          series->kind, &policy, &analysis) != PL_TR_OK ||
            pl_tr_plan_recovery(&analysis, task_flags[i], &policy,
                                &decision) != PL_TR_OK) {
            fputs("PTFR: could not analyze a validated event series\n", stderr);
            return 1;
        }
        ++series_count;
        printf("worker=%llu cpu=%u fault=%u kind=%s observations=%u "
               "intervals=%u period_ns=%llu jitter_ppm=%u periodicity=%s "
               "action=%s retry_budget=%u operator_review=%s "
               "applied=no host_cpu_state_change=no reason=\"%s\"\n",
               (unsigned long long)analysis.worker_id,
               analysis.cpu_id, analysis.fault_code,
               pl_tr_fault_kind_name(analysis.kind),
               analysis.observations, analysis.intervals,
               (unsigned long long)analysis.estimated_period_ns,
               analysis.jitter_ppm,
               pl_tr_periodicity_name(analysis.periodicity),
               pl_tr_action_name(decision.action),
               decision.retry_budget,
               decision.requires_operator_review ? "yes" : "no",
               decision.reason != NULL ? decision.reason : "unspecified");
    }
    printf("series=%zu execution=advisory hardware_writes=no\n", series_count);
    return series_count == 0u ? 1 : 0;
}

static int ingest_stream(FILE *input, const char *source) {
    pl_tr_monitor monitor;
    uint32_t flags[PL_TR_MAX_SERIES] = {0u};
    uint8_t flags_present[PL_TR_MAX_SERIES] = {0u};
    char line[PL_TR_CLI_LINE_BYTES];
    size_t line_number = 0u;
    size_t event_count = 0u;
    pl_tr_monitor_init(&monitor);

    while (fgets(line, sizeof(line), input) != NULL) {
        pl_tr_observation event;
        uint32_t event_flags = 0u;
        char *cursor = line;
        size_t index;
        ++line_number;
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r') ++cursor;
        if (*cursor == '\0' || *cursor == '\n' || *cursor == '#') continue;
        if (strchr(line, '\n') == NULL && !feof(input)) {
            int value;
            while ((value = fgetc(input)) != '\n' && value != EOF) { }
            fprintf(stderr, "%s:%zu: input line exceeds %u bytes\n",
                    source, line_number, PL_TR_CLI_LINE_BYTES - 1u);
            return 2;
        }
        if (event_count >= PL_TR_CLI_MAX_EVENTS) {
            fprintf(stderr, "%s: event limit (%u) exceeded\n",
                    source, PL_TR_CLI_MAX_EVENTS);
            return 2;
        }
        if (!parse_event_line(cursor, &event, &event_flags)) {
            fprintf(stderr, "%s:%zu: invalid CSV event record\n", source, line_number);
            return 2;
        }
        const int status = pl_tr_record(&monitor, &event);
        if (status != PL_TR_OK) {
            fprintf(stderr, "%s:%zu: PTFR rejected event (status=%d)\n",
                    source, line_number, status);
            return 2;
        }
        index = find_series(&monitor, &event);
        if (index >= PL_TR_MAX_SERIES) {
            fprintf(stderr, "%s:%zu: event series lookup failed\n", source, line_number);
            return 2;
        }
        if (flags_present[index] != 0u && flags[index] != event_flags) {
            fprintf(stderr, "%s:%zu: task flags changed inside one event series\n",
                    source, line_number);
            return 2;
        }
        flags[index] = event_flags;
        flags_present[index] = 1u;
        ++event_count;
    }
    if (ferror(input)) {
        fprintf(stderr, "%s: read error\n", source);
        return 2;
    }
    return report_monitor(&monitor, flags, flags_present, event_count, source);
}

int pl_thread_recovery_cli(const char *path) {
    FILE *input;
    int result;
    if (path == NULL || path[0] == '\0') {
        fputs("PTFR: an event CSV path is required\n", stderr);
        return 2;
    }
    input = fopen(path, "rb");
    if (input == NULL) {
        fprintf(stderr, "PTFR: cannot open %s\n", path);
        return 2;
    }
    result = ingest_stream(input, path);
    fclose(input);
    return result;
}

int pl_thread_recovery_demo_cli(void) {
    static const uint64_t timestamps[] = {
        UINT64_C(1000000000), UINT64_C(2000000000),
        UINT64_C(3000000000), UINT64_C(4000000000),
        UINT64_C(5000000000), UINT64_C(6000000000)
    };
    pl_tr_monitor monitor;
    uint32_t flags[PL_TR_MAX_SERIES] = {0u};
    uint8_t flags_present[PL_TR_MAX_SERIES] = {0u};
    pl_tr_observation event = {
        7u, 0u, 2u, 65u, PL_TR_FAULT_CORRECTED_HARDWARE
    };
    pl_tr_monitor_init(&monitor);
    for (size_t i = 0u; i < sizeof(timestamps) / sizeof(timestamps[0]); ++i) {
        event.timestamp_ns = timestamps[i];
        if (pl_tr_record(&monitor, &event) != PL_TR_OK) {
            fputs("PTFR: deterministic demonstration setup failed\n", stderr);
            return 1;
        }
    }
    const size_t index = find_series(&monitor, &event);
    if (index >= PL_TR_MAX_SERIES) return 1;
    flags[index] = PL_TR_TASK_MANAGED_POOL;
    flags_present[index] = 1u;
    return report_monitor(&monitor, flags, flags_present,
                          sizeof(timestamps) / sizeof(timestamps[0]),
                          "synthetic-demo");
}
