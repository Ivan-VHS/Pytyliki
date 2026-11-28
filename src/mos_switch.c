#include "mos_switch.h"

#include <string.h>

static int network_is_valid(const pl_mos_switch_network *network)
{
    size_t i;

    if (network == NULL || network->node_count == 0u ||
        network->node_count > PL_MOS_SWITCH_MAX_NODES ||
        network->device_count > PL_MOS_SWITCH_MAX_DEVICES ||
        network->vdd_node >= network->node_count ||
        network->vss_node >= network->node_count ||
        network->vdd_node == network->vss_node) {
        return 0;
    }

    for (i = 0u; i < network->device_count; ++i) {
        const pl_mos_switch_device *device = &network->devices[i];

        if ((unsigned)device->channel > (unsigned)PL_MOS_CHANNEL_P ||
            device->gate >= network->node_count ||
            device->terminal_a >= network->node_count ||
            device->terminal_b >= network->node_count ||
            device->terminal_a == device->terminal_b) {
            return 0;
        }
    }

    return 1;
}

int pl_mos_switch_network_init(pl_mos_switch_network *network,
                               size_t node_count,
                               size_t vdd_node,
                               size_t vss_node)
{
    if (network == NULL || node_count == 0u ||
        node_count > PL_MOS_SWITCH_MAX_NODES ||
        vdd_node >= node_count || vss_node >= node_count ||
        vdd_node == vss_node) {
        return PL_MOS_SWITCH_INVALID_ARGUMENT;
    }

    memset(network, 0, sizeof(*network));
    network->node_count = (uint16_t)node_count;
    network->vdd_node = (uint16_t)vdd_node;
    network->vss_node = (uint16_t)vss_node;
    return PL_MOS_SWITCH_OK;
}

int pl_mos_switch_add(pl_mos_switch_network *network,
                      pl_mos_channel channel,
                      size_t gate_node,
                      size_t terminal_a,
                      size_t terminal_b)
{
    pl_mos_switch_device *device;

    if (network == NULL) {
        return PL_MOS_SWITCH_INVALID_ARGUMENT;
    }
    if (!network_is_valid(network)) {
        return PL_MOS_SWITCH_INVALID_TOPOLOGY;
    }
    if ((unsigned)channel > (unsigned)PL_MOS_CHANNEL_P ||
        gate_node >= network->node_count ||
        terminal_a >= network->node_count ||
        terminal_b >= network->node_count ||
        terminal_a == terminal_b) {
        return PL_MOS_SWITCH_INVALID_ARGUMENT;
    }

    if (network->device_count >= PL_MOS_SWITCH_MAX_DEVICES) {
        return PL_MOS_SWITCH_CAPACITY;
    }

    device = &network->devices[network->device_count];
    device->gate = (uint16_t)gate_node;
    device->terminal_a = (uint16_t)terminal_a;
    device->terminal_b = (uint16_t)terminal_b;
    device->channel = channel;
    ++network->device_count;
    return PL_MOS_SWITCH_OK;
}

static int device_is_on(const pl_mos_switch_device *device,
                        const pl_mos_logic *gate_levels,
                        int include_unknown)
{
    pl_mos_logic level = gate_levels[device->gate];

    if (level == PL_MOS_LOGIC_UNKNOWN) {
        return include_unknown;
    }

    if (device->channel == PL_MOS_CHANNEL_N) {
        return level == PL_MOS_LOGIC_HIGH;
    }

    return level == PL_MOS_LOGIC_LOW;
}

static void mark_reachable(const pl_mos_switch_network *network,
                           const pl_mos_logic *gate_levels,
                           size_t start_node,
                           int include_unknown,
                           unsigned char *visited)
{
    uint16_t queue[PL_MOS_SWITCH_MAX_NODES];
    size_t head = 0u;
    size_t tail = 0u;

    memset(visited, 0, PL_MOS_SWITCH_MAX_NODES * sizeof(visited[0]));
    visited[start_node] = 1u;
    queue[tail++] = (uint16_t)start_node;

    while (head < tail) {
        size_t current = queue[head++];
        size_t i;

        for (i = 0u; i < network->device_count; ++i) {
            const pl_mos_switch_device *device = &network->devices[i];
            size_t next_node;

            if (!device_is_on(device, gate_levels, include_unknown)) {
                continue;
            }

            if (device->terminal_a == current) {
                next_node = device->terminal_b;
            } else if (device->terminal_b == current) {
                next_node = device->terminal_a;
            } else {
                continue;
            }

            if (visited[next_node] == 0u) {
                visited[next_node] = 1u;
                queue[tail++] = (uint16_t)next_node;
            }
        }
    }
}

int pl_mos_switch_evaluate(const pl_mos_switch_network *network,
                           const pl_mos_logic *gate_levels,
                           size_t gate_level_count,
                           pl_mos_state *states,
                           size_t state_capacity)
{
    unsigned char definite_high[PL_MOS_SWITCH_MAX_NODES];
    unsigned char definite_low[PL_MOS_SWITCH_MAX_NODES];
    unsigned char possible_high[PL_MOS_SWITCH_MAX_NODES];
    unsigned char possible_low[PL_MOS_SWITCH_MAX_NODES];
    size_t i;

    if (network == NULL || gate_levels == NULL || states == NULL) {
        return PL_MOS_SWITCH_INVALID_ARGUMENT;
    }
    if (!network_is_valid(network)) {
        return PL_MOS_SWITCH_INVALID_TOPOLOGY;
    }
    if (gate_level_count < network->node_count ||
        state_capacity < network->node_count) {
        return PL_MOS_SWITCH_INVALID_ARGUMENT;
    }

    for (i = 0u; i < network->node_count; ++i) {
        if ((unsigned)gate_levels[i] > (unsigned)PL_MOS_LOGIC_UNKNOWN) {
            return PL_MOS_SWITCH_INVALID_ARGUMENT;
        }
    }

    mark_reachable(network, gate_levels, network->vdd_node, 0,
                   definite_high);
    mark_reachable(network, gate_levels, network->vss_node, 0,
                   definite_low);
    mark_reachable(network, gate_levels, network->vdd_node, 1,
                   possible_high);
    mark_reachable(network, gate_levels, network->vss_node, 1,
                   possible_low);

    for (i = 0u; i < network->node_count; ++i) {
        if (definite_high[i] != 0u && definite_low[i] != 0u) {
            states[i] = PL_MOS_STATE_CONTENDED;
        } else if (definite_high[i] != 0u && possible_low[i] == 0u) {
            states[i] = PL_MOS_STATE_HIGH;
        } else if (definite_low[i] != 0u && possible_high[i] == 0u) {
            states[i] = PL_MOS_STATE_LOW;
        } else if (possible_high[i] != 0u || possible_low[i] != 0u) {
            states[i] = PL_MOS_STATE_INDETERMINATE;
        } else {
            states[i] = PL_MOS_STATE_FLOATING;
        }
    }

    return PL_MOS_SWITCH_OK;
}

const char *pl_mos_switch_state_name(pl_mos_state state)
{
    switch (state) {
    case PL_MOS_STATE_FLOATING:
        return "floating";
    case PL_MOS_STATE_LOW:
        return "low";
    case PL_MOS_STATE_HIGH:
        return "high";
    case PL_MOS_STATE_CONTENDED:
        return "contended";
    case PL_MOS_STATE_INDETERMINATE:
        return "indeterminate";
    default:
        return "invalid";
    }
}

const char *pl_mos_switch_channel_name(pl_mos_channel channel)
{
    switch (channel) {
    case PL_MOS_CHANNEL_N:
        return "nmos";
    case PL_MOS_CHANNEL_P:
        return "pmos";
    default:
        return "invalid";
    }
}
