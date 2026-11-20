#include "embedded.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>
int main(void) {
    const pl_embedded_target *target=NULL;
    pl_embedded_source_report report;
    const char source[]="#include <stdint.h>\nvolatile unsigned *GPIO=0;\nvoid task(void) { malloc(8); HAL_Delay(10); printf(\"x\"); }\n";
    assert(pl_embedded_target_count()>=7u);
    assert(pl_embedded_target_find("arm-cortex-m0",&target)==0);
    assert(strcmp(target->compiler,"arm-none-eabi-gcc")==0&&target->word_bits==32u);
    assert(pl_embedded_target_at(pl_embedded_target_count(),&target)!=0);
    assert(pl_embedded_inspect_source(source,sizeof(source)-1u,&report)==0);
    assert(report.source_bytes==sizeof(source)-1u&&report.line_count==4u);
    assert(report.include_count==1u&&report.heap_calls==1u&&report.blocking_calls==1u);
    assert(report.register_markers>=2u&&report.host_io_calls==1u);
    assert((report.warnings&PL_EMBEDDED_WARN_DYNAMIC_ALLOCATION)!=0u);
    assert((report.warnings&PL_EMBEDDED_WARN_BLOCKING_CALLS)!=0u);
    assert(pl_embedded_inspect_source(NULL,0u,&report)!=0);
    assert(pl_embedded_inspect_source(source,PL_EMBEDDED_MAX_SOURCE_BYTES+1u,&report)!=0);
    return 0;
}
