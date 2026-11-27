#include "iron_syntax.h"
#include <string.h>
#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))
static const pl_iron_syntax_info syntax_catalog[PL_IRON_SYNTAX_RECORD_COUNT] = {
 {PL_IRON_BEGIN_IRON,"begin_iron","begin iron )","iron",")","boundary"},
 {PL_IRON_CORE,"core","() core --","core","--","section"},
 {PL_IRON_SYSTEM,"system","[ system ]-","system","]-","section"},
 {PL_IRON_STOP_OPTION,"stop_option","stop option ;","option",";","control"},
 {PL_IRON_LEVEL,"level","] level .","level",".","control"},
 {PL_IRON_SELECTIVE,"selective","{ selective /","selective","/","control"},
 {PL_IRON_OPERATION,"operation","} operation P)-","operation","P)-","control"},
 {PL_IRON_MASSIVE,"massive","?? massive <>","massive","<>","modifier"},
 {PL_IRON_START,"start","< start >?","start",">?","boundary"},
 {PL_IRON_NEW_WORK,"new_work","grown new-work >/","new-work",">/","modifier"},
 {PL_IRON_CONCRETE,"concrete","> concrete //*","concrete","//*","modifier"},
 {PL_IRON_UTILITY,"utility","## utility _","utility","_","directive"},
 {PL_IRON_UNIFY,"unify","# unify TAB","unify","TAB","directive"},
 {PL_IRON_SETUP_BRACHPASS,"setup_brachpass","setup brachpass ^^","brachpass","^^","directive"},
 {PL_IRON_HYPER_TOURNAMENT,"hyper_tournament","!+ hyper tournament ^","hyper tournament","^","modifier"},
 {PL_IRON_BASED_RULES,"based_rules","CC based rules ^^^","based rules","^^^","directive"},
 {PL_IRON_LEVEL_JUMP_TO_RUN,"level_jump_to_run","level jump to run *(","jump to run","*(","control"},
 {PL_IRON_TRUE_CONNECTIVE,"true_connective","TRUE connective *+","connective","*+","operator"},
 {PL_IRON_VERISON,"verison","verison ///. *","///.","*","metadata"},
 {PL_IRON_VERIFICATED,"verificated","verificated /// !!","///","!!","metadata"}
};
static int is_space(char value) { return value==' '||value=='\t'||value=='\v'||value=='\f'; }
static int is_break(char value) { return value=='\n'||value=='\r'; }
static void set_error(pl_iron_syntax_error *error,const char *code,const char *message,
                      size_t offset,size_t line,size_t column) {
 if(error==NULL)return;
 (void)snprintf(error->code,sizeof(error->code),"%s",code);
 (void)snprintf(error->message,sizeof(error->message),"%s",message);
 error->offset=offset; error->line=line; error->column=column;
}
static int match_phrase(const char *source,size_t start,size_t end,const char *phrase) {
 size_t input=start,expected=0u;
 while(phrase[expected]!='\0') {
  size_t expected_length=0u,input_length=0u;
  while(phrase[expected]!='\0'&&is_space(phrase[expected]))++expected;
  while(phrase[expected+expected_length]!='\0'&&!is_space(phrase[expected+expected_length]))++expected_length;
  while(input<end&&is_space(source[input]))++input;
  while(input+input_length<end&&!is_space(source[input+input_length]))++input_length;
  if(expected_length==0u||input_length!=expected_length||input+input_length>end||
     memcmp(source+input,phrase+expected,expected_length)!=0)return 0;
  input+=input_length; expected+=expected_length;
 }
 while(input<end&&is_space(source[input]))++input;
 return input==end;
}
size_t pl_iron_syntax_catalog(const pl_iron_syntax_info **out) {
 if(out!=NULL)*out=syntax_catalog;
 return ARRAY_COUNT(syntax_catalog);
}
const pl_iron_syntax_info *pl_iron_syntax_get(pl_iron_syntax_kind kind) {
 if((unsigned)kind>=PL_IRON_SYNTAX_RECORD_COUNT)return NULL;
 return &syntax_catalog[(size_t)kind];
}
int pl_iron_syntax_parse(const char *source,pl_iron_syntax_program *out,
                         pl_iron_syntax_node *storage,size_t storage_capacity,
                         pl_iron_syntax_error *error) {
 size_t source_length=0u,offset=0u,line_number=1u,record_count=0u;
 if(error!=NULL)memset(error,0,sizeof(*error));
 if(source==NULL||out==NULL||storage==NULL||storage_capacity==0u) {
  set_error(error,"PYT-IRON-0001","source, output, and record storage are required",0u,1u,1u);
  return 0;
 }
 while(source_length<=PL_IRON_SYNTAX_MAX_SOURCE_BYTES&&source[source_length]!='\0')++source_length;
 if(source_length>PL_IRON_SYNTAX_MAX_SOURCE_BYTES) {
  set_error(error,"PYT-IRON-0002","Iron source exceeds the 64 KiB limit",
            PL_IRON_SYNTAX_MAX_SOURCE_BYTES,1u,PL_IRON_SYNTAX_MAX_SOURCE_BYTES+1u);
  return 0;
 }
 out->nodes=storage; out->length=0u; out->source_length=source_length;
 while(offset<source_length) {
  size_t line_start=offset,content_start,content_end,candidate,line_length;
  int matched=0;
  while(offset<source_length&&!is_break(source[offset]))++offset;
  content_end=offset;
  if(offset<source_length&&source[offset]=='\r') {
   ++offset; if(offset<source_length&&source[offset]=='\n')++offset;
  } else if(offset<source_length&&source[offset]=='\n')++offset;
  line_length=content_end-line_start; content_start=line_start;
  while(content_start<content_end&&is_space(source[content_start]))++content_start;
  while(content_end>content_start&&is_space(source[content_end-1u]))--content_end;
  if(content_start!=content_end) {
   for(candidate=0u;candidate<ARRAY_COUNT(syntax_catalog);++candidate) {
    if(match_phrase(source,content_start,content_end,syntax_catalog[candidate].phrase)) {
     if(record_count>=storage_capacity||record_count>=PL_IRON_SYNTAX_MAX_RECORDS) {
      set_error(error,"PYT-IRON-0003","Iron record count exceeds the configured limit",
                content_start,line_number,content_start-line_start+1u);
      return 0;
     }
     storage[record_count].kind=syntax_catalog[candidate].kind;
     storage[record_count].source_offset=line_start;
     storage[record_count].source_length=line_length;
     storage[record_count].line=line_number;
     storage[record_count].column=content_start-line_start+1u;
     ++record_count; matched=1; break;
    }
   }
   if(!matched) {
    set_error(error,"PYT-IRON-0004","line does not match an exact Iron Syntax v1 record",
              content_start,line_number,content_start-line_start+1u);
    return 0;
   }
  }
  ++line_number;
 }
 if(record_count==0u) {
  set_error(error,"PYT-IRON-0005","Iron source must contain at least one record",0u,1u,1u);
  return 0;
 }
 out->length=record_count;
 return 1;
}
static int write_json_string(FILE *output,const char *value) {
 const unsigned char *cursor=(const unsigned char *)value;
 if(fputc('"',output)==EOF)return 0;
 while(*cursor!=0u) {
  if(*cursor==(unsigned char)'"'||*cursor==(unsigned char)'\\') {
   if(fputc('\\',output)==EOF||fputc((int)*cursor,output)==EOF)return 0;
  } else if(*cursor<0x20u) {
   if(fprintf(output,"\\u%04x",(unsigned)*cursor)<0)return 0;
  } else if(fputc((int)*cursor,output)==EOF)return 0;
  ++cursor;
 }
 return fputc('"',output)!=EOF;
}
int pl_iron_syntax_write_json(const pl_iron_syntax_program *program,FILE *output) {
 size_t index;
 if(program==NULL||output==NULL||program->nodes==NULL||program->length==0u||
    program->length>PL_IRON_SYNTAX_MAX_RECORDS)return 0;
 if(fprintf(output,"{\"schema\":\"%s\",\"version\":\"%s\","
                   "\"canonical_owner\":\"Pytyliki\","
                   "\"execution_performed\":false,\"records\":[",
            PL_IRON_SYNTAX_SCHEMA,PL_IRON_SYNTAX_VERSION)<0)return 0;
 for(index=0u;index<program->length;++index) {
  const pl_iron_syntax_node *node=&program->nodes[index];
  const pl_iron_syntax_info *info=pl_iron_syntax_get(node->kind);
  if(info==NULL)return 0;
  if(index!=0u&&fputc(',',output)==EOF)return 0;
  if(fputs("{\"kind\":",output)==EOF||!write_json_string(output,info->name)||
     fputs(",\"phrase\":",output)==EOF||!write_json_string(output,info->phrase)||
     fputs(",\"concept\":",output)==EOF||!write_json_string(output,info->concept)||
     fputs(",\"marker\":",output)==EOF||!write_json_string(output,info->marker)||
     fputs(",\"family\":",output)==EOF||!write_json_string(output,info->family)||
     fprintf(output,",\"record_id\":%u,\"source_offset\":%zu,"
                    "\"source_length\":%zu,\"line\":%zu,\"column\":%zu}",
             (unsigned)node->kind,node->source_offset,node->source_length,node->line,node->column)<0)return 0;
 }
 return fputs("]}\n",output)!=EOF;
}
int pl_iron_syntax_write_catalog_json(FILE *output) {
 size_t index;
 if(output==NULL)return 0;
 if(fprintf(output,"{\"schema\":\"%s\",\"records\":[",PL_IRON_SYNTAX_SCHEMA)<0)return 0;
 for(index=0u;index<ARRAY_COUNT(syntax_catalog);++index) {
  const pl_iron_syntax_info *info=&syntax_catalog[index];
  if(index!=0u&&fputc(',',output)==EOF)return 0;
  if(fprintf(output,"{\"record_id\":%u,\"name\":",(unsigned)info->kind)<0||
     !write_json_string(output,info->name)||fputs(",\"phrase\":",output)==EOF||
     !write_json_string(output,info->phrase)||fputs(",\"concept\":",output)==EOF||
     !write_json_string(output,info->concept)||fputs(",\"marker\":",output)==EOF||
     !write_json_string(output,info->marker)||fputs(",\"family\":",output)==EOF||
     !write_json_string(output,info->family)||fputc('}',output)==EOF)return 0;
 }
 return fputs("]}\n",output)!=EOF;
}
