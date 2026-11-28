#include "mos_switch.h"

#include <stdio.h>
#include <string.h>

static int parse_logic(const char *text, pl_mos_logic *out)
{
    if (text == NULL || out == NULL) {
        return 0;
    }

    if (strcmp(text, "low") == 0) {
        *out = PL_MOS_LOGIC_LOW;
    } else if (strcmp(text, "high") == 0) {
        *out = PL_MOS_LOGIC_HIGH;
    } else if (strcmp(text, "unknown") == 0) {
        *out = PL_MOS_LOGIC_UNKNOWN;
    } else {
        return 0;
    }

    return 1;
}

static int run_inverter(const char *input_text)
{
    pl_mos_switch_network network;
    pl_mos_logic input;
    pl_mos_logic levels[4] = {
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_UNKNOWN,
        PL_MOS_LOGIC_UNKNOWN
    };
    pl_mos_state states[4];

    if (!parse_logic(input_text, &input)) {
        fprintf(stderr, "Input must be low, high, or unknown.\n");
        return 2;
    }

    if (pl_mos_switch_network_init(&network, 4u, 0u, 1u) != PL_MOS_SWITCH_OK ||
        pl_mos_switch_add(&network, PL_MOS_CHANNEL_P, 2u, 0u, 3u) != PL_MOS_SWITCH_OK ||
        pl_mos_switch_add(&network, PL_MOS_CHANNEL_N, 2u, 3u, 1u) != PL_MOS_SWITCH_OK) {
        fprintf(stderr, "Could not construct the CMOS inverter model.\n");
        return 1;
    }

    levels[2] = input;
    if (pl_mos_switch_evaluate(&network, levels, 4u, states, 4u) != PL_MOS_SWITCH_OK) {
        fprintf(stderr, "Could not evaluate the CMOS inverter model.\n");
        return 1;
    }

    printf("model=%s input=%s output=%s\n",
           PL_MOS_SWITCH_SCHEMA, input_text,
           pl_mos_switch_state_name(states[3]));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 3 && strcmp(argv[1], "--inverter") == 0) {
        return run_inverter(argv[2]);
    }

    fprintf(stderr, "Usage: pytyliki-mos --inverter low|high|unknown\n");
    return 2;
}
