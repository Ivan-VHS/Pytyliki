#include "iron_core.h"
#include <stdio.h>
#include <string.h>
#define NO_INDEX ((size_t)-1)
#define COUNT(a) (sizeof(a)/sizeof((a)[0]))
static const pl_iron_marker_info catalog[PL_IRON_MARKER_COUNT]={
 {PL_IRON_BEGIN,"begin iron )","module.begin","root","declarative","Opens a source document only; it does not boot hardware.",0},
 {PL_IRON_CORE,"() core --","core.context","block","declarative","Declares a core scope; processor access remains target-gated.",1},
 {PL_IRON_SYSTEM,"[ system ]-","system.context","block","declarative","Declares system metadata; it does not start services.",1},
 {PL_IRON_STOP,"stop option ;","block.end","delimiter","structural","Closes the most recently opened block.",0},
 {PL_IRON_LEVEL,"] level .","level.context","block","declarative","Does not change privilege or protection state.",1},
 {PL_IRON_SELECTIVE,"{ selective /","selection.filter","block","plan-only","No bus, device or memory operation is issued.",1},
 {PL_IRON_OPERATION,"} operation P)-","operation.unit","block","plan-only","Instructions are not executed by this frontend.",1},
 {PL_IRON_MASSIVE,"?? massive <>","work.massive","block","plan-only","Bulk work is described, not spawned.",1},
 {PL_IRON_START,"< start >?","startup.plan","block","plan-only","No boot vector is transferred and no machine is started.",1},
 {PL_IRON_NEW_WORK,"grown new-work >/","work.spawn.plan","block","plan-only","No threads, jobs or external processes are created.",1},
 {PL_IRON_CONCRETE,"> concrete //*","implementation.binding","block","declarative","Does not link or execute code.",1},
 {PL_IRON_UTILITY,"## utility _","utility.helper","block","pure-declaration","Purity requires later semantic analysis.",1},
 {PL_IRON_UNIFY,"# unify TAB","layout.unify","block","format-only","Does not rewrite source files.",1},
 {PL_IRON_BRACHPASS,"setup brachpass ^^","branchpass.plan","block","analysis-only","No processor predictor state is modified.",1},
 {PL_IRON_TOURNAMENT,"!+ hyper tournament ^","tournament.rank","block","deterministic-plan","Does not select or merge a GitHub change.",1},
 {PL_IRON_CC_RULES,"CC based rules ^^^","consistency.rules","policy","validation-policy","Names a policy; it is not evidence that checks passed.",0},
 {PL_IRON_LEVEL_JUMP,"level jump to run *(","execution.jump.plan","block","plan-only","No jump or execution-level change occurs.",1},
 {PL_IRON_TRUE_CONNECTIVE,"TRUE connective *+","logic.true_connective","block","logic-declaration","Predicates are not evaluated by structural parsing.",1},
 {PL_IRON_VERSION,"verison ///. *","module.version","metadata","declarative","Preserves the historical source spelling.",0},
 {PL_IRON_VERIFICATED,"verificated /// !!","source.verification_claim","metadata","untrusted-claim","A source claim only; CI, signatures and reviews are checked separately.",0}
};
static int space(char c){return c==' '||c=='\t'||c=='\r'||c=='\n';}
static int label_char(unsigned char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='.'||c=='-';}
static int fail(pl_iron_error*e,size_t off,size_t line,const char*m){if(e){snprintf(e->message,sizeof(e->message),"%s",m);e->offset=off;e->line=line;}return 0;}
static size_t bounded_len(const char*s){size_t n=0;while(n<=PL_IRON_MAX_SOURCE_BYTES&&s[n])++n;return n;}
static size_t skip_space(const char*s,size_t p,size_t end){while(p<end&&space(s[p]))++p;return p;}
/* Find first active byte on a line; strings and comments cannot forge markers. */
static int scan_line(const char*s,size_t a,size_t b,int*block,size_t*first,pl_iron_error*e,size_t line){
 size_t i=a,quote_at=0;char q=0;*first=NO_INDEX;
 while(i<b){char c=s[i];
  if(*block){if(c=='*'&&i+1<b&&s[i+1]=='/'){*block=0;i+=2;}else ++i;continue;}
  if(q){if(c=='\\'){if(i+1>=b)return fail(e,i,line,"string escape ends at line boundary");i+=2;}
   else if(c==q){q=0;++i;}else ++i;continue;}
  if(c=='/'&&i+1<b&&s[i+1]=='/')break;
  if(c=='/'&&i+1<b&&s[i+1]=='*'){*block=1;i+=2;continue;}
  if(c=='"'||c=='\''){if(*first==NO_INDEX)*first=i;q=c;quote_at=i;++i;continue;}
  if(*first==NO_INDEX&&!space(c))*first=i;
  ++i;
 }
 if(q)return fail(e,quote_at,line,"unterminated string or character literal");
 return 1;
}
static int matches(const char*s,size_t p,size_t end,const char*t){
 size_t n=strlen(t);if(p+n>end||strncmp(s+p,t,n))return 0;
 return p+n==end||space(s[p+n])||(s[p+n]=='/'&&p+n+1<end&&(s[p+n+1]=='/'||s[p+n+1]=='*'));
}
static int trivia_after(const char*s,size_t p,size_t end){
 p=skip_space(s,p,end);if(p==end)return 1;
 if(p+1<end&&s[p]=='/'&&s[p+1]=='/')return 1;
 if(p+1<end&&s[p]=='/'&&s[p+1]=='*'){p+=2;while(p+1<end&&!(s[p]=='*'&&s[p+1]=='/'))++p;
  if(p+1>=end)return 0;p=skip_space(s,p+2,end);return p==end||(p+1<end&&s[p]=='/'&&s[p+1]=='/');}
 return 0;
}
static int read_label(const char*s,size_t p,size_t end,char*out,size_t line,pl_iron_error*e){
 size_t a,n;p=skip_space(s,p,end);a=p;while(p<end&&label_char((unsigned char)s[p]))++p;n=p-a;
 if(!n)return fail(e,a,line,"marker requires a non-empty identifier");
 if(n>=PL_IRON_MAX_LABEL)return fail(e,a,line,"marker identifier exceeds 95 bytes");
 if(!trivia_after(s,p,end))return fail(e,p,line,"unexpected text after marker identifier");
 memcpy(out,s+a,n);out[n]='\0';return 1;
}
size_t pl_iron_catalog(const pl_iron_marker_info**out){if(out)*out=catalog;return COUNT(catalog);}
const pl_iron_marker_info*pl_iron_marker_for_kind(pl_iron_marker_kind k){return (unsigned)k<PL_IRON_MARKER_COUNT?&catalog[(size_t)k]:NULL;}
int pl_iron_parse(const char*s,pl_iron_program*out,pl_iron_error*e){
 size_t len,line_start=0,line=1,stack[PL_IRON_MAX_NESTING],depth=0;int block_comment=0,final_seal=0;
 if(e){e->message[0]='\0';e->offset=0;e->line=0;}
 if(!s||!out)return fail(e,0,0,"source and output are required");
 len=bounded_len(s);if(len>PL_IRON_MAX_SOURCE_BYTES)return fail(e,PL_IRON_MAX_SOURCE_BYTES,0,"source exceeds one-megabyte limit");
 memset(out,0,sizeof(*out));out->source_bytes=len;
 while(line_start<len){size_t end=line_start,first=NO_INDEX,i;const pl_iron_marker_info*m=NULL;
  while(end<len&&s[end]!='\n')++end;
  if(!scan_line(s,line_start,end,&block_comment,&first,e,line))return 0;
  if(first!=NO_INDEX)for(i=0;i<PL_IRON_MARKER_COUNT;i++)if(matches(s,first,end,catalog[i].token)){m=&catalog[i];break;}
  if(m){
   size_t after=first+strlen(m->token);pl_iron_marker_kind k=m->kind;
   if(final_seal)return fail(e,first,line,"marker appears after final verification claim");
   ++out->marker_counts[(size_t)k];
   if(k==PL_IRON_BEGIN){
    if(out->root_seen)return fail(e,first,line,"duplicate begin iron root marker");
    if(depth||final_seal)return fail(e,first,line,"root marker is outside the module prelude");
    if(!trivia_after(s,after,end))return fail(e,after,line,"begin iron marker must occupy its own line");
    out->root_seen=1;out->root_offset=first;out->root_line=line;
   }else if(k==PL_IRON_STOP){
    if(!trivia_after(s,after,end))return fail(e,after,line,"stop option marker must occupy its own line");
    if(!depth)return fail(e,first,line,"stop option has no matching open block");
    {size_t b=stack[--depth];out->blocks[b].body_length=first-out->blocks[b].body_offset;out->blocks[b].end_offset=first;out->blocks[b].closed=1;}
   }else if(m->opens_block){
    size_t b;if(!out->root_seen)return fail(e,first,line,"block marker appears before begin iron");
    if(final_seal)return fail(e,first,line,"block marker appears after final verification claim");
    if(!trivia_after(s,after,end))return fail(e,after,line,"block marker must occupy its own line");
    if(depth>=PL_IRON_MAX_NESTING)return fail(e,first,line,"Iron block nesting limit exceeded");
    if(out->block_count>=PL_IRON_MAX_BLOCKS)return fail(e,first,line,"Iron block count limit exceeded");
    b=out->block_count++;out->blocks[b].kind=k;out->blocks[b].start_offset=first;
    out->blocks[b].body_offset=end<len?end+1:end;out->blocks[b].line=line;stack[depth++]=b;
   }else if(k==PL_IRON_VERSION){if(!read_label(s,after,end,out->version,line,e))return 0;++out->version_count;
   }else if(k==PL_IRON_VERIFICATED){if(!read_label(s,after,end,out->verification_label,line,e))return 0;
    ++out->verification_count;out->source_seal_asserted=1;
    if(out->root_seen){if(depth)return fail(e,first,line,"verification claim appears before all blocks close");++out->verification_after_root_count;final_seal=1;}
   }else if(k==PL_IRON_CC_RULES){char policy[PL_IRON_MAX_LABEL];if(!read_label(s,after,end,policy,line,e))return 0;++out->policy_count;}
  }
  line_start=end<len?end+1:end;++line;
 }
 if(block_comment)return fail(e,len,line,"unterminated block comment");
 if(depth){size_t b=stack[depth-1];return fail(e,out->blocks[b].start_offset,out->blocks[b].line,"Iron block is missing stop option ;");}
 if(!out->root_seen)return fail(e,0,1,"missing begin iron root marker");
 if(!out->version_count)return fail(e,0,1,"missing verison metadata marker");
 if(!out->policy_count)return fail(e,0,1,"missing CC based rules policy marker");
 if(!out->verification_count||!out->verification_after_root_count)return fail(e,len,line,"missing final verificated source claim");
 if(!out->block_count)return fail(e,out->root_offset,out->root_line,"Iron module must contain at least one block");
 return 1;
}
int pl_iron_write_json(const pl_iron_program*p,FILE*f){
 size_t i;if(!p||!f)return 0;
 if(fprintf(f,"{\n  \"schema\": \"%s\",\n  \"source_bytes\": %zu,\n  \"root_seen\": %s,\n  \"version\": \"%s\",\n  \"version_declarations\": %zu,\n  \"policy_declarations\": %zu,\n  \"source_seal_asserted\": %s,\n  \"verification_claim\": \"%s\",\n  \"verification_claims_are_ci_evidence\": false,\n  \"semantic_validation_performed\": false,\n  \"execution_performed\": false,\n  \"hardware_access_performed\": false,\n  \"blocks\": [\n",PL_IRON_AST_SCHEMA,p->source_bytes,p->root_seen?"true":"false",p->version,p->version_count,p->policy_count,p->source_seal_asserted?"true":"false",p->verification_label)<0)return 0;
 for(i=0;i<p->block_count;i++){const pl_iron_block*b=&p->blocks[i];const pl_iron_marker_info*m=pl_iron_marker_for_kind(b->kind);
  if(fprintf(f,"    {\"marker\":\"%s\",\"semantic_id\":\"%s\",\"start_offset\":%zu,\"body_offset\":%zu,\"body_length\":%zu,\"end_offset\":%zu,\"line\":%zu,\"closed\":%s}%s\n",m->token,m->semantic_id,b->start_offset,b->body_offset,b->body_length,b->end_offset,b->line,b->closed?"true":"false",i+1==p->block_count?"":",")<0)return 0;}
 if(fputs("  ],\n  \"marker_counts\": {\n",f)==EOF)return 0;
 for(i=0;i<PL_IRON_MARKER_COUNT;i++)if(fprintf(f,"    \"%s\": %zu%s\n",catalog[i].semantic_id,p->marker_counts[i],i+1==PL_IRON_MARKER_COUNT?"":",")<0)return 0;
 return fputs("  }\n}\n",f)!=EOF;
}
int pl_iron_write_catalog_json(FILE*f){
 size_t i;if(!f)return 0;if(fputs("{\n  \"schema\": \"pytyliki.iron-core-catalog.v1\",\n  \"count\": 20,\n  \"markers\": [\n",f)==EOF)return 0;
 for(i=0;i<PL_IRON_MARKER_COUNT;i++){const pl_iron_marker_info*m=&catalog[i];
  if(fprintf(f,"    {\"token\":\"%s\",\"semantic_id\":\"%s\",\"category\":\"%s\",\"effect_class\":\"%s\",\"opens_block\":%s,\"safety_contract\":\"%s\"}%s\n",m->token,m->semantic_id,m->category,m->effect_class,m->opens_block?"true":"false",m->safety_contract,i+1==PL_IRON_MARKER_COUNT?"":",")<0)return 0;}
 return fputs("  ]\n}\n",f)!=EOF;
}
