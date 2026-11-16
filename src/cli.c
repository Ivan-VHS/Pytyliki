#include "core.h"
#include "processor.h"
#include "dsp.h"
#include "improvement.h"
#include "hw_rules.h"
#include "clinical_rules.h"
#include "pipeline.h"
#include "policy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(void) {
    puts("Pytyliki Processor Systems Workstation 0.7.0\n"
         "Usage:\n"
         "  pytyliki [--profile] [--capabilities] [--benchmark] [--suite]\n"
         "           [--improvement-plan] [--hw-rules] [--pipeline] [--clinical-catalog] [--policy-check ROLE ACTION] [--review-processor FILE] (alias: --analyze FILE) [--json FILE]\n"
         "  pytyliki --tone-out FILE.wav [--frequency HZ] [--seconds N]\n"
         "  pytyliki --audio-info FILE.wav\n"
         "  pytyliki --process-wav INPUT.wav OUTPUT.wav [--lowpass HZ]\n"
         "           [--compress THRESHOLD_DB RATIO] [--saturate DRIVE] [--gain DB]\n"
         "Processor inspection is read-only. Execution and DSP datapath workloads are bounded.");
}
static int parse_double(const char *s,double *out) {
    char *end=NULL; double v=strtod(s,&end); if(!s[0]||!end||*end||v!=v) return -1; *out=v; return 0;
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
static void print_foundations(int pipeline, int clinical, int policy, pl_pipeline_target target, pl_role role, pl_action action) {
    if (pipeline) {
        pl_pipeline_plan plan; size_t i;
        if (pl_pipeline_build(target, &plan) != 0) { fputs("Pipeline plan failed.\n", stderr); return; }
        printf("Reference pipeline for %s:\n", pl_pipeline_target_name(target));
        for (i = 0; i < plan.stage_count; ++i) printf("%u. %s\n", (unsigned)(i + 1u), pl_pipeline_stage_name(plan.stages[i]));
        puts("Semantic equivalence and human review are mandatory; this is a plan, not an active transpiler.");
    }
    if (clinical) {
        size_t i; pl_clinical_rule rule;
        puts("Engineering workflow templates (not clinical instructions):");
        for (i = 0; i < pl_clinical_catalog_count(); ++i) if (pl_clinical_catalog_get(i, &rule) == 0)
            printf("- %s | %s | %s | source=%s | expert-signoff=required\n", rule.id, pl_clinical_domain_name(rule.domain), rule.title, rule.approved_source);
    }
    if (policy) {
        int allowed = pl_policy_authorize(role, action);
        printf("Authorization: %s (role=%s, action=%s)\n", allowed ? "ALLOW" : "DENY", pl_policy_role_name(role), pl_policy_action_name(action));
    }
}
static void print_profile(void) {
    pl_processor_snapshot p; if(pl_processor_scan(&p)!=0){fprintf(stderr,"Processor scan failed.\n");return;}
    printf("Architecture: %s\nCPU: %s\nVendor: %s\nLogical processors: %u\nFamily/model: %u/%u\nPhysical memory: %llu MiB | available: %llu MiB\n",
           p.architecture,p.base.model,p.base.vendor,p.base.logical_cpus,p.base.family,p.base.model_id,
           (unsigned long long)p.physical_memory_mib,(unsigned long long)p.available_memory_mib);
    printf("ISA: SSE2=%u SSE4.2=%u AVX=%u AVX2=%u AVX-512F=%u AES=%u FMA=%u virtualization=%u\n",
           p.sse2,p.sse42,p.avx,p.avx2,p.avx512f,p.aes,p.fma,p.virtualization);
    puts("Note: AVX hardware flags are informational; OS vector-state support is not validated.");
}
int main(int argc,char **argv) {
    int profile=0,bench=0,capabilities=0,suite=0,improvement_plan=0,hw_rules=0,pipeline=0,clinical=0,policy=0; const char *analyze=NULL,*json=NULL;
    const char *tone_out=NULL,*audio_info=NULL,*process_in=NULL,*process_out=NULL;
    pl_pipeline_target pipeline_target=PL_TARGET_C11; pl_role policy_role=PL_ROLE_VIEWER; pl_action policy_action=PL_ACTION_READ;
    double frequency=440.0,seconds=3.0,lowpass=0.0,threshold=-12.0,ratio=4.0,saturation=0.0,gain=0.0;
    int use_compressor=0,use_gain=0;
    for(int i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--profile")) profile=1;
        else if(!strcmp(argv[i],"--capabilities")) capabilities=1;
        else if(!strcmp(argv[i],"--benchmark")) bench=1;
        else if(!strcmp(argv[i],"--suite")) suite=1;
        else if(!strcmp(argv[i],"--improvement-plan")) improvement_plan=1;
        else if(!strcmp(argv[i],"--hw-rules")) hw_rules=1;
        else if(!strcmp(argv[i],"--pipeline")) pipeline=1;
        else if(!strcmp(argv[i],"--clinical-catalog")) clinical=1;
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
    if(improvement_plan) print_improvement_plan();
    if(hw_rules) print_hw_report();
    if(pipeline||clinical||policy) print_foundations(pipeline,clinical,policy,pipeline_target,policy_role,policy_action);
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
    if(improvement_plan && argc==2) return 0;
    if(!profile&&!bench&&!suite&&!capabilities&&!analyze&&!json&&!improvement_plan&&!hw_rules&&!pipeline&&!clinical&&!policy){profile=1;bench=1;}
    pl_cpu_info cpu; pl_benchmark b={0}; pl_analysis a={0}; char details[8192]={0};
    if(pl_cpu_profile(&cpu)!=0)return 1;
    if((profile||capabilities))print_profile();
    if(bench){if(pl_run_benchmark(UINT64_C(10000000),&b)!=0)return 1;printf("Benchmark: %llu iterations, %.3f ms, %.1f iterations/ms\n",(unsigned long long)b.iterations,b.elapsed_ms,b.score);}
    if(suite){pl_suite_result sr;int rc=pl_processor_benchmark_suite(UINT64_C(12000000),8,4,&sr);if(rc!=0){fprintf(stderr,"Processor suite failed (%d).\n",rc);return 1;}printf("Suite: integer %.2f ms | memory %.2f ms | matrix %.2f ms | checksum %llu | integrity %s\n",sr.integer_ms,sr.memory_ms,sr.matrix_ms,(unsigned long long)sr.checksum,sr.integrity_ok?"PASS":"CHECK");}
    if(analyze){if(pl_analyze_file(analyze,&a,details,sizeof(details))!=0){fprintf(stderr,"Could not inspect processor source: %s\n",analyze);return 1;}printf("Processor implementation review: %u file(s), %u finding(s), %u read error(s)\n%s",a.files_scanned,a.findings,a.read_errors,details);}
    if(json){if(pl_write_json(json,&cpu,&b,&a)!=0){fprintf(stderr,"Could not write JSON report: %s\n",json);return 1;}printf("JSON report: %s\n",json);}
    return 0;
}
