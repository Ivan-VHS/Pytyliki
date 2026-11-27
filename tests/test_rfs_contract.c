#include "interop.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <string.h>

static void copy_text(char *destination, size_t capacity, const char *source)
{
    size_t length = strlen(source);
    assert(length < capacity);
    memcpy(destination, source, length + 1u);
}

static pytyliki_rfs01_node_v1 make_node(
    const char *id, uint32_t layer, double pressure, double load_limit,
    double integrity, double noise_db, const char *seed)
{
    pytyliki_rfs01_node_v1 node;
    memset(&node, 0, sizeof node);
    node.struct_size = (uint32_t)sizeof node;
    node.abi_version = PYTYLIKI_RFS01_ABI_VERSION;
    copy_text(node.id, sizeof node.id, id);
    node.layer = layer;
    node.pressure = pressure;
    node.load_limit = load_limit;
    node.integrity = integrity;
    node.noise_db = noise_db;
    copy_text(node.core_seed, sizeof node.core_seed, seed);
    return node;
}

int main(void)
{
    pytyliki_rfs01_node_v1 source = make_node(
        "basement", PYTYLIKI_RFS01_LAYER_BASEMENT, 20.0, 100.0, 0.9, -18.0, "root-a");
    pytyliki_rfs01_node_v1 target = make_node(
        "corridor", PYTYLIKI_RFS01_LAYER_CORRIDOR, 2.0, 100.0, 0.8, -10.0, "root-b");
    pytyliki_rfs01_node_v1 output;
    pytyliki_rfs01_node_v1 before;
    pytyliki_rfs01_node_v1 restored;
    pytyliki_rfs01_link_v1 link;
    pytyliki_rfs01_signal_v1 signal;
    double reserve = -1.0;

    assert(pytyliki_rfs01_abi_version() == PYTYLIKI_RFS01_ABI_VERSION);
    assert(pytyliki_rfs01_validate_node_v1(&source) == PYTYLIKI_RFS01_OK);
    assert(pytyliki_rfs01_pressure_reserve_v1(&source, &reserve) == PYTYLIKI_RFS01_OK);
    assert(fabs(reserve - 0.72) < 1e-12);

    memset(&link, 0, sizeof link);
    link.struct_size = (uint32_t)sizeof link;
    link.abi_version = PYTYLIKI_RFS01_ABI_VERSION;
    copy_text(link.id, sizeof link.id, "bridge-1");
    copy_text(link.origin, sizeof link.origin, "basement");
    copy_text(link.target, sizeof link.target, "corridor");
    link.channel = PYTYLIKI_RFS01_CHANNEL_BRIDGE;
    link.weight = 0.1;
    assert(pytyliki_rfs01_validate_link_v1(&link) == PYTYLIKI_RFS01_OK);
    assert(pytyliki_rfs01_plan_pressure_forward_v1(&source, &target, &link, &output) == PYTYLIKI_RFS01_OK);
    assert(fabs(output.pressure - 4.0) < 1e-12 && output.active == 1);

    before = output;
    target.pressure = 99.0;
    assert(pytyliki_rfs01_plan_pressure_forward_v1(&source, &target, &link, &output) == PYTYLIKI_RFS01_LOAD_LIMIT);
    assert(memcmp(&before, &output, sizeof output) == 0);
    target.pressure = 2.0;

    link.blocked = 1;
    assert(pytyliki_rfs01_plan_pressure_forward_v1(&source, &target, &link, &output) == PYTYLIKI_RFS01_BLOCKED);
    link.blocked = 0;

    memset(&signal, 0, sizeof signal);
    signal.struct_size = (uint32_t)sizeof signal;
    signal.abi_version = PYTYLIKI_RFS01_ABI_VERSION;
    signal.sequence = 7u;
    copy_text(signal.origin, sizeof signal.origin, "basement");
    copy_text(signal.payload, sizeof signal.payload, "trusted pulse");
    signal.trusted = 1;
    signal.manufactured = 0;
    assert(pytyliki_rfs01_validate_signal_v1(&signal) == PYTYLIKI_RFS01_OK);

    /* Exactly 4 KiB of valid UTF-8 is accepted, including the terminator byte. */
    {
        static const unsigned char emoji[] = {0xf0u, 0x9fu, 0x8cu, 0x99u};
        size_t offset;
        for (offset = 0u; offset < PYTYLIKI_RFS01_PAYLOAD_CAP; offset += sizeof emoji) {
            memcpy(signal.payload + offset, emoji, sizeof emoji);
        }
        signal.payload[PYTYLIKI_RFS01_PAYLOAD_CAP] = '\0';
        assert(pytyliki_rfs01_validate_signal_v1(&signal) == PYTYLIKI_RFS01_OK);
        signal.payload[0] = (char)0xc0u; /* overlong UTF-8 lead byte */
        signal.payload[1] = '\0';
        assert(pytyliki_rfs01_validate_signal_v1(&signal) == PYTYLIKI_RFS01_INVALID_ARGUMENT);
        copy_text(signal.payload, sizeof signal.payload, "trusted pulse");
    }
    assert(pytyliki_rfs01_signal_is_clean_v1(&signal) == 1);
    assert(pytyliki_rfs01_plan_signal_forward_v1(&source, &target, &link, &signal, &output) == PYTYLIKI_RFS01_OK);
    assert(fabs(output.noise_db - (-14.436974992327127)) < 1e-12);
    assert(output.active == 1);

    before = output;
    signal.manufactured = 1;
    assert(pytyliki_rfs01_signal_is_clean_v1(&signal) == 0);
    assert(pytyliki_rfs01_plan_signal_forward_v1(&source, &target, &link, &signal, &output) == PYTYLIKI_RFS01_SIGNAL_REJECTED);
    assert(memcmp(&before, &output, sizeof output) == 0);
    signal.manufactured = 0;

    assert(pytyliki_rfs01_core_restore_v1(&output, &target, &restored) == PYTYLIKI_RFS01_OK);
    assert(strcmp(restored.core_seed, "root-b") == 0);
    assert(restored.pressure == output.pressure);
    assert(restored.noise_db == output.noise_db);

    assert(pytyliki_rfs01_root_restore_v1(&target, &restored) == PYTYLIKI_RFS01_OK);
    assert(strcmp(restored.id, target.id) == 0);
    assert(restored.layer == target.layer);
    assert(restored.pressure == target.pressure);
    assert(restored.load_limit == target.load_limit);
    assert(restored.integrity == target.integrity);
    assert(restored.noise_db == target.noise_db);
    assert(strcmp(restored.core_seed, target.core_seed) == 0);
    assert(restored.active == target.active);

    source.integrity = 1.5;
    assert(pytyliki_rfs01_validate_node_v1(&source) == PYTYLIKI_RFS01_INVALID_ARGUMENT);
    source.integrity = 0.9;
    source.abi_version = 99u;
    assert(pytyliki_rfs01_validate_node_v1(&source) == PYTYLIKI_RFS01_UNSUPPORTED_ABI);
    assert(pytyliki_rfs01_validate_node_v1(NULL) == PYTYLIKI_RFS01_INVALID_ARGUMENT);
    assert(pytyliki_rfs01_pressure_reserve_v1(&source, NULL) == PYTYLIKI_RFS01_INVALID_ARGUMENT);
    return 0;
}
