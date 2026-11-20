#include "embedded.h"
#include <string.h>
static const pl_embedded_target targets[] = {
    {"native-c11","host-native","host C ABI","cc","Portable host-side C11 development and test profile",0u,0u},
    {"avr-atmega328p","AVR8","avr-gcc ABI","avr-gcc","8-bit AVR target profile for ATmega328P",8u,PL_EMBEDDED_CAP_FREESTANDING|PL_EMBEDDED_CAP_HARVARD},
    {"arm-cortex-m0","ARM Cortex-M0","AAPCS","arm-none-eabi-gcc","32-bit Cortex-M0 bare-metal profile",32u,PL_EMBEDDED_CAP_FREESTANDING|PL_EMBEDDED_CAP_RTOS_COMMON},
    {"riscv32-imac","RISC-V32","ILP32","riscv64-unknown-elf-gcc","32-bit RISC-V IMAC embedded profile",32u,PL_EMBEDDED_CAP_FREESTANDING|PL_EMBEDDED_CAP_RTOS_COMMON},
    {"msp430-g2553","MSP430","MSP430 EABI","msp430-elf-gcc","16-bit MSP430G2553 microcontroller profile",16u,PL_EMBEDDED_CAP_FREESTANDING|PL_EMBEDDED_CAP_HARVARD},
    {"xtensa-esp32","Xtensa LX6","ESP-IDF Xtensa ABI","xtensa-esp32-elf-gcc","32-bit ESP32 toolchain profile; board and SDK configuration remain external",32u,PL_EMBEDDED_CAP_FREESTANDING|PL_EMBEDDED_CAP_RTOS_COMMON},
    {"wasm32-wasi","WebAssembly32","WASI","clang","Portable WebAssembly C target with an explicit WASI runtime boundary",32u,PL_EMBEDDED_CAP_SANDBOXED}
};
static size_t count_token(const char *source,size_t length,const char *token) {
    size_t i,count=0u,token_length=strlen(token);
    if(token_length==0u||token_length>length)return 0u;
    for(i=0u;i+token_length<=length;++i) {
        if(memcmp(source+i,token,token_length)==0) { ++count; i+=token_length-1u; }
    }
    return count;
}
size_t pl_embedded_target_count(void) { return sizeof(targets)/sizeof(targets[0]); }
int pl_embedded_target_at(size_t index,const pl_embedded_target **out) {
    if(out==NULL||index>=pl_embedded_target_count())return -1;
    *out=&targets[index]; return 0;
}
int pl_embedded_target_find(const char *id,const pl_embedded_target **out) {
    size_t i;
    if(id==NULL||out==NULL)return -1;
    for(i=0u;i<pl_embedded_target_count();++i) {
        if(strcmp(id,targets[i].id)==0) { *out=&targets[i]; return 0; }
    }
    return -1;
}
int pl_embedded_inspect_source(const char *source,size_t length,pl_embedded_source_report *out) {
    size_t i;
    if(source==NULL||out==NULL||length>PL_EMBEDDED_MAX_SOURCE_BYTES)return -1;
    memset(out,0,sizeof(*out));
    out->source_bytes=length;
    if(length!=0u)out->line_count=1u;
    for(i=0u;i<length;++i)if(source[i]=='\n')++out->line_count;
    out->include_count=count_token(source,length,"#include");
    out->heap_calls=count_token(source,length,"malloc(")+count_token(source,length,"calloc(")+
        count_token(source,length,"realloc(")+count_token(source,length,"free(");
    out->blocking_calls=count_token(source,length,"sleep(")+count_token(source,length,"usleep(")+
        count_token(source,length,"HAL_Delay(")+count_token(source,length,"vTaskDelay(");
    out->register_markers=count_token(source,length,"volatile")+count_token(source,length,"_MMIO")+
        count_token(source,length,"GPIO")+count_token(source,length,"PORT")+count_token(source,length,"REG_");
    out->host_io_calls=count_token(source,length,"printf(")+count_token(source,length,"fprintf(")+
        count_token(source,length,"fopen(");
    if(out->heap_calls!=0u)out->warnings|=PL_EMBEDDED_WARN_DYNAMIC_ALLOCATION;
    if(out->blocking_calls!=0u)out->warnings|=PL_EMBEDDED_WARN_BLOCKING_CALLS;
    if(out->register_markers!=0u)out->warnings|=PL_EMBEDDED_WARN_REGISTER_ACCESS;
    if(out->host_io_calls!=0u)out->warnings|=PL_EMBEDDED_WARN_HOST_IO;
    return 0;
}
