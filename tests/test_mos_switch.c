#include "mos_switch.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_cmos_inverter(void)
{
    pl_mos_switch_network network;
    pl_mos_logic levels[4] = {
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_LOW,
        PL_MOS_LOGIC_UNKNOWN
    };
    pl_mos_state states[4];

    assert(pl_mos_switch_network_init(&network, 4u, 0u, 1u) == PL_MOS_SWITCH_OK);
    assert(pl_mos_switch_add(&network, PL_MOS_CHANNEL_P, 2u, 0u, 3u) == PL_MOS_SWITCH_OK);
    assert(pl_mos_switch_add(&network, PL_MOS_CHANNEL_N, 2u, 3u, 1u) == PL_MOS_SWITCH_OK);
    assert(pl_mos_switch_evaluate(&network, levels, 4u, states, 4u) == PL_MOS_SWITCH_OK);
    assert(states[0] == PL_MOS_STATE_HIGH);
    assert(states[1] == PL_MOS_STATE_LOW);
    assert(states[3] == PL_MOS_STATE_HIGH);

    levels[2] = PL_MOS_LOGIC_HIGH;
    assert(pl_mos_switch_evaluate(&network, levels, 4u, states, 4u) == PL_MOS_SWITCH_OK);
    assert(states[3] == PL_MOS_STATE_LOW);

    levels[2] = PL_MOS_LOGIC_UNKNOWN;
    assert(pl_mos_switch_evaluate(&network, levels, 4u, states, 4u) == PL_MOS_SWITCH_OK);
    assert(states[3] == PL_MOS_STATE_INDETERMINATE);
}

static void test_cmos_nand2_truth_table(void)
{
    pl_mos_switch_network network;
    pl_mos_logic levels[6] = {
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_LOW,
        PL_MOS_LOGIC_LOW,
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_UNKNOWN
    };
    pl_mos_state states[6];

    assert(pl_mos_switch_network_init(&network, 6u, 0u, 1u) == PL_MOS_SWITCH_OK);

    /* Parallel P-MOS pull-up network. */
    assert(pl_mos_switch_add(&network, PL_MOS_CHANNEL_P, 2u, 0u, 5u) == PL_MOS_SWITCH_OK);
    assert(pl_mos_switch_add(&network, PL_MOS_CHANNEL_P, 3u, 0u, 5u) == PL_MOS_SWITCH_OK);

    /* Series N-MOS pull-down network. */
    assert(pl_mos_switch_add(&network, PL_MOS_CHANNEL_N, 2u, 5u, 4u) == PL_MOS_SWITCH_OK);
    assert(pl_mos_switch_add(&network, PL_MOS_CHANNEL_N, 3u, 4u, 1u) == PL_MOS_SWITCH_OK);

    assert(pl_mos_switch_evaluate(&network, levels, 6u, states, 6u) == PL_MOS_SWITCH_OK);
    assert(states[5] == PL_MOS_STATE_HIGH);

    levels[2] = PL_MOS_LOGIC_LOW;
    levels[3] = PL_MOS_LOGIC_HIGH;
    assert(pl_mos_switch_evaluate(&network, levels, 6u, states, 6u) == PL_MOS_SWITCH_OK);
    assert(states[5] == PL_MOS_STATE_HIGH);

    levels[2] = PL_MOS_LOGIC_HIGH;
    levels[3] = PL_MOS_LOGIC_LOW;
    assert(pl_mos_switch_evaluate(&network, levels, 6u, states, 6u) == PL_MOS_SWITCH_OK);
    assert(states[5] == PL_MOS_STATE_HIGH);

    levels[2] = PL_MOS_LOGIC_HIGH;
    levels[3] = PL_MOS_LOGIC_HIGH;
    assert(pl_mos_switch_evaluate(&network, levels, 6u, states, 6u) == PL_MOS_SWITCH_OK);
    assert(states[5] == PL_MOS_STATE_LOW);
}

static void test_contention_and_floating(void)
{
    pl_mos_switch_network network;
    pl_mos_logic levels[5] = {
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_LOW,
        PL_MOS_LOGIC_HIGH,
        PL_MOS_LOGIC_UNKNOWN
    };
    pl_mos_state states[5];

    assert(pl_mos_switch_network_init(&network, 5u, 0u, 1u) == PL_MOS_SWITCH_OK);
    assert(pl_mos_switch_add(&network, PL_MOS_CHANNEL_P, 2u, 0u, 3u) == PL_MOS_SWITCH_OK);
    assert(pl_mos_switch_add(&network, PL_MOS_CHANNEL_N, 3u, 3u, 1u) == PL_MOS_SWITCH_OK);
    assert(pl_mos_switch_evaluate(&network, levels, 5u, states, 5u) == PL_MOS_SWITCH_OK);
    assert(states[3] == PL_MOS_STATE_CONTENDED);
    assert(states[4] == PL_MOS_STATE_FLOATING);
}

static void test_invalid_inputs(void)
{
    pl_mos_switch_network network;
    pl_mos_logic levels[3] = {
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_LOW
    };
    pl_mos_state states[3] = {
        PL_MOS_STATE_HIGH,
        PL_MOS_STATE_HIGH,
        PL_MOS_STATE_HIGH
    };

    assert(pl_mos_switch_network_init(NULL, 3u, 0u, 1u) == PL_MOS_SWITCH_INVALID_ARGUMENT);
    assert(pl_mos_switch_network_init(&network, 0u, 0u, 1u) == PL_MOS_SWITCH_INVALID_ARGUMENT);
    assert(pl_mos_switch_network_init(&network, PL_MOS_SWITCH_MAX_NODES + 1u,
                                      0u, 1u) == PL_MOS_SWITCH_INVALID_ARGUMENT);
    assert(pl_mos_switch_network_init(&network, 3u, 0u, 0u) == PL_MOS_SWITCH_INVALID_ARGUMENT);

    assert(pl_mos_switch_network_init(&network, 3u, 0u, 1u) == PL_MOS_SWITCH_OK);
    assert(pl_mos_switch_add(&network, PL_MOS_CHANNEL_N, 3u, 1u, 2u) ==
           PL_MOS_SWITCH_INVALID_ARGUMENT);
    assert(pl_mos_switch_add(&network, (pl_mos_channel)9, 2u, 1u, 2u) ==
           PL_MOS_SWITCH_INVALID_ARGUMENT);
    assert(pl_mos_switch_add(&network, PL_MOS_CHANNEL_N, 2u, 1u, 1u) ==
           PL_MOS_SWITCH_INVALID_ARGUMENT);

    assert(pl_mos_switch_evaluate(&network, levels, 2u, states, 3u) ==
           PL_MOS_SWITCH_INVALID_ARGUMENT);
    assert(states[0] == PL_MOS_STATE_HIGH);
    levels[2] = (pl_mos_logic)7;
    assert(pl_mos_switch_evaluate(&network, levels, 3u, states, 3u) ==
           PL_MOS_SWITCH_INVALID_ARGUMENT);
    assert(states[0] == PL_MOS_STATE_HIGH);
}

int main(void)
{
    test_cmos_inverter();
    test_cmos_nand2_truth_table();
    test_contention_and_floating();
    test_invalid_inputs();
    puts("Pytyliki MOS switch model tests passed.");
    return 0;
}
