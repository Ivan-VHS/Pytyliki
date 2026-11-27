#include "iron_syntax.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *const records[PL_IRON_SYNTAX_RECORD_COUNT] = {
 "begin iron )","() core --","[ system ]-","stop option ;","] level .",
 "{ selective /","} operation P)-","?? massive <>","< start >?",
 "grown new-work >/","> concrete //*","## utility _","# unify TAB",
 "setup brachpass ^^","!+ hyper tournament ^","CC based rules ^^^",
 "level jump to run *(","TRUE connective *+","verison ///. *","verificated /// !!"
};
static void test_catalog(void) {
 const pl_iron_syntax_info *catalog=NULL;
 size_t count=pl_iron_syntax_catalog(&catalog),index;
 assert(count==PL_IRON_SYNTAX_RECORD_COUNT&&catalog!=NULL);
 for(index=0u;index<count;++index) {
  assert(catalog[index].kind==(pl_iron_syntax_kind)index);
  assert(strcmp(catalog[index].phrase,records[index])==0);
  assert(catalog[index].name[0]!='\0'&&catalog[index].concept[0]!='\0');
  assert(catalog[index].marker[0]!='\0'&&catalog[index].family[0]!='\0');
 }
}
static void test_all_records(void) {
 size_t index;
 for(index=0u;index<PL_IRON_SYNTAX_RECORD_COUNT;++index) {
  char source[96]; pl_iron_syntax_node storage[4];
  pl_iron_syntax_program program; pl_iron_syntax_error error;
  (void)snprintf(source,sizeof(source),"%s\n",records[index]);
  assert(pl_iron_syntax_parse(source,&program,storage,4u,&error));
  assert(program.length==1u&&storage[0].kind==(pl_iron_syntax_kind)index);
  assert(storage[0].line==1u&&storage[0].column==1u);
  assert(storage[0].source_length==strlen(records[index]));
 }
}
static void test_whitespace_and_lines(void) {
 const char *source="\tbegin\t iron\t )  \r\n\r\n#\tunify\tTAB\nTRUE\tconnective\t*+";
 pl_iron_syntax_node storage[8]; pl_iron_syntax_program program; pl_iron_syntax_error error;
 assert(pl_iron_syntax_parse(source,&program,storage,8u,&error));
 assert(program.length==3u);
 assert(storage[0].kind==PL_IRON_BEGIN_IRON&&storage[0].column==2u);
 assert(storage[1].kind==PL_IRON_UNIFY&&storage[1].line==3u);
 assert(storage[2].kind==PL_IRON_TRUE_CONNECTIVE&&storage[2].line==4u);
}
static void test_invalid_forms(void) {
 static const char *const invalid[]={"begin iron","begin iron ) extra","Begin iron )",
  "setup branchpass ^^","verizon ///. *","# unify tab","// comment","?? massive <",
  "begin iron ) () core --"};
 size_t index;
 for(index=0u;index<sizeof(invalid)/sizeof(invalid[0]);++index) {
  pl_iron_syntax_node storage[4]; pl_iron_syntax_program program; pl_iron_syntax_error error;
  assert(!pl_iron_syntax_parse(invalid[index],&program,storage,4u,&error));
  assert(error.code[0]!='\0'&&error.message[0]!='\0');
 }
}
static void test_duplicates_keep_order(void) {
 const char *source="begin iron )\nbegin iron )\nstop option ;\n";
 pl_iron_syntax_node storage[4]; pl_iron_syntax_program program; pl_iron_syntax_error error;
 assert(pl_iron_syntax_parse(source,&program,storage,4u,&error)&&program.length==3u);
 assert(storage[0].kind==PL_IRON_BEGIN_IRON&&storage[1].kind==PL_IRON_BEGIN_IRON);
 assert(storage[2].kind==PL_IRON_STOP_OPTION);
}
static void test_limits_and_nulls(void) {
 pl_iron_syntax_node storage[1]; pl_iron_syntax_program program; pl_iron_syntax_error error;
 char *oversized=(char *)malloc(PL_IRON_SYNTAX_MAX_SOURCE_BYTES+2u);
 assert(oversized!=NULL); memset(oversized,'x',PL_IRON_SYNTAX_MAX_SOURCE_BYTES+1u);
 oversized[PL_IRON_SYNTAX_MAX_SOURCE_BYTES+1u]='\0';
 assert(!pl_iron_syntax_parse(oversized,&program,storage,1u,&error));
 assert(strcmp(error.code,"PYT-IRON-0002")==0); free(oversized);
 assert(!pl_iron_syntax_parse(" \n\t\n",&program,storage,1u,&error));
 assert(strcmp(error.code,"PYT-IRON-0005")==0);
 assert(!pl_iron_syntax_parse(NULL,&program,storage,1u,&error));
 assert(!pl_iron_syntax_parse("begin iron )\nstop option ;",&program,storage,1u,&error));
 assert(strcmp(error.code,"PYT-IRON-0003")==0);
}
static void test_json_is_syntax_only(void) {
 const char *source="begin iron )\n# unify TAB\n";
 pl_iron_syntax_node storage[4]; pl_iron_syntax_program program; pl_iron_syntax_error error;
 FILE *stream=tmpfile(); char buffer[2048]; size_t count;
 assert(stream!=NULL&&pl_iron_syntax_parse(source,&program,storage,4u,&error));
 assert(pl_iron_syntax_write_json(&program,stream)); rewind(stream);
 count=fread(buffer,1u,sizeof(buffer)-1u,stream); buffer[count]='\0';
 assert(strstr(buffer,PL_IRON_SYNTAX_SCHEMA)!=NULL);
 assert(strstr(buffer,"\"execution_performed\":false")!=NULL);
 assert(strstr(buffer,"\"phrase\":\"# unify TAB\"")!=NULL);
 fclose(stream);
}
int main(void) {
 test_catalog(); test_all_records(); test_whitespace_and_lines(); test_invalid_forms();
 test_duplicates_keep_order(); test_limits_and_nulls(); test_json_is_syntax_only();
 puts("Pytyliki Iron Syntax v1 tests passed"); return 0;
}
