#include "self_product.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
int main(void) {
    const uint64_t fibonacci[4]={1u,1u,1u,0u};
    uint64_t output[4]={0u,0u,0u,0u};
    pl_self_product_report report;
    assert(pl_self_product_u64(fibonacci,2u,2u,output,&report)==0);
    assert(output[0]==5u&&output[1]==3u&&output[2]==3u&&output[3]==2u);
    assert(report.dimension==2u&&report.layers_completed==2u);
    assert(report.diagonal_sum==7u&&report.saturated_cells==0u&&!report.saturated);
    {
        const uint64_t large[1]={UINT64_MAX};
        uint64_t saturated[1]={0u};
        assert(pl_self_product_u64(large,1u,1u,saturated,&report)==0);
        assert(saturated[0]==UINT64_MAX&&report.saturated&&report.saturated_cells==1u);
    }
    assert(pl_self_product_u64(fibonacci,0u,1u,output,&report)!=0);
    assert(pl_self_product_u64(fibonacci,2u,0u,output,&report)!=0);
    assert(pl_self_product_u64(fibonacci,2u,PL_SELF_PRODUCT_MAX_DEPTH+1u,output,&report)!=0);
    assert(pl_self_product_u64(NULL,2u,1u,output,&report)!=0);
    assert(pl_self_product_u64(fibonacci,2u,1u,NULL,&report)!=0);
    assert(pl_self_product_u64(fibonacci,2u,1u,output,NULL)!=0);
    puts("Vertical matrix self-product tests passed");
    return 0;
}
