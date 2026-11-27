#include "processor_evidence.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>
int main(void) {
    char hash[PL_TR_SHA256_HEX_SIZE];
    assert(pl_tr_sha256_hex("abc",3u,hash)==0);
    assert(strcmp(hash,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")==0);
    return 0;
}
