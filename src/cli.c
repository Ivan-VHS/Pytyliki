#include "core.h"
#include "processor.h"
#include "dsp.h"
#include "improvement.h"
#include "hw_rules.h"
#include "processor_catalog.h"
#include "pipeline.h"
#include "policy.h"
#include "evolution.h"
#include "syntax_pie.h"
#include "berry_core.h"
#include "ironclad_systems.h"
#include "architecture_atlas.h"
#include "systems_accel.h"
#include "trimatrix_autoforge.h"
#include "embedded.h"
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static void usage(void) {
    puts("Pytyliki Processor Systems Workstation 0.7.0\n"
         "Usage:\n"
         "  pytyliki [--profile] [--capabilities] [--benchmark] [--suite] [--e2k-profile]\n"
         "           [--systems] [--systems-lab] [--architectures] [--architecture NAME] [--improvement-plan] [--trimatrix [DEPTH]] [--syntax-pie] [--hw-rules] [--pipeline] [--processor-catalog] [--policy-check ROLE ACTION] [--evolve] [--evolution-rounds N] [--review-processor FILE] (alias: --analyze FILE) [--json FILE]\n"
         "  pytyliki --tone-out FILE.wav [--frequency HZ] [--seconds N]\n"
         "  pytyliki --audio-info FILE.wav\n"
         "  pytyliki --process-wav INPUT.wav OUTPUT.wav [--lowpass HZ]\n"
         "  pytyliki --embedded profiles|profile ID|budget ID TEXT RODATA DATA BSS STACK|template FILE.c\n"
         "           [--compress THRESHOLD_DB RATIO] [--saturate DRIVE] [--gain DB]\n"
         "Processor inspection is read-only. Execution and DSP datapath workloads are bounded.");
}
static void print_berry_core_catalog(void) {
    const pl_berry_core_command *items = NULL;
    size_t count = pl_berry_core_catalog(&items);
    puts("Berry processor-core command catalogue (dry-run planner):");
    for (size_t i = 0u; i < count; ++i)
        printf("%02u | %-12s | %-18s | %-28s | %s\n",
               (unsigned)i + 1u, items[i].name, items[i].virtue,
               items[i].operation, items[i].description);
    puts("No hardware interrupt, register write, affinity change, or firmware operation is performed.");
}
static int dispatch_berry_core(const char *name, unsigned core) {
    const pl_berry_core_command *items = NULL;
    size_t count = pl_berry_core_catalog(&items);
    pl_berry_core_request request;
    pl_berry_core_result result;
    size_t i;
    for (i = 0u; i < count; ++i)
        if (strcmp(items[i].name, name) == 0) break;
    if (i == count) { fprintf(stderr, "Unknown berry command: %s\n", name); return 2; }
    request.command = items[i].id;
    request.target_logical_core = core;
    if (pl_berry_core_dispatch(&request, &result) != 0) {
        fprintf(stderr, "Core command rejected: %s\n", result.detail[0] ? result.detail : "invalid request");
        return 2;
    }
    printf("%s -> logical core %u | %s | PLANNED ONLY | hardware modified: no\n",
           result.command_name, result.target_logical_core, result.operation);
    return 0;
}
static int parse_double(const char *s,double *out) {
    char *end=NULL; errno=0; double v=strtod(s,&end); if(!s[0]||!end||end==s||*end||errno==ERANGE||!isfinite(v)) return -1; *out=v; return 0;
}
static void print_trimatrix(unsigned depth) {
    pl_tma_plan plan;
    pl_tma_step step;
    const pl_tma_cell *cells = NULL;
    size_t count = pl_tma_catalog(&cells);
    if (count != PL_TMA_CELL_COUNT || pl_tma_plan_init(&plan, depth) != 0) {
        fputs("TriMatrix depth must be 1..100.\n", stderr);
        return;
    }
    printf("TriMatrix AutoForge: 3 lines x 3 matrices x 3 stages | %u generations | %u steps\n",
           depth, pl_tma_plan_total(&plan));
    while (pl_tma_plan_next(&plan, &step) == 1)
        printf("%04u | generation %03u | %-8s | %-16s | %s\n",
               step.step, step.depth, pl_tma_line_name(step.cell.line),
               pl_tma_matrix_name(step.cell.matrix), step.cell.name);
}

static void print_improvement_plan(void) {
    const pl_improvement_capability *items = NULL;
    size_t count = pl_improvement_catalog(&items);
    puts("Recursive processor improvement capability matrix:");
    for (size_t i = 0u; i < count; ++i)
        printf("%02u | %-16s | %-16s | %s\n", items[i].id,
               items[i].engineering, items[i].optimization, items[i].automation);
    puts("Lifecycle: observe -> diagnose -> propose -> implement -> verify -> evaluate -> review -> integrate -> learn -> improve-engine");
    printf("Evidence-gated iteration target: %llu (maximum %llu)\n",
           (unsigned long long)PL_IMPROVEMENT_DEFAULT_ITERATIONS,
           (unsigned long long)PL_IMPROVEMENT_MAX_ITERATIONS);
    puts("The 66,666,666x ambition is not a guaranteed multiplier; progress requires measured evidence.");
    puts("Integration requires human approval. This command displays the plan; it does not rewrite or merge code.");
}
static void print_system_enhancements(void) {
    const pl_system_enhancement *items = NULL;
    size_t count = pl_system_enhancements(&items);
    puts("Twenty-system engineering contract catalogue:");
    for (size_t i = 0u; i < count; ++i) {
        printf("%02u | %-22s | %-14s | %s\n",
               items[i].id, items[i].name,
               pl_system_domain_name(items[i].domain), items[i].objective);
    }
    puts("These are acceptance contracts and roadmap tracks, not claims of completed features.");
}

static void print_systems_lab(void) {
    const pl_systems_function_info *items = NULL;
    size_t count = pl_systems_lab_catalog(&items);
    puts("33-function blockchain and processor systems lab:");
    for (size_t i = 0u; i < count; ++i)
        printf("%02u | %-36s | %-16s | %s\n", items[i].id,
               items[i].name, pl_systems_mode_name(items[i].mode), items[i].description);
    puts("Blockchain proofs use supplied cryptographic adapters; VM/CPU effects are local simulations or plans.");
    puts("No provider limits are bypassed; no physical clock, voltage, PMU, CAT, firmware or microcode registers are written.");
}

static void print_architecture_atlas(void) {
    const pl_architecture_profile *items = NULL;
    size_t count = pl_architecture_atlas_catalog(&items);
    puts("Architecture systems atlas (14 targets):");
    for (size_t i = 0u; i < count; ++i) {
        const pl_architecture_profile *p = &items[i];
        printf("%02u | %-12s | %-16s | %2u-bit | %-12s | %s\\n",
               p->id, p->name, p->class_name, p->word_bits,
               p->endianness, p->execution_model);
        printf("    Toolchain: %s\\n    Diagnostics: %s\\n",
               p->toolchain_hint, p->diagnostic_focus);
    }
    puts("Reference metadata only; no toolchain is assumed installed and no hardware is modified.");
}
static int print_architecture_profile(const char *name) {
    pl_architecture_profile p;
    int result = pl_architecture_atlas_find(name, &p);
    if (result != 0) {
        fprintf(stderr, "Unknown architecture profile: %s\\n", name ? name : "(null)");
        return 2;
    }
    printf("%s | %s | %s | %u-bit | %s-endian\\n",
           p.name, p.family, p.class_name, p.word_bits, p.endianness);
    printf("Execution: %s\\nToolchains: %s\\nDiagnostics: %s\\n",
           p.execution_model, p.toolchain_hint, p.diagnostic_focus);
    puts("This is a reference profile, not a live compatibility or toolchain-availability verdict.");
    return 0;
}

static void print_syntax_pie(void) {
    const pl_syntax_pie_layer *items = NULL;
    size_t count = pl_syntax_pie_catalog(&items);
    puts("Syntax-pie function layers (20 levels):");
    for (size_t i = 0u; i < count; ++i)
        printf("%02u | %-18s | %-12s | %-36s | %s\n", items[i].id,
               items[i].function_name, items[i].room_name, items[i].level_name,
               items[i].operation);
    puts("This is a deterministic syntax/function catalogue; it does not execute generated code.");
}
static void print_hw_report(void) {
    pl_hw_metrics metrics = {0};
    pl_hw_report report;
    size_t i;
    if (pl_hw_evaluate(&metrics, &report) != 0) { fputs("Hardware rule evaluation failed.\n", stderr); return; }
    printf("Hardware rules evaluated: %zu (implemented rules only)\n", report.rules_evaluated);
    puts("No live hardware-counter input was supplied; these are evidence/process gates, not measured CPU defects.");
    for (i = 0; i < report.findings_count; ++i) {
        const pl_hw_finding *f = &report.findings[i];
        printf("  [%03u] %s / %s: %s\n      %s\n", f->id, pl_hw_domain_name(f->domain), pl_hw_severity_name(f->severity), f->title, f->recommendation);
    }
}
static void print_foundations(int pipeline, int catalog, int policy, pl_pipeline_target target, pl_role role, pl_action action) {
    if (pipeline) {
        pl_pipeline_plan plan; size_t i;
        if (pl_pipeline_build(target, &plan) != 0) { fputs("Pipeline plan failed.\n", stderr); return; }
        printf("Reference pipeline for %s:\n", pl_pipeline_target_name(target));
        for (i = 0; i < plan.stage_count; ++i) printf("%u. %s\n", (unsigned)(i + 1u), pl_pipeline_stage_name(plan.stages[i]));
        puts("Semantic equivalence and human review are mandatory; this is a plan, not an active transpiler.");
    }
    if (catalog) {
        size_t i; pl_processor_rule rule;
        puts("Processor engineering workflow catalogue:");
        for (i = 0; i < pl_processor_catalog_count(); ++i) if (pl_processor_catalog_get(i, &rule) == 0)
            printf("- %s | %s | %s | source=%s | expert-signoff=required\n", rule.id, pl_processor_domain_name(rule.domain), rule.title, rule.evidence_source);
    }
    if (policy) {
        int allowed = pl_policy_authorize(role, action);
        printf("Authorization: %s (role=%s, action=%s)\n", allowed ? "ALLOW" : "DENY", pl_policy_role_name(role), pl_policy_action_name(action));
    }
}

static void print_e2k_profile(void) {
    pl_processor_snapshot p;
    if (pl_processor_scan(&p) != 0) {
        fputs("Target profile scan failed.\n", stderr);
        return;
    }
    printf("Build architecture: %s\n", p.architecture);
    if (p.is_e2k_target && p.mcst_lcc)
        printf("Toolchain: MCST LCC (major version %u)\n", p.lcc_major);
    else if (p.is_e2k_target)
        puts("Toolchain: E2K compiler detected; MCST LCC version macro unavailable");
    else
        puts("E2K target: not detected in this build");
    puts("VLIW scheduling and software pipelining: delegated to the actual target compiler");
    puts("AETHER-ANAS native backend: not implemented (target profile only)");
    puts("No fixed issue width, ALU count, register count, or cycle model is assumed.");
}

static void print_profile(void) {
    pl_processor_snapshot p; if(pl_processor_scan(&p)!=0){fprintf(stderr,"Processor scan failed.\n");return;}
    printf("Architecture: %s\nCPU: %s\nVendor: %s\nLogical processors: %u\nFamily/model: %u/%u\nPhysical memory: %llu MiB | available: %llu MiB\n",
           p.architecture,p.base.model,p.base.vendor,p.base.logical_cpus,p.base.family,p.base.model_id,
           (unsigned long long)p.physical_memory_mib,(unsigned long long)p.available_memory_mib);
    printf("ISA: SSE2=%u SSE4.2=%u AVX-HW=%u OSXSAVE=%u AVX-OS=%u AVX-usable=%u AVX2-usable=%u AVX512F-usable=%u AES=%u FMA-usable=%u virtualization=%u\n",
           p.sse2,p.sse42,p.avx_hw,p.osxsave,p.avx_os_state,p.avx,p.avx2,p.avx512f,p.aes,p.fma,p.virtualization);
    puts("AVX-family usable flags include OS-managed XCR0 vector-state validation; capability reporting does not execute SIMD instructions.");
}
int main(int argc,char **argv) {
    if (argc > 1 && strcmp(argv[1], "--embedded") == 0)
        return pl_embedded_cli(argc - 1, argv + 1);
    int profile=0,bench=0,capabilities=0,suite=0,improvement_plan=0,trimatrix=0,syntax_pie=0,hw_rules=0,pipeline=0,catalog=0,policy=0,evolve=0,berry_list=0,e2k_profile=0,systems=0,systems_lab=0,architectures=0; const char *architecture_name=NULL; const char *berry_name=NULL; unsigned berry_core=0u; const char *analyze=NULL,*json=NULL;
    const char *tone_out=NULL,*audio_info=NULL,*process_in=NULL,*process_out=NULL;
    pl_pipeline_target pipeline_target=PL_TARGET_C11; pl_role policy_role=PL_ROLE_VIEWER; pl_action policy_action=PL_ACTION_READ;
    unsigned evolution_rounds=8u, trimatrix_depth=PL_TMA_MAX_DEPTH;
    double frequency=440.0,seconds=3.0,lowpass=0.0,threshold=-12.0,ratio=4.0,saturation=0.0,gain=0.0;
    int use_compressor=0,use_gain=0;
    for(int i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--profile")) profile=1;
        else if(!strcmp(argv[i],"--e2k-profile")) e2k_profile=1;
        else if(!strcmp(argv[i],"--systems")) systems=1;
        else if(!strcmp(argv[i],"--systems-lab")) systems_lab=1;
        else if(!strcmp(argv[i],"--architectures")) architectures=1;
        else if(!strcmp(argv[i],"--architecture")&&i+1<argc) architecture_name=argv[++i];
        else if(!strcmp(argv[i],"--capabilities")) capabilities=1;
        else if(!strcmp(argv[i],"--benchmark")) bench=1;
        else if(!strcmp(argv[i],"--suite")) suite=1;
        else if(!strcmp(argv[i],"--improvement-plan")) improvement_plan=1;
        else if(!strcmp(argv[i],"--trimatrix")) {
            trimatrix=1;
            if(i+1<argc && argv[i+1][0]!='-') {
                char *end=NULL;
                unsigned long n=strtoul(argv[++i],&end,10);
                if(!end || *end || n<1u || n>PL_TMA_MAX_DEPTH) return 2;
                trimatrix_depth=(unsigned)n;
            }
        }
        else if(!strcmp(argv[i],"--syntax-pie")) syntax_pie=1;
        else if(!strcmp(argv[i],"--hw-rules")) hw_rules=1;
        else if(!strcmp(argv[i],"--pipeline")) pipeline=1;
        else if(!strcmp(argv[i],"--processor-catalog")) catalog=1;
        else if(!strcmp(argv[i],"--evolve")) evolve=1;
        else if(!strcmp(argv[i],"--core-commands")) berry_list=1;
        else if(!strcmp(argv[i],"--core-command")&&i+1<argc) berry_name=argv[++i];
        else if(!strcmp(argv[i],"--core")&&i+1<argc){char *end=NULL;unsigned long n=strtoul(argv[++i],&end,10);if(!end||*end||n>UINT32_MAX)return 2;berry_core=(unsigned)n;}
        else if(!strcmp(argv[i],"--evolution-rounds")&&i+1<argc){char *end=NULL;unsigned long n=strtoul(argv[++i],&end,10);if(!end||*end||n<1u||n>PL_EVOLUTION_MAX_ROUNDS){fprintf(stderr,"Evolution rounds must be 1..%u.\n",PL_EVOLUTION_MAX_ROUNDS);return 2;}evolution_rounds=(unsigned)n;evolve=1;}
        else if(!strcmp(argv[i],"--target")&&i+1<argc){const char *v=argv[++i];if(!strcmp(v,"c11"))pipeline_target=PL_TARGET_C11;else if(!strcmp(v,"c23"))pipeline_target=PL_TARGET_C23;else return 2;}
        else if(!strcmp(argv[i],"--policy-check")&&i+2<argc){policy=1;if(pl_policy_parse_role(argv[++i],&policy_role)!=0||pl_policy_parse_action(argv[++i],&policy_action)!=0)return 2;}
        else if((!strcmp(argv[i],"--review-processor")||!strcmp(argv[i],"--analyze"))&&i+1<argc) analyze=argv[++i];
        else if(!strcmp(argv[i],"--json")&&i+1<argc) json=argv[++i];
        else if(!strcmp(argv[i],"--tone-out")&&i+1<argc) tone_out=argv[++i];
        else if(!strcmp(argv[i],"--audio-info")&&i+1<argc) audio_info=argv[++i];
        else if(!strcmp(argv[i],"--process-wav")&&i+2<argc){process_in=argv[++i];process_out=argv[++i];}
        else if(!strcmp(argv[i],"--frequency")&&i+1<argc){if(parse_double(argv[++i],&frequency))return 2;}
        else if(!strcmp(argv[i],"--seconds")&&i+1<argc){if(parse_double(argv[++i],&seconds))return 2;}
        else if(!strcmp(argv[i],"--lowpass")&&i+1<argc){if(parse_double(argv[++i],&lowpass))return 2;}
        else if(!strcmp(argv[i],"--compress")&&i+2<argc){if(parse_double(argv[++i],&threshold)||parse_double(argv[++i],&ratio))return 2;use_compressor=1;}
        else if(!strcmp(argv[i],"--saturate")&&i+1<argc){if(parse_double(argv[++i],&saturation))return 2;}
        else if(!strcmp(argv[i],"--gain")&&i+1<argc){if(parse_double(argv[++i],&gain))return 2;use_gain=1;}
        else if(!strcmp(argv[i],"--help")){usage();return 0;}
        else {fprintf(stderr,"Unknown or incomplete option: %s\n",argv[i]);usage();return 2;}
    }
    if(e2k_profile) print_e2k_profile();
    if(berry_list) { print_berry_core_catalog(); if(argc==2) return 0; }
    if(berry_name) return dispatch_berry_core(berry_name,berry_core);
    if(systems) print_system_enhancements();
    if(systems_lab) print_systems_lab();
    if(architectures) print_architecture_atlas();
    if(architecture_name) return print_architecture_profile(architecture_name);
    if(improvement_plan) print_improvement_plan();
    if(trimatrix) print_trimatrix(trimatrix_depth);
    if(syntax_pie) print_syntax_pie();
    if(hw_rules) print_hw_report();
    if(pipeline||catalog||policy) print_foundations(pipeline,catalog,policy,pipeline_target,policy_role,policy_action);
    if(policy && !pl_policy_authorize(policy_role,policy_action)) return 3;
    if(tone_out) {
        pl_audio_buffer a={0}; int rc=pl_audio_generate_tone(&a,48000,2,frequency,seconds);
        if(rc!=0){fprintf(stderr,"Tone parameters rejected (%d).\n",rc);return 1;}
        rc=pl_audio_write_wav(tone_out,&a); pl_audio_free(&a);
        if(rc!=0){fprintf(stderr,"Could not write WAV: %s\n",tone_out);return 1;}
        printf("Generated %s (%g Hz, %g s, stereo 48 kHz PCM16).\n",tone_out,frequency,seconds); return 0;
    }
    if(audio_info || process_in) {
        pl_audio_buffer a={0}; const char *input=audio_info?audio_info:process_in;
        int rc=pl_audio_load_wav(input,&a); if(rc!=0){fprintf(stderr,"Could not load supported PCM16 WAV (%d): %s\n",rc,input);return 1;}
        if(lowpass>0.0 && pl_audio_apply_lowpass(&a,lowpass)!=0){fprintf(stderr,"Invalid low-pass cutoff.\n");pl_audio_free(&a);return 2;}
        if(use_compressor && pl_audio_apply_compressor(&a,threshold,ratio)!=0){fprintf(stderr,"Invalid compressor settings.\n");pl_audio_free(&a);return 2;}
        if(saturation>0.0 && pl_audio_apply_saturation(&a,saturation)!=0){fprintf(stderr,"Invalid saturation drive.\n");pl_audio_free(&a);return 2;}
        if(use_gain && pl_audio_apply_gain(&a,gain)!=0){fprintf(stderr,"Invalid gain.\n");pl_audio_free(&a);return 2;}
        pl_audio_stats st; if(pl_audio_measure(&a,&st)==0) printf("%u Hz | %u channel(s) | %lu frames | peak %.5f | RMS %.5f | crest %.2f dB | clipped %lu\n",a.sample_rate,a.channels,(unsigned long)a.frames,st.peak,st.rms,st.crest_db,(unsigned long)st.clipped_samples);
        if(process_in && pl_audio_write_wav(process_out,&a)!=0){fprintf(stderr,"Could not write processed WAV: %s\n",process_out);pl_audio_free(&a);return 1;}
        if(process_in) printf("Processed WAV written: %s\n",process_out);
        pl_audio_free(&a); return 0;
    }
    if((systems || systems_lab || architectures || architecture_name || improvement_plan || syntax_pie || trimatrix || e2k_profile) && (argc==2 || (trimatrix && argc<=3))) return 0;
    if(!profile&&!bench&&!suite&&!capabilities&&!analyze&&!json&&!systems&&!systems_lab&&!architectures&&!architecture_name&&!improvement_plan&&!trimatrix&&!syntax_pie&&!hw_rules&&!pipeline&&!catalog&&!policy&&!evolve&&!e2k_profile){profile=1;bench=1;}
    pl_cpu_info cpu; pl_benchmark b={0}; pl_analysis a={0}; char details[8192]={0};
    if(pl_cpu_profile(&cpu)!=0)return 1;
    if((profile||capabilities))print_profile();
    if(bench){if(pl_run_benchmark(UINT64_C(10000000),&b)!=0)return 1;printf("Benchmark: %llu iterations, %.3f ms, %.1f iterations/ms\n",(unsigned long long)b.iterations,b.elapsed_ms,b.score);}
    if(suite){pl_suite_result sr;int rc=pl_processor_benchmark_suite(UINT64_C(12000000),8,4,&sr);if(rc!=0){fprintf(stderr,"Processor suite failed (%d).\n",rc);return 1;}printf("Suite: integer %.2f ms | memory %.2f ms | matrix %.2f ms | checksum %llu | integrity %s\n",sr.integer_ms,sr.memory_ms,sr.matrix_ms,(unsigned long long)sr.checksum,sr.integrity_ok?"PASS":"CHECK");}
    if(analyze){if(pl_analyze_file(analyze,&a,details,sizeof(details))!=0){fprintf(stderr,"Could not inspect processor source: %s\n",analyze);return 1;}printf("Processor implementation review: %u file(s), %u finding(s), %u read error(s)\n%s",a.files_scanned,a.findings,a.read_errors,details);}
    if(json){if(pl_write_json(json,&cpu,&b,&a)!=0){fprintf(stderr,"Could not write JSON report: %s\n",json);return 1;}printf("JSON report: %s\n",json);}
    if(evolve){
        pl_processor_snapshot snapshot;
        pl_evolution_input input={0};
        pl_evolution_report report;
        input.source_files=a.files_scanned;
        input.static_findings=a.findings;
        input.benchmark_samples=bench?1u:0u;
        input.baseline_iterations_per_ms=bench?b.score:0.0;
        if(pl_processor_scan(&snapshot)==0){
            input.architecture_known=strcmp(snapshot.architecture,"unknown")!=0?1u:0u;
            input.memory_mib=snapshot.physical_memory_mib>PL_EVOLUTION_MAX_MEMORY_MIB
                ? PL_EVOLUTION_MAX_MEMORY_MIB : (unsigned)snapshot.physical_memory_mib;
        }
        input.tests_passed=0u;
        if(pl_evolution_run(&input,evolution_rounds,&report)!=0){
            fprintf(stderr,"HyperFusion evaluation failed.\n");return 1;
        }
        printf("HyperFusion: %u round(s), %llu module calls, %llu directed interactions\n",
            report.rounds_completed,(unsigned long long)report.module_invocations,
            (unsigned long long)report.pairwise_interactions);
        printf("Readiness: %.2f/100 | synergy: %.2f | convergence: %s | promotion gate: %s\n",
            report.readiness_score,report.synergy_score,report.converged?"YES":"NO",
            report.gate_passed?"PASS":"HELD (CI test result required)");
    }
    return 0;
}
