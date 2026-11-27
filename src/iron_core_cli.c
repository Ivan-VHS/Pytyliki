#include "iron_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static FILE *open_source_file(const char *path) {
    FILE *file = NULL;
#if defined(_MSC_VER)
    if (fopen_s(&file, path, "rb") != 0) {
        return NULL;
    }
#else
    file = fopen(path, "rb");
#endif
    return file;
}

static char*read_file(const char*path){
 FILE*f=open_source_file(path);long n;size_t got;char*s;if(!f)return NULL;
 if(fseek(f,0,SEEK_END)){fclose(f);return NULL;}n=ftell(f);
 if(n<0||(unsigned long)n>PL_IRON_MAX_SOURCE_BYTES||fseek(f,0,SEEK_SET)){fclose(f);return NULL;}
 s=(char*)malloc((size_t)n+1);if(!s){fclose(f);return NULL;}got=fread(s,1,(size_t)n,f);
 if(ferror(f)||got!=(size_t)n||memchr(s,'\0',got)){free(s);fclose(f);return NULL;}
 s[got]='\0';fclose(f);return s;
}
static void usage(FILE*f){fputs("pytyliki-iron --version | --catalog [--json] | --check FILE | --ast-json FILE\n",f);}
int main(int argc,char**argv){
 if(argc==2&&!strcmp(argv[1],"--version")){puts("Pytyliki Iron Core structural validator 1.0.0");return 0;}
 if(argc==2&&!strcmp(argv[1],"--catalog")){const pl_iron_marker_info*a;size_t i,n=pl_iron_catalog(&a);for(i=0;i<n;i++)printf("%-22s %-28s %-18s\n",a[i].token,a[i].semantic_id,a[i].effect_class);return 0;}
 if(argc==3&&!strcmp(argv[1],"--catalog")&&!strcmp(argv[2],"--json"))return pl_iron_write_catalog_json(stdout)?0:1;
 if(argc!=3||(strcmp(argv[1],"--check")&&strcmp(argv[1],"--ast-json"))){usage(stderr);return 2;}
 {char*s=read_file(argv[2]);pl_iron_program p;pl_iron_error e;int rc=0;
  if(!s){fprintf(stderr,"pytyliki-iron: cannot read %s (1 MiB max; embedded NUL rejected)\n",argv[2]);return 2;}
  if(!pl_iron_parse(s,&p,&e)){fprintf(stderr,"%s:%zu: Iron syntax error at byte %zu: %s\n",argv[2],e.line,e.offset,e.message);free(s);return 1;}
  if(!strcmp(argv[1],"--ast-json")){if(!pl_iron_write_json(&p,stdout))rc=1;}
  else printf("Iron Core source structurally valid: %zu blocks, %zu bytes; no semantic validation or execution performed\n",p.block_count,p.source_bytes);
  free(s);return rc;
 }
}
