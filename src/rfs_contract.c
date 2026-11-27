#include "interop.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static int utf8_bytes_valid(const unsigned char *value, size_t length)
{
    size_t index = 0u;
    while (index < length) {
        unsigned char first = value[index];
        if (first <= 0x7fu) {
            ++index;
        } else if (first >= 0xc2u && first <= 0xdfu) {
            if (index + 1u >= length ||
                (value[index + 1u] & 0xc0u) != 0x80u) return 0;
            index += 2u;
        } else if (first >= 0xe0u && first <= 0xefu) {
            unsigned char second;
            if (index + 2u >= length) return 0;
            second = value[index + 1u];
            if ((second & 0xc0u) != 0x80u ||
                (value[index + 2u] & 0xc0u) != 0x80u) return 0;
            if ((first == 0xe0u && second < 0xa0u) ||
                (first == 0xedu && second > 0x9fu)) return 0;
            index += 3u;
        } else if (first >= 0xf0u && first <= 0xf4u) {
            unsigned char second;
            if (index + 3u >= length) return 0;
            second = value[index + 1u];
            if ((second & 0xc0u) != 0x80u ||
                (value[index + 2u] & 0xc0u) != 0x80u ||
                (value[index + 3u] & 0xc0u) != 0x80u) return 0;
            if ((first == 0xf0u && second < 0x90u) ||
                (first == 0xf4u && second > 0x8fu)) return 0;
            index += 4u;
        } else {
            return 0;
        }
    }
    return 1;
}

static int text_field_valid(const char *value, size_t capacity, int allow_empty)
{
    size_t index;
    if (value == NULL || capacity == 0u) return 0;
    for (index = 0u; index < capacity; ++index) {
        if (value[index] == '\0') {
            return (allow_empty || index != 0u) &&
                   utf8_bytes_valid((const unsigned char *)value, index);
        }
    }
    return 0;
}

static int32_t check_header(uint32_t struct_size, uint32_t abi_version, size_t expected_size)
{
    if (abi_version != PYTYLIKI_RFS01_ABI_VERSION)
        return PYTYLIKI_RFS01_UNSUPPORTED_ABI;
    if ((size_t)struct_size < expected_size)
        return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    return PYTYLIKI_RFS01_OK;
}

uint32_t pytyliki_rfs01_abi_version(void)
{
    return PYTYLIKI_RFS01_ABI_VERSION;
}

int32_t pytyliki_rfs01_validate_node_v1(const pytyliki_rfs01_node_v1 *node)
{
    int32_t status;
    if (node == NULL) return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    status = check_header(node->struct_size, node->abi_version, sizeof *node);
    if (status != PYTYLIKI_RFS01_OK) return status;
    if (!text_field_valid(node->id, sizeof node->id, 0) ||
        !text_field_valid(node->core_seed, sizeof node->core_seed, 0))
        return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    if (node->layer < PYTYLIKI_RFS01_LAYER_BASEMENT ||
        node->layer > PYTYLIKI_RFS01_LAYER_HORIZON ||
        !isfinite(node->pressure) || node->pressure < 0.0 ||
        !isfinite(node->load_limit) || node->load_limit <= 0.0 ||
        !isfinite(node->integrity) || node->integrity < 0.0 || node->integrity > 1.0 ||
        !isfinite(node->noise_db) || node->noise_db < -160.0 || node->noise_db > 24.0 ||
        (node->active != 0 && node->active != 1))
        return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    return PYTYLIKI_RFS01_OK;
}

int32_t pytyliki_rfs01_validate_link_v1(const pytyliki_rfs01_link_v1 *link)
{
    int32_t status;
    if (link == NULL) return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    status = check_header(link->struct_size, link->abi_version, sizeof *link);
    if (status != PYTYLIKI_RFS01_OK) return status;
    if (!text_field_valid(link->id, sizeof link->id, 0) ||
        !text_field_valid(link->origin, sizeof link->origin, 0) ||
        !text_field_valid(link->target, sizeof link->target, 0))
        return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    if (link->channel < PYTYLIKI_RFS01_CHANNEL_WIRE ||
        link->channel > PYTYLIKI_RFS01_CHANNEL_BRIDGE ||
        (link->blocked != 0 && link->blocked != 1) ||
        !isfinite(link->weight) || link->weight < 0.0)
        return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    return PYTYLIKI_RFS01_OK;
}

int32_t pytyliki_rfs01_validate_signal_v1(const pytyliki_rfs01_signal_v1 *signal)
{
    int32_t status;
    if (signal == NULL) return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    status = check_header(signal->struct_size, signal->abi_version, sizeof *signal);
    if (status != PYTYLIKI_RFS01_OK) return status;
    if (!text_field_valid(signal->origin, sizeof signal->origin, 0) ||
        !text_field_valid(signal->payload, sizeof signal->payload, 1) ||
        (signal->trusted != 0 && signal->trusted != 1) ||
        (signal->manufactured != 0 && signal->manufactured != 1))
        return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    return PYTYLIKI_RFS01_OK;
}

/* Returns 1 for a clean frame, 0 for a valid but rejected frame, or a negative error. */
int32_t pytyliki_rfs01_signal_is_clean_v1(const pytyliki_rfs01_signal_v1 *signal)
{
    int32_t status = pytyliki_rfs01_validate_signal_v1(signal);
    if (status != PYTYLIKI_RFS01_OK) return status;
    return signal->trusted == 1 && signal->manufactured == 0 ? 1 : 0;
}

int32_t pytyliki_rfs01_pressure_reserve_v1(
    const pytyliki_rfs01_node_v1 *node, double *reserve_out)
{
    double ratio;
    double result;
    int32_t status;
    if (reserve_out == NULL) return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    status = pytyliki_rfs01_validate_node_v1(node);
    if (status != PYTYLIKI_RFS01_OK) return status;
    ratio = node->pressure / node->load_limit;
    if (ratio > 1.0) ratio = 1.0;
    result = node->integrity * (1.0 - ratio);
    *reserve_out = result;
    return PYTYLIKI_RFS01_OK;
}

int32_t pytyliki_rfs01_plan_pressure_forward_v1(
    const pytyliki_rfs01_node_v1 *source,
    const pytyliki_rfs01_node_v1 *target,
    const pytyliki_rfs01_link_v1 *link,
    pytyliki_rfs01_node_v1 *planned_target)
{
    pytyliki_rfs01_node_v1 next;
    double pressure;
    int32_t status;
    if (planned_target == NULL) return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    status = pytyliki_rfs01_validate_node_v1(source);
    if (status != PYTYLIKI_RFS01_OK) return status;
    status = pytyliki_rfs01_validate_node_v1(target);
    if (status != PYTYLIKI_RFS01_OK) return status;
    status = pytyliki_rfs01_validate_link_v1(link);
    if (status != PYTYLIKI_RFS01_OK) return status;
    if (link->blocked != 0 ||
        strcmp(link->origin, source->id) != 0 ||
        strcmp(link->target, target->id) != 0)
        return PYTYLIKI_RFS01_BLOCKED;

    pressure = target->pressure + source->pressure * link->weight;
    if (!isfinite(pressure) || pressure > target->load_limit)
        return PYTYLIKI_RFS01_LOAD_LIMIT;
    next = *target;
    next.pressure = pressure;
    next.active = 1;
    *planned_target = next;
    return PYTYLIKI_RFS01_OK;
}

int32_t pytyliki_rfs01_plan_signal_forward_v1(
    const pytyliki_rfs01_node_v1 *source,
    const pytyliki_rfs01_node_v1 *target,
    const pytyliki_rfs01_link_v1 *link,
    const pytyliki_rfs01_signal_v1 *signal,
    pytyliki_rfs01_node_v1 *planned_target)
{
    pytyliki_rfs01_node_v1 next;
    int32_t status;
    int32_t clean;
    if (planned_target == NULL) return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    status = pytyliki_rfs01_validate_node_v1(source);
    if (status != PYTYLIKI_RFS01_OK) return status;
    status = pytyliki_rfs01_validate_node_v1(target);
    if (status != PYTYLIKI_RFS01_OK) return status;
    status = pytyliki_rfs01_validate_link_v1(link);
    if (status != PYTYLIKI_RFS01_OK) return status;
    status = pytyliki_rfs01_validate_signal_v1(signal);
    if (status != PYTYLIKI_RFS01_OK) return status;
    if (link->blocked != 0 ||
        strcmp(link->origin, source->id) != 0 ||
        strcmp(link->target, target->id) != 0)
        return PYTYLIKI_RFS01_BLOCKED;
    if (strcmp(signal->origin, source->id) != 0)
        return PYTYLIKI_RFS01_SIGNAL_REJECTED;
    clean = pytyliki_rfs01_signal_is_clean_v1(signal);
    if (clean < 0) return clean;
    if (clean == 0) return PYTYLIKI_RFS01_SIGNAL_REJECTED;

    next = *target;
    next.noise_db -= 4.436974992327127;
    if (next.noise_db < -160.0) next.noise_db = -160.0;
    next.active = 1;
    *planned_target = next;
    return PYTYLIKI_RFS01_OK;
}

int32_t pytyliki_rfs01_root_restore_v1(
    const pytyliki_rfs01_node_v1 *root_snapshot,
    pytyliki_rfs01_node_v1 *restored)
{
    int32_t status;
    if (restored == NULL) return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    status = pytyliki_rfs01_validate_node_v1(root_snapshot);
    if (status != PYTYLIKI_RFS01_OK) return status;
    *restored = *root_snapshot;
    return PYTYLIKI_RFS01_OK;
}

int32_t pytyliki_rfs01_core_restore_v1(
    const pytyliki_rfs01_node_v1 *current,
    const pytyliki_rfs01_node_v1 *root_snapshot,
    pytyliki_rfs01_node_v1 *restored)
{
    pytyliki_rfs01_node_v1 next;
    int32_t status;
    if (restored == NULL) return PYTYLIKI_RFS01_INVALID_ARGUMENT;
    status = pytyliki_rfs01_validate_node_v1(current);
    if (status != PYTYLIKI_RFS01_OK) return status;
    status = pytyliki_rfs01_validate_node_v1(root_snapshot);
    if (status != PYTYLIKI_RFS01_OK) return status;
    next = *current;
    memcpy(next.core_seed, root_snapshot->core_seed, sizeof next.core_seed);
    *restored = next;
    return PYTYLIKI_RFS01_OK;
}
