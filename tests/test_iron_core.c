#include "iron_core.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static FILE *open_test_stream(void) {
    FILE *stream = NULL;
#if defined(_MSC_VER)
    if (fopen_s(&stream, "pytyliki-iron-core-test.tmp", "w+b") != 0) {
        return NULL;
    }
#else
    stream = fopen("pytyliki-iron-core-test.tmp", "w+b");
#endif
    return stream;
}

static const char valid[]=
"verison ///. * PYTYLIKI_IRON_CORE_V1\nCC based rules ^^^ BASELINE_V1\nbegin iron )\n"
"() core --\ncore_id = 1;\nstop option ;\n[ system ]-\nmode = read_only;\nstop option ;\n"
"] level .\nprivilege_ring = 0;\nstop option ;\n< start >?\nboot_vector = 0;\nstop option ;\n"
"{ selective /\nfilter = bounded;\nstop option ;\n} operation P)-\nop = inspect;\nstop option ;\n"
"?? massive <>\nworkers = 4;\nstop option ;\ngrown new-work >/\nqueue = local;\nstop option ;\n"
"> concrete //*\nbinding = explicit;\nstop option ;\n## utility _\nhelper = pure;\nstop option ;\n"
"# unify TAB\npitch = 4;\nstop option ;\nsetup brachpass ^^\nhints = offline;\nstop option ;\n"
"!+ hyper tournament ^\nrounds = 2;\nstop option ;\nlevel jump to run *(\ntarget = local;\nstop option ;\n"
"TRUE connective *+\npredicate = all;\nstop option ;\n"
"verison ///. * PYTYLIKI_IRON_CORE_COMPLETE\nverificated /// !! SOURCE_CLAIM_ONLY\n";
static void test_catalog(void){const pl_iron_marker_info*a;assert(pl_iron_catalog(&a)==20);assert(!strcmp(a[PL_IRON_BRACHPASS].token,"setup brachpass ^^"));assert(pl_iron_marker_for_kind((pl_iron_marker_kind)99)==NULL);}
static void test_valid(void){pl_iron_program p;pl_iron_error e;assert(pl_iron_parse(valid,&p,&e));assert(p.block_count==15);assert(p.version_count==2);assert(p.policy_count==1);assert(p.verification_after_root_count==1);assert(p.source_seal_asserted&&!p.source_seal_is_ci_evidence);assert(!p.semantic_validation_performed&&!p.execution_performed&&!p.hardware_access_performed);assert(!strcmp(p.version,"PYTYLIKI_IRON_CORE_COMPLETE"));assert(p.blocks[14].kind==PL_IRON_TRUE_CONNECTIVE&&p.blocks[14].closed);}
static void test_bad_frames(void){pl_iron_program p;pl_iron_error e;
 const char*unclosed="verison ///. * V1\nCC based rules ^^^ R1\nbegin iron )\n() core --\nx=1;\nverificated /// !! CLAIM\n";
 const char*stray="verison ///. * V1\nCC based rules ^^^ R1\nbegin iron )\nstop option ;\nverificated /// !! CLAIM\n";
 const char*no_root="verison ///. * V1\nCC based rules ^^^ R1\nverificated /// !! CLAIM\n";
 const char*no_seal="verison ///. * V1\nCC based rules ^^^ R1\nbegin iron )\n() core --\nstop option ;\n";
 assert(!pl_iron_parse(unclosed,&p,&e)&&strstr(e.message,"missing stop option"));assert(!pl_iron_parse(stray,&p,&e)&&strstr(e.message,"no matching open block"));assert(!pl_iron_parse(no_root,&p,&e)&&strstr(e.message,"missing begin iron"));assert(!pl_iron_parse(no_seal,&p,&e)&&strstr(e.message,"final verificated"));}
static void test_comments_strings(void){const char*s="verison ///. * V1\nCC based rules ^^^ R1\nbegin iron )\n() core --\n/*\n{ selective /\nstop option ;\n*/\nconst char*x=\"stop option ;\";\nstop option ;\nverificated /// !! CLAIM\n";pl_iron_program p;pl_iron_error e;assert(pl_iron_parse(s,&p,&e));assert(p.block_count==1&&p.marker_counts[PL_IRON_SELECTIVE]==0&&p.marker_counts[PL_IRON_STOP]==1);}
static void test_duplicate_and_limit(void){pl_iron_program p;pl_iron_error e;const char*d="verison ///. * V1\nCC based rules ^^^ R1\nbegin iron )\nbegin iron )\n() core --\nstop option ;\nverificated /// !! CLAIM\n";char big[PL_IRON_MAX_SOURCE_BYTES+2];size_t i;assert(!pl_iron_parse(d,&p,&e)&&strstr(e.message,"duplicate begin iron"));for(i=0;i<sizeof(big)-1;i++)big[i]='x';big[sizeof(big)-1]='\0';assert(!pl_iron_parse(big,&p,&e)&&strstr(e.message,"one-megabyte"));}
static void test_json(void){pl_iron_program p;pl_iron_error e;FILE*f=open_test_stream();char b[12000];size_t n;assert(f&&pl_iron_parse(valid,&p,&e)&&pl_iron_write_json(&p,f));fflush(f);rewind(f);n=fread(b,1,sizeof(b)-1,f);b[n]='\0';assert(strstr(b,PL_IRON_AST_SCHEMA)&&strstr(b,"\"verification_claims_are_ci_evidence\": false")&&strstr(b,"\"hardware_access_performed\": false"));fclose(f);f=open_test_stream();assert(f&&pl_iron_write_catalog_json(f));fflush(f);rewind(f);n=fread(b,1,sizeof(b)-1,f);b[n]='\0';assert(strstr(b,"pytyliki.iron-core-catalog.v1")&&strstr(b,"\"count\": 20"));fclose(f);}
int main(void){test_catalog();test_valid();test_bad_frames();test_comments_strings();test_duplicate_and_limit();test_json();puts("Pytyliki Iron Core tests passed");return 0;}
