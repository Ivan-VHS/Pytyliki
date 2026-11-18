#include "ironclad_systems.h"

static const pl_system_enhancement systems[PL_SYSTEM_ENHANCEMENT_COUNT] = {
    { 1u, "Chrome Ironclad", PL_SYSTEM_RELIABILITY, "Define invariants and fail-safe boundaries for core operations.", "Invalid inputs fail closed; existing public behavior remains covered by tests." },
    { 2u, "Velocity Tempest", PL_SYSTEM_PERFORMANCE, "Require reproducible benchmarks before performance changes.", "Benchmark inputs, compiler, platform, and baseline are recorded." },
    { 3u, "Midnight Monolith", PL_SYSTEM_MODULARITY, "Keep subsystem interfaces explicit and dependencies acyclic.", "Core modules build independently through declared interfaces." },
    { 4u, "Raven Vortex", PL_SYSTEM_DIAGNOSTICS, "Produce actionable diagnostics with stable identifiers.", "Findings include source, severity, evidence, and remediation." },
    { 5u, "Circuit Cobalt", PL_SYSTEM_LOW_LEVEL, "Gate hardware-specific paths on detected capabilities.", "Portable fallback is always available and unsupported paths are not executed." },
    { 6u, "Thunder Phantom", PL_SYSTEM_ASYNC, "Bound asynchronous work and define cancellation ownership.", "Work has explicit limits, completion state, and cancellation semantics." },
    { 7u, "Phantom Radiance", PL_SYSTEM_API, "Keep CLI and library contracts predictable and documented.", "Options and public APIs have deterministic validation and help text." },
    { 8u, "Velvet Gravitas", PL_SYSTEM_USABILITY, "Make reports and workflows readable and consistent.", "Output is stable, concise, and usable without color." },
    { 9u, "Obsidian Inferno", PL_SYSTEM_RECOVERY, "Preserve state and report errors without unsafe retries.", "Failure paths are tested and never silently claim success." },
    { 10u, "Horizon Spectral", PL_SYSTEM_PORTABILITY, "Preserve Windows and Linux portability.", "Both supported CI targets compile and pass CTest." },
    { 11u, "Crimson Overdrive", PL_SYSTEM_COMPILATION, "Make compiler diagnostics and optimization flags explicit.", "Strict warning builds remain clean on supported toolchains." },
    { 12u, "Voltage Citadel", PL_SYSTEM_SECURITY, "Apply least privilege and bounded input handling.", "Authorization defaults to deny and untrusted input is bounded." },
    { 13u, "Shadow Nemesis", PL_SYSTEM_TESTING, "Exercise boundaries, regressions, and negative paths.", "Every new public contract has deterministic tests." },
    { 14u, "Mirage Aether", PL_SYSTEM_DISTRIBUTED, "Specify transport, ownership, and failure semantics before distribution.", "No distributed capability is advertised without an executable backend." },
    { 15u, "Titanium Riptide", PL_SYSTEM_MEMORY, "Make allocation ownership and size limits explicit.", "Resource limits are validated before allocation or iteration." },
    { 16u, "Eclipse Outlaw", PL_SYSTEM_EXTENSIBILITY, "Use versioned extension points with validation.", "Unknown extensions are rejected safely and ABI changes are documented." },
    { 17u, "Neutron Zenith", PL_SYSTEM_PROFILING, "Separate measured profiles from heuristic estimates.", "Reports identify measurement source and never invent speedups." },
    { 18u, "Scarlet Avalanche", PL_SYSTEM_AUTOMATION, "Automate reproducible build, test, and evidence collection.", "CI gates changes on Windows and Linux validation." },
    { 19u, "Quantum Paradox", PL_SYSTEM_EXPERIMENTAL, "Isolate experiments behind opt-in interfaces.", "Experimental paths cannot silently alter stable defaults." },
    { 20u, "Reactor Afterglow", PL_SYSTEM_OBSERVABILITY, "Expose health, progress, and failure state.", "Status output distinguishes measured, inferred, and unavailable data." }
};

size_t pl_system_enhancements(const pl_system_enhancement **out) {
    if (out == NULL) return 0u;
    *out = systems;
    return PL_SYSTEM_ENHANCEMENT_COUNT;
}

const char *pl_system_domain_name(pl_system_domain domain) {
    static const char *const names[] = {
        "reliability", "performance", "modularity", "diagnostics", "low-level",
        "async", "api", "usability", "recovery", "portability", "compilation",
        "security", "testing", "distributed", "memory", "extensibility",
        "profiling", "automation", "experimental", "observability"
    };
    if ((unsigned)domain >= PL_SYSTEM_ENHANCEMENT_COUNT) return "invalid";
    return names[(unsigned)domain];
}

int pl_system_enhancement_validate(const pl_system_enhancement *item) {
    if (item == NULL || item->id == 0u || item->id > PL_SYSTEM_ENHANCEMENT_COUNT ||
        item->name == NULL || item->name[0] == '\0' ||
        item->objective == NULL || item->objective[0] == '\0' ||
        item->acceptance == NULL || item->acceptance[0] == '\0' ||
        (unsigned)item->domain >= PL_SYSTEM_ENHANCEMENT_COUNT) return -1;
    return 0;
}
