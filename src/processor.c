#define _POSIX_C_SOURCE 200809L
#include "processor.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#include <intrin.h>
#elif defined(__x86_64__) || defined(__i386__)
#include <cpuid.h>
#endif
static uint64_t pages_to_mib(long pages,long page_size){
 if(pages<=0||page_size<=0)return 0u;
 const uint64_t page_count=(uint64_t)pages;
 const uint64_t bytes_per_page=(uint64_t)page_size;
 if(page_count>UINT64_MAX/bytes_per_page)
  return UINT64_MAX/(UINT64_C(1024)*UINT64_C(1024));
 return (page_count*bytes_per_page)/(UINT64_C(1024)*UINT64_C(1024));
}
static double processor_now_ms(void){
#if defined(_WIN32)
 LARGE_INTEGER f,t;if(!QueryPerformanceFrequency(&f)||!QueryPerformanceCounter(&t))return 0.0;return 1000.0*(double)t.QuadPart/(double)f.QuadPart;
#else
 struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t)!=0)return 0.0;return 1000.0*(double)t.tv_sec+(double)t.tv_nsec/1000000.0;
#endif
}
int pl_processor_scan(pl_processor_snapshot*out){if(!out)return -1;memset(out,0,sizeof(*out));if(pl_cpu_profile(&out->base)!=0)return -1;
#if defined(_WIN32)
 MEMORYSTATUSEX m;memset(&m,0,sizeof(m));m.dwLength=sizeof(m);if(GlobalMemoryStatusEx(&m)){out->physical_memory_mib=(uint64_t)(m.ullTotalPhys/(1024u*1024u));out->available_memory_mib=(uint64_t)(m.ullAvailPhys/(1024u*1024u));}
#elif defined(_SC_PHYS_PAGES)&&defined(_SC_PAGESIZE)
 long pages=sysconf(_SC_PHYS_PAGES),size=sysconf(_SC_PAGESIZE);
 out->physical_memory_mib=pages_to_mib(pages,size);
# if defined(_SC_AVPHYS_PAGES)
 long avail=sysconf(_SC_AVPHYS_PAGES);
 out->available_memory_mib=pages_to_mib(avail,size);
 if(out->physical_memory_mib!=0u&&out->available_memory_mib>out->physical_memory_mib)
  out->available_memory_mib=out->physical_memory_mib;
# endif
#endif
#if defined(__e2k__)
 snprintf(out->architecture,sizeof(out->architecture),"e2k");
#elif defined(_M_X64)||defined(__x86_64__)
 snprintf(out->architecture,sizeof(out->architecture),"x86-64");
#elif defined(_M_IX86)||defined(__i386__)
 snprintf(out->architecture,sizeof(out->architecture),"x86");
#elif defined(_M_ARM64)||defined(__aarch64__)
 snprintf(out->architecture,sizeof(out->architecture),"ARM64");
#elif defined(_M_ARM)||defined(__arm__)
 snprintf(out->architecture,sizeof(out->architecture),"ARM");
#else
 snprintf(out->architecture,sizeof(out->architecture),"unknown");
#endif
#if defined(_MSC_VER)&&(defined(_M_X64)||defined(_M_IX86))
 int r[4]={0,0,0,0};
 __cpuid(r,0);
 int max_leaf=r[0];
 if(max_leaf>=1){
  __cpuidex(r,1,0);
  out->sse2=(unsigned)((r[3]>>26)&1);
  out->sse42=(unsigned)((r[2]>>20)&1);
  out->avx_hw=(unsigned)((r[2]>>28)&1);
  out->osxsave=(unsigned)((r[2]>>27)&1);
  out->aes=(unsigned)((r[2]>>25)&1);
  out->virtualization=(unsigned)((r[2]>>5)&1);
  if(out->osxsave){
   unsigned __int64 xcr0=_xgetbv(0);
   out->avx_os_state=(unsigned)((xcr0&0x6u)==0x6u);
   out->avx512_os_state=(unsigned)((xcr0&0xE6u)==0xE6u);
  }
  out->avx=(unsigned)(out->avx_hw&&out->avx_os_state);
  out->fma_hw=(unsigned)((r[2]>>12)&1);
  out->fma=(unsigned)(out->avx&&out->fma_hw);
 }
 if(max_leaf>=7){
  __cpuidex(r,7,0);
  out->avx2_hw=(unsigned)((r[1]>>5)&1);
   out->avx2=(unsigned)(out->avx&&out->avx2_hw);
  out->avx512f_hw=(unsigned)((r[1]>>16)&1);
   out->avx512f=(unsigned)(out->avx&&out->avx512_os_state&&out->avx512f_hw);
 }
#elif(defined(__x86_64__)||defined(__i386__))&&(defined(__GNUC__)||defined(__clang__))
 unsigned a=0,b=0,c=0,d=0;
 if(__get_cpuid(1,&a,&b,&c,&d)){
  out->sse2=(unsigned)((d>>26)&1u);
  out->sse42=(unsigned)((c>>20)&1u);
  out->avx_hw=(unsigned)((c>>28)&1u);
  out->osxsave=(unsigned)((c>>27)&1u);
  out->aes=(unsigned)((c>>25)&1u);
  out->virtualization=(unsigned)((c>>5)&1u);
  if(out->osxsave){
   unsigned int xcr0_lo=0,xcr0_hi=0;
   __asm__ volatile("xgetbv" : "=a"(xcr0_lo),"=d"(xcr0_hi) : "c"(0));
   (void)xcr0_hi;
   out->avx_os_state=(unsigned)((xcr0_lo&0x6u)==0x6u);
   out->avx512_os_state=(unsigned)((xcr0_lo&0xE6u)==0xE6u);
  }
  out->avx=(unsigned)(out->avx_hw&&out->avx_os_state);
  out->fma_hw=(unsigned)((c>>12)&1u);
  out->fma=(unsigned)(out->avx&&out->fma_hw);
 }
 if(__get_cpuid_count(7,0,&a,&b,&c,&d)){
  out->avx2_hw=(unsigned)((b>>5)&1u);
   out->avx2=(unsigned)(out->avx&&out->avx2_hw);
  out->avx512f_hw=(unsigned)((b>>16)&1u);
   out->avx512f=(unsigned)(out->avx&&out->avx512_os_state&&out->avx512f_hw);
 }
#endif
 return 0;}
int pl_processor_benchmark_suite(uint64_t iterations,unsigned memory_mib,unsigned passes,pl_suite_result*out){if(!out||iterations==0||iterations>UINT64_C(100000000)||memory_mib==0||memory_mib>64||passes==0||passes>32)return -1;memset(out,0,sizeof(*out));out->iterations=iterations;out->memory_mib=memory_mib;out->passes=passes;volatile uint64_t state=UINT64_C(0x9e3779b97f4a7c15);double start=processor_now_ms();for(uint64_t i=0;i<iterations;i++){state^=state>>12;state^=state<<25;state^=state>>27;state*=UINT64_C(2685821657736338717);}out->integer_ms=processor_now_ms()-start;out->checksum=(uint64_t)state;size_t bytes=(size_t)memory_mib*1024u*1024u;unsigned char*memory=(unsigned char*)malloc(bytes);if(!memory)return -2;for(size_t i=0;i<bytes;i++)memory[i]=(unsigned char)((i*131u+17u)&0xffu);uint64_t sum=0;start=processor_now_ms();for(unsigned p=0;p<passes;p++)for(size_t i=0;i<bytes;i+=64u){memory[i]=(unsigned char)(memory[i]+(unsigned char)(p+1u));sum+=memory[i];}out->memory_ms=processor_now_ms()-start;out->checksum^=sum;free(memory);volatile double matrix[24][24];start=processor_now_ms();for(unsigned r=0;r<24;r++)for(unsigned c=0;c<24;c++)matrix[r][c]=(double)(r+1u)/(double)(c+1u);double msum=0.0;for(unsigned k=0;k<32;k++)for(unsigned r=0;r<24;r++)for(unsigned c=0;c<24;c++)matrix[r][c]=matrix[r][c]*0.999+(double)((r+c+k)%7u)*0.0001;for(unsigned r=0;r<24;r++)for(unsigned c=0;c<24;c++)msum+=matrix[r][c];out->matrix_ms=processor_now_ms()-start;out->checksum^=(uint64_t)(msum*1000000.0);out->integrity_ok=(out->checksum!=0&&msum>0.0);return 0;}
