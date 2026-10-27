#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include "sha256.h"
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define CAP_MAX (4u * 1024u * 1024u)
struct buf { char *p; size_t n, cap; };
static int assertions = 0;
static int failures = 0;
static void check(bool ok, const char *label) { ++assertions; if (!ok) { ++failures; fprintf(stderr,"FAIL: %s\n",label); } }
static bool append(struct buf *b,const char *p,size_t n){if(b->n+n+1u>CAP_MAX)return false;if(b->n+n+1u>b->cap){size_t c=b->cap?b->cap:1024u;while(c<b->n+n+1u)c*=2u;char*q=realloc(b->p,c);if(!q)return false;b->p=q;b->cap=c;}memcpy(b->p+b->n,p,n);b->n+=n;b->p[b->n]='\0';return true;}
static int capture(char *const argv[],struct buf *out){int fd[2];if(pipe(fd)!=0)return -1;pid_t p=fork();if(p<0){close(fd[0]);close(fd[1]);return -1;}if(p==0){close(fd[0]);dup2(fd[1],STDOUT_FILENO);dup2(fd[1],STDERR_FILENO);close(fd[1]);execv(argv[0],argv);_exit(127);}close(fd[1]);char b[4096];for(;;){ssize_t n=read(fd[0],b,sizeof(b));if(n<0){if(errno==EINTR)continue;close(fd[0]);return -1;}if(n==0)break;if(!append(out,b,(size_t)n)){close(fd[0]);return -1;}}close(fd[0]);int s=0;if(waitpid(p,&s,0)<0)return -1;if(WIFEXITED(s))return WEXITSTATUS(s);if(WIFSIGNALED(s))return 128+WTERMSIG(s);return 125;}
static bool read_file(const char *path,struct buf *b){FILE*f=fopen(path,"rb");if(!f)return false;char x[4096];for(;;){size_t n=fread(x,1u,sizeof(x),f);if(n&&!append(b,x,n)){fclose(f);return false;}if(n<sizeof(x)){bool ok=!ferror(f);fclose(f);return ok;}}}
static bool exists(const char*p){struct stat s;return stat(p,&s)==0;}
static bool contains(const char *hay,const char *needle){return hay&&strstr(hay,needle)!=NULL;}
static bool join(char*out,size_t n,const char*a,const char*b){int z=snprintf(out,n,"%s/%s",a,b);return z>=0&&(size_t)z<n;}
static bool hash_file(const char *path,char hex[65],uint64_t *size){FILE*f=fopen(path,"rb");if(!f)return false;sha256_ctx c;sha256_init(&c);uint8_t b[8192];uint64_t total=0;for(;;){size_t n=fread(b,1u,sizeof(b),f);if(n){sha256_update(&c,b,n);total+=(uint64_t)n;}if(n<sizeof(b)){if(ferror(f)){fclose(f);return false;}break;}}fclose(f);uint8_t d[32];sha256_final(&c,d);static const char h[]="0123456789abcdef";for(size_t i=0;i<32;i++){hex[2*i]=h[d[i]>>4];hex[2*i+1]=h[d[i]&15];}hex[64]=0;*size=total;return true;}
static bool verify_manifest(const char *dir){char p[PATH_MAX];if(!join(p,sizeof(p),dir,"SHA256SUMS.txt"))return false;FILE*f=fopen(p,"r");if(!f)return false;char line[1024];bool all=true;size_t rows=0;while(fgets(line,sizeof(line),f)){if(strlen(line)<67u||line[64]!=' '||line[65]!=' ') {all=false;break;}char expected[65];memcpy(expected,line,64u);expected[64]=0;char *name=line+66;name[strcspn(name,"\r\n")]=0;char fp[PATH_MAX],actual[65];uint64_t size;if(!join(fp,sizeof(fp),dir,name)||!hash_file(fp,actual,&size)||strcmp(expected,actual)!=0){all=false;break;}++rows;}fclose(f);return all&&rows==9u;}
static int validate_run(const char *dir,const char *marker,int expected,const char *status,const char *evidence_status){char p[PATH_MAX];struct buf f={0};
#define READ_CHECK(file,label) do { f.n=0;if(f.p)f.p[0]=0;check(join(p,sizeof(p),dir,file)&&read_file(p,&f),label); } while(0)
    READ_CHECK("FRAME.json","FRAME.json exists and is readable");check(contains(f.p,"K501-CONTROLLED-RUN-0.1")&&contains(f.p,"command_argv"),"FRAME binds protocol and argv");
    READ_CHECK("RESULT.txt","RESULT.txt exists and is readable");char code[64];snprintf(code,sizeof(code),"exit_code=%d",expected);check(contains(f.p,code),"RESULT records actual exit code");
    READ_CHECK("STATUS.txt","STATUS.txt exists and is readable");check(contains(f.p,status),"STATUS records process exit class");
    READ_CHECK("TRANSCRIPT.log","TRANSCRIPT.log exists and is readable");check(contains(f.p,marker),"TRANSCRIPT captures child output");
    READ_CHECK("EVIDENCE_RECORD.json","normalized Evidence Record exists");check(contains(f.p,"\"schema_version\": \"0.1.0\"")&&contains(f.p,"NO_GATE_EFFECT_UNTIL_REVIEW"),"Evidence Record version and gate boundary present");char expected_status[64];snprintf(expected_status,sizeof(expected_status),"\"status\": \"%s\"",evidence_status);check(contains(f.p,expected_status)&&contains(f.p,"\"actual_exit_code\""),"check outcome conforms to expected child code");
    check(verify_manifest(dir),"SHA256SUMS verifies every evidence file");free(f.p);return failures?1:0;
#undef READ_CHECK
}

int main(int argc,char **argv){
    if(argc>1&&strcmp(argv[1],"--child-emit")==0){puts("K501_C_RECORDER_CHILD_PASS");return 0;}
    if(argc>2&&strcmp(argv[1],"--child-exit")==0)return atoi(argv[2]);
    if(argc<2){fprintf(stderr,"test-runtime requires application path\n");return 64;}
    char app[PATH_MAX];if(realpath(argv[1],app)==NULL){perror("realpath app");return 2;}
    struct buf out={0};char *noargs[]={app,NULL};int rc=capture(noargs,&out);check(rc==64,"no-argument invocation returns usage exit 64");free(out.p);out.p=NULL;out.n=out.cap=0;
    char *self[]={app,"self-test",NULL};rc=capture(self,&out);check(rc==0&&contains(out.p,"SELF_TEST=PASS"),"self-test passes SHA256 and QH256 module tests");free(out.p);out.p=NULL;out.n=out.cap=0;
    char root[]="/tmp/k501-proof-evidence-test-XXXXXX";check(mkdtemp(root)!=NULL,"isolated test workspace created");
    char no_runs[PATH_MAX];join(no_runs,sizeof(no_runs),root,"runs");check(!exists(no_runs),"invalid invocation did not create run evidence");
    char this_exe[PATH_MAX];if(realpath(argv[0],this_exe)==NULL)snprintf(this_exe,sizeof(this_exe),"%s",argv[0]);
    char *success[]={app,"run",root,"system","native-c-success","--",this_exe,"--child-emit",NULL};rc=capture(success,&out);check(rc==0,"successful child exit is returned unchanged");
    char runs_system[PATH_MAX];join(runs_system,sizeof(runs_system),root,"runs/system");DIR*d=opendir(runs_system);char success_dir[PATH_MAX]="";if(d){struct dirent*e;while((e=readdir(d))!=NULL){if(strstr(e->d_name,"_native-c-success")!=NULL){join(success_dir,sizeof(success_dir),runs_system,e->d_name);break;}}closedir(d);}check(success_dir[0]!=0,"success run has a unique run directory");if(success_dir[0])validate_run(success_dir,"K501_C_RECORDER_CHILD_PASS",0,"EXIT_ZERO","PASS");
    out.n=0;if(out.p)out.p[0]=0;
    char *negative[]={app,"run",root,"system","native-c-expected-23","--expect-exit","23","--",this_exe,"--child-exit","23",NULL};rc=capture(negative,&out);check(rc==23,"expected nonzero child exit is returned unchanged");
    char negative_dir[PATH_MAX]="";d=opendir(runs_system);if(d){struct dirent*e;while((e=readdir(d))!=NULL){if(strstr(e->d_name,"_native-c-expected-23")!=NULL){join(negative_dir,sizeof(negative_dir),runs_system,e->d_name);break;}}closedir(d);}check(negative_dir[0]!=0,"expected-failure run has a unique run directory");if(negative_dir[0])validate_run(negative_dir,"",23,"NONZERO_EXIT","PASS");
    out.n=0;if(out.p)out.p[0]=0;
    char *mismatch[]={app,"run",root,"system","native-c-exit-mismatch","--",this_exe,"--child-exit","23",NULL};rc=capture(mismatch,&out);check(rc==23,"unexpected child exit is returned unchanged");
    char mismatch_dir[PATH_MAX]="";d=opendir(runs_system);if(d){struct dirent*e;while((e=readdir(d))!=NULL){if(strstr(e->d_name,"_native-c-exit-mismatch")!=NULL){join(mismatch_dir,sizeof(mismatch_dir),runs_system,e->d_name);break;}}closedir(d);}check(mismatch_dir[0]!=0,"exit-mismatch run has a unique run directory");if(mismatch_dir[0])validate_run(mismatch_dir,"",23,"NONZERO_EXIT","FAIL");
    free(out.p); out.p=NULL;
    if (argc >= 3 && argv[2][0] != '\0') {
        char runtime[PATH_MAX];
        if (realpath(argv[2],runtime)==NULL) { check(false,"optional K501 runtime binary resolves to an exact file"); }
        else {
            char *integration[]={app,"run",root,"pipeline","runtime-alpha1-qh256","--",runtime,"qh256-test",NULL};
            struct buf integration_out={0}; int irc=capture(integration,&integration_out);
            check(irc==0,"existing K501 Runtime qh256-test runs as a distinct child process");
            char pipeline_dir[PATH_MAX]; join(pipeline_dir,sizeof(pipeline_dir),root,"runs/pipeline");
            DIR *pd=opendir(pipeline_dir); char integration_dir[PATH_MAX]="";
            if(pd){struct dirent*e;while((e=readdir(pd))!=NULL){if(strstr(e->d_name,"_runtime-alpha1-qh256")!=NULL){join(integration_dir,sizeof(integration_dir),pipeline_dir,e->d_name);break;}}closedir(pd);}
            check(integration_dir[0]!=0,"runtime integration evidence run created");
            if(integration_dir[0]) validate_run(integration_dir,"QH256 CORE TEST PASS",0,"EXIT_ZERO","PASS");
            free(integration_out.p);
        }
    }
    if(failures==0)printf("TEST_RESULT=PASS ASSERTIONS=%d FAILURES=0\n",assertions);else printf("TEST_RESULT=FAIL ASSERTIONS=%d FAILURES=%d\n",assertions,failures);
    /* Keep generated evidence for examination; tests never clean an existing path. */
    printf("TEST_WORKSPACE=%s\n",root);
    return failures==0?0:1;
}
