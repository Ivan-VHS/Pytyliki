#define _GNU_SOURCE
#include "processor_affinity.h"

#include <string.h>

#define PL_TR_AFFINITY_MAGIC UINT32_C(0x50544146)
#define PL_TR_AFFINITY_WINDOWS 1u
#define PL_TR_AFFINITY_LINUX 2u

void pl_tr_affinity_lease_init(pl_tr_affinity_lease *lease) {
    if (lease != NULL) memset(lease, 0, sizeof(*lease));
}

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef struct {
    GROUP_AFFINITY previous;
    DWORD owner_thread;
} affinity_saved_state;
_Static_assert(sizeof(affinity_saved_state)<=PL_TR_AFFINITY_TOKEN_BYTES,
               "affinity lease storage is too small");
int pl_tr_affinity_preview(uint32_t group,uint32_t processor) {
    GROUP_AFFINITY current;
    if(group>=GetActiveProcessorGroupCount() || processor>=64u) return -1;
    if(processor>=GetActiveProcessorCount((WORD)group)) return -1;
    if(!GetThreadGroupAffinity(GetCurrentThread(),&current)) return -2;
    if(current.Group!=group ||
       (current.Mask&((KAFFINITY)1u<<processor))==0u) return -3;
    return 0;
}
int pl_tr_affinity_apply_current(uint32_t group,uint32_t processor,
                                 pl_tr_affinity_lease *lease) {
    GROUP_AFFINITY requested,previous;
    affinity_saved_state saved;
    if(lease==NULL || lease->active || pl_tr_affinity_preview(group,processor)!=0) return -1;
    memset(&requested,0,sizeof requested);
    requested.Group=(WORD)group;
    requested.Mask=((KAFFINITY)1u<<processor);
    if(!SetThreadGroupAffinity(GetCurrentThread(),&requested,&previous)) return -2;
    saved.previous=previous;
    saved.owner_thread=GetCurrentThreadId();
    memset(lease,0,sizeof(*lease));
    memcpy(lease->storage.bytes,&saved,sizeof saved);
    lease->magic=PL_TR_AFFINITY_MAGIC;
    lease->platform=PL_TR_AFFINITY_WINDOWS;
    lease->active=1u;
    return 0;
}
int pl_tr_affinity_restore_current(pl_tr_affinity_lease *lease) {
    affinity_saved_state saved;
    if(lease==NULL || lease->active!=1u ||
       lease->magic!=PL_TR_AFFINITY_MAGIC ||
       lease->platform!=PL_TR_AFFINITY_WINDOWS) return -1;
    memcpy(&saved,lease->storage.bytes,sizeof saved);
    if(saved.owner_thread!=GetCurrentThreadId()) return -2;
    if(!SetThreadGroupAffinity(GetCurrentThread(),&saved.previous,NULL)) return -3;
    memset(lease,0,sizeof(*lease));
    return 0;
}
#elif defined(__linux__)
#include <pthread.h>
#include <sched.h>
#include <errno.h>
typedef struct {
    cpu_set_t previous;
    pthread_t owner_thread;
} affinity_saved_state;
_Static_assert(sizeof(affinity_saved_state)<=PL_TR_AFFINITY_TOKEN_BYTES,
               "affinity lease storage is too small");
int pl_tr_affinity_preview(uint32_t group,uint32_t processor) {
    cpu_set_t allowed;
    int status;
    if(group!=0u || processor>=CPU_SETSIZE) return -1;
    CPU_ZERO(&allowed);
    status=pthread_getaffinity_np(pthread_self(),sizeof allowed,&allowed);
    if(status!=0) return -2;
    return CPU_ISSET((int)processor,&allowed) ? 0 : -3;
}
int pl_tr_affinity_apply_current(uint32_t group,uint32_t processor,
                                 pl_tr_affinity_lease *lease) {
    cpu_set_t requested;
    affinity_saved_state saved;
    int status;
    if(lease==NULL || lease->active || group!=0u || processor>=CPU_SETSIZE) return -1;
    CPU_ZERO(&saved.previous);
    status=pthread_getaffinity_np(pthread_self(),sizeof saved.previous,&saved.previous);
    if(status!=0) return -2;
    if(!CPU_ISSET((int)processor,&saved.previous)) return -3;
    saved.owner_thread=pthread_self();
    CPU_ZERO(&requested);
    CPU_SET((int)processor,&requested);
    status=pthread_setaffinity_np(pthread_self(),sizeof requested,&requested);
    if(status!=0) return -4;
    memset(lease,0,sizeof(*lease));
    memcpy(lease->storage.bytes,&saved,sizeof saved);
    lease->magic=PL_TR_AFFINITY_MAGIC;
    lease->platform=PL_TR_AFFINITY_LINUX;
    lease->active=1u;
    return 0;
}
int pl_tr_affinity_restore_current(pl_tr_affinity_lease *lease) {
    affinity_saved_state saved;
    int status;
    if(lease==NULL || lease->active!=1u ||
       lease->magic!=PL_TR_AFFINITY_MAGIC ||
       lease->platform!=PL_TR_AFFINITY_LINUX) return -1;
    memcpy(&saved,lease->storage.bytes,sizeof saved);
    if(!pthread_equal(saved.owner_thread,pthread_self())) return -2;
    status=pthread_setaffinity_np(pthread_self(),sizeof saved.previous,&saved.previous);
    if(status!=0) return -3;
    memset(lease,0,sizeof(*lease));
    return 0;
}
#else
int pl_tr_affinity_preview(uint32_t group,uint32_t processor) {
    (void)group;(void)processor;return -1;
}
int pl_tr_affinity_apply_current(uint32_t group,uint32_t processor,
                                 pl_tr_affinity_lease *lease) {
    (void)group;(void)processor;(void)lease;return -1;
}
int pl_tr_affinity_restore_current(pl_tr_affinity_lease *lease) {
    (void)lease;return -1;
}
#endif
