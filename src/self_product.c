#include "self_product.h"
#include <string.h>
static uint64_t saturating_add(uint64_t left,uint64_t right,int *saturated) {
    if (right > UINT64_MAX-left) { *saturated=1; return UINT64_MAX; }
    return left+right;
}
int pl_self_product_u64(const uint64_t *matrix,size_t dimension,unsigned depth,
                        uint64_t *output,pl_self_product_report *report) {
    uint64_t current[PL_SELF_PRODUCT_MAX_DIM*PL_SELF_PRODUCT_MAX_DIM];
    uint64_t next[PL_SELF_PRODUCT_MAX_DIM*PL_SELF_PRODUCT_MAX_DIM];
    size_t cells;
    if(matrix==NULL||output==NULL||report==NULL||dimension==0u||
       dimension>PL_SELF_PRODUCT_MAX_DIM||depth==0u||
       depth>PL_SELF_PRODUCT_MAX_DEPTH) return -1;
    cells=dimension*dimension;
    memset(report,0,sizeof(*report));
    memcpy(current,matrix,cells*sizeof(current[0]));
    for(unsigned layer=1u;layer<=depth;++layer) {
        for(size_t row=0u;row<dimension;++row) {
            for(size_t column=0u;column<dimension;++column) {
                uint64_t sum=0u;
                int cell_saturated=0;
                for(size_t inner=0u;inner<dimension;++inner) {
                    uint64_t left=current[row*dimension+inner];
                    uint64_t right=current[inner*dimension+column];
                    uint64_t product;
                    if(left!=0u&&right>UINT64_MAX/left) {
                        product=UINT64_MAX; cell_saturated=1;
                    } else product=left*right;
                    sum=saturating_add(sum,product,&cell_saturated);
                }
                next[row*dimension+column]=sum;
                if(cell_saturated) { ++report->saturated_cells; report->saturated=1; }
            }
        }
        memcpy(current,next,cells*sizeof(current[0]));
        report->layers_completed=layer;
    }
    memcpy(output,current,cells*sizeof(current[0]));
    report->dimension=dimension;
    for(size_t index=0u;index<dimension;++index)
        report->diagonal_sum=saturating_add(report->diagonal_sum,
            current[index*dimension+index],&report->saturated);
    return 0;
}
