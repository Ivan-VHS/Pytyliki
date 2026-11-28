#ifndef PYTYLIKI_MOS_SWITCH_H
#define PYTYLIKI_MOS_SWITCH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_MOS_SWITCH_SCHEMA "pytyliki.mos-switch-model.v1"
#define PL_MOS_SWITCH_MAX_NODES 128u
#define PL_MOS_SWITCH_MAX_DEVICES 256u

typedef enum {
    PL_MOS_CHANNEL_N = 0,
    PL_MOS_CHANNEL_P = 1
} pl_mos_channel;

typedef enum {
    PL_MOS_LOGIC_LOW = 0,
    PL_MOS_LOGIC_HIGH = 1,
    PL_MOS_LOGIC_UNKNOWN = 2
} pl_mos_logic;

typedef enum {
    PL_MOS_STATE_FLOATING = 0,
    PL_MOS_STATE_LOW,
    PL_MOS_STATE_HIGH,
    PL_MOS_STATE_CONTENDED,
    PL_MOS_STATE_INDETERMINATE
} pl_mos_state;

typedef enum {
    PL_MOS_SWITCH_OK = 0,
    PL_MOS_SWITCH_INVALID_ARGUMENT = -1,
    PL_MOS_SWITCH_CAPACITY = -2,
    PL_MOS_SWITCH_INVALID_TOPOLOGY = -3
} pl_mos_switch_result;

typedef struct {
    uint16_t gate;
    uint16_t terminal_a;
    uint16_t terminal_b;
    pl_mos_channel channel;
} pl_mos_switch_device;

typedef struct {
    uint16_t node_count;
    uint16_t device_count;
    uint16_t vdd_node;
    uint16_t vss_node;
    pl_mos_switch_device devices[PL_MOS_SWITCH_MAX_DEVICES];
} pl_mos_switch_network;

/*
 * Initializes a caller-owned, fixed-capacity MOS switch network.
 * VDD and VSS are distinct node indices inside [0, node_count).
 */
int pl_mos_switch_network_init(pl_mos_switch_network *network,
                               size_t node_count,
                               size_t vdd_node,
                               size_t vss_node);

/*
 * Adds an ideal N-channel or P-channel MOS switch.
 * Source/drain terminals are treated as electrically symmetric.
 */
int pl_mos_switch_add(pl_mos_switch_network *network,
                      pl_mos_channel channel,
                      size_t gate_node,
                      size_t terminal_a,
                      size_t terminal_b);

/*
 * Resolves rail connectivity from the supplied gate logic levels.
 * gate_levels and states each contain at least network->node_count elements.
 * No allocation, I/O, hardware access, or persistent mutation is performed.
 */
int pl_mos_switch_evaluate(const pl_mos_switch_network *network,
                           const pl_mos_logic *gate_levels,
                           size_t gate_level_count,
                           pl_mos_state *states,
                           size_t state_capacity);

const char *pl_mos_switch_state_name(pl_mos_state state);
const char *pl_mos_switch_channel_name(pl_mos_channel channel);

#ifdef __cplusplus
}
#endif

#endif
