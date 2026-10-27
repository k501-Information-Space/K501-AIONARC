/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Patrick R. Miller (Iinkognit0).
 */
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include "qh256.h"
#include "sha256.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define VERSION "3.0.0-alpha.1"
#define STORE_MAGIC "K501-AIONARC-AI-MEMORY-STORE-V3A1"
#define STATE_MAGIC "K501-AIONARC-MEMORY-STATE-V3A1"
#define TEXT_MAGIC "K501-MEMORY-TEXT-CAPTURE-V3A1"
#define INDEX_MAGIC "K501-MEMORY-INDEX-V3A1"
#define TRANS_MAGIC "K501-MEMORY-TRANSITION-V3A1"
#define GATEWAY_MAGIC "K501-MEMORY-GATEWAY-V3A1"
#define FRAME_MAGIC "K501-MEMORY-FRAME-V3A1"
#define PROOF_MAGIC "K501-PROOF-BEFORE-STATE-V3A1"
#define RECORD_MAGIC "K501-AIONARC-MEMORY-OBJECT-RECORD-V3A0"
#define OBJECT_ID_DOMAIN "K501-AIONARC-MEMORY-OBJECT-V3A0"
#define MAPPING_PROFILE "QH256-K501-AIONARC-MEMORY-V3A1"
#define MAX_OBJECTS 200000u
#define MAX_PATH_BYTES (1024u * 1024u)
#define MAX_TEXT_FILE (16u * 1024u * 1024u)
#define MAX_TEXT_TOTAL (256u * 1024u * 1024u)
#define IO_BUF_SIZE (64u * 1024u)

/* Linux/POSIX implementation. QH256 wire bytes: T plane [0..15], F plane [16..31]. */
typedef struct {
    char *path;                    /* raw relative filesystem path; root is "." */
    char type[16];                 /* file, directory, symlink, other */
    uint32_t mode;
    uint64_t size;
    int64_t mtime_sec;
    uint32_t mtime_nsec;
    char content_sha[65];          /* SHA-256 or "-" for objects without byte streams */
    char object_id[65];
    char record_hash[65];
    unsigned char *text;           /* derived searchable text, never canonical source */
    size_t text_len;
    char text_sha[65];
    char extractor[64];
    char text_status[48];
} Object;

typedef struct {
    Object *v;
    size_t n, cap;
} ObjectVec;

typedef struct {
    char frame_id[256];
    char parent_frame_id[256];
    char parent_state_sha[65];
    char root_id[256];
    uint64_t sequence;
    char state_sha[65];
    char chain_sha[65];
    uint64_t object_count;
    uint64_t added, removed, content_changed, metadata_changed, unchanged;
} FrameMeta;

typedef struct {
    char kind[32];
    char object_id[65];
    char old_record_hash[65];
    char new_record_hash[65];
    char *path;
} Event;

typedef struct {
    Event *v;
    size_t n, cap;
    uint64_t added, removed, content_changed, metadata_changed, unchanged;
} EventVec;

typedef struct {
    char object_id[65];
    char content_sha[65];
    char text_sha[65];
    char extractor[64];
    char *path;
    unsigned char *text;
    size_t text_len;
} TextEntry;

typedef struct {
    TextEntry *v;
    size_t n, cap;
} TextVec;

static void die(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap); fputc('\n', stderr); exit(2);
}
static void *xmalloc(size_t n) { void *p = malloc(n ? n : 1); if (!p) die("ERROR: out of memory"); return p; }
static void *xrealloc(void *p, size_t n) { void *q = realloc(p, n ? n : 1); if (!q) die("ERROR: out of memory"); return q; }
static char *xstrdup(const char *s) { char *p = strdup(s); if (!p) die("ERROR: out of memory"); return p; }
static bool has_suffix_ci(const char *s, const char *ext) {
    size_t a = strlen(s), b = strlen(ext); if (a < b) return false;
    for (size_t i = 0; i < b; i++) {
        unsigned char x=(unsigned char)s[a-b+i], y=(unsigned char)ext[i];
        if (x >= 'A' && x <= 'Z') x=(unsigned char)(x+32);
        if (y >= 'A' && y <= 'Z') y=(unsigned char)(y+32);
        if (x != y) return false;
    }
    return true;
}
static bool is_text_candidate(const char *p) {
    static const char *exts[]={".txt",".md",".markdown",".rst",".json",".jsonl",".yaml",".yml",".toml",".ini",".conf",".cfg",".csv",".tsv",".xml",".html",".htm",".c",".h",".cc",".cpp",".hpp",".py",".sh",".bash",".sql",".log",".tex",".bib",".diff",".patch",".make",".cmake",".gitignore",".sha256",".sum",".service",".desktop",".js",".ts",".css",".svg",".ndjson"};
    const char *base=strrchr(p,'/'); base=base?base+1:p;
    if (strcmp(base,"Makefile")==0 || strcmp(base,"Dockerfile")==0 || strcmp(base,"README")==0 || strcmp(base,"LICENSE")==0) return true;
    for (size_t i=0;i<sizeof(exts)/sizeof(exts[0]);i++) if (has_suffix_ci(base,exts[i])) return true;
    return false;
}
static bool is_config_candidate(const char *p) { return has_suffix_ci(p,".json")||has_suffix_ci(p,".yaml")||has_suffix_ci(p,".yml")||has_suffix_ci(p,".toml")||has_suffix_ci(p,".ini")||has_suffix_ci(p,".conf")||has_suffix_ci(p,".cfg"); }
static bool is_source_candidate(const char *p) { return has_suffix_ci(p,".c")||has_suffix_ci(p,".h")||has_suffix_ci(p,".cc")||has_suffix_ci(p,".cpp")||has_suffix_ci(p,".hpp")||has_suffix_ci(p,".py")||has_suffix_ci(p,".sh")||has_suffix_ci(p,".sql")||has_suffix_ci(p,".js")||has_suffix_ci(p,".ts"); }
static bool safe_id(const char *s) {
    if (!s || !*s || strlen(s)>200) return false;
    if (!( (s[0]>='A'&&s[0]<='Z')||(s[0]>='a'&&s[0]<='z')||(s[0]>='0'&&s[0]<='9'))) return false;
    for (const unsigned char *p=(const unsigned char*)s; *p; ++p)
        if (!( (*p>='A'&&*p<='Z')||(*p>='a'&&*p<='z')||(*p>='0'&&*p<='9')||*p=='-'||*p=='_'||*p=='.')) return false;
    return true;
}
static char *join_path(const char *a, const char *b) {
    size_t na=strlen(a), nb=strlen(b); bool slash=na && a[na-1]=='/';
    char *p=xmalloc(na+nb+(slash?1:2)); snprintf(p,na+nb+(slash?1:2),"%s%s%s",a,slash?"":"/",b); return p;
}
static bool path_is_within(const char *parent, const char *child) {
    size_t n=strlen(parent); if (strncmp(parent,child,n)!=0) return false;
    return child[n]=='\0' || (n==1 && parent[0]=='/') || child[n]=='/';
}
static void canonical_abs(const char *p, char out[PATH_MAX]) {
    char *r=realpath(p,NULL); if (!r) die("ERROR: cannot resolve path '%s': %s",p,strerror(errno));
    if (strlen(r)>=PATH_MAX) { free(r); die("ERROR: path exceeds PATH_MAX"); }
    strcpy(out,r); free(r);
}
static void bytes_to_hex(const uint8_t *bytes,size_t n,char *out) {
    static const char h[]="0123456789abcdef";
    for(size_t i=0;i<n;i++){out[i*2]=h[bytes[i]>>4];out[i*2+1]=h[bytes[i]&15u];}
    out[n*2]='\0';
}
static bool sha256_buf(const void *data,size_t n,char out[65]) {
    uint8_t md[32];sha256_bytes((const uint8_t*)data,n,md);bytes_to_hex(md,32,out);return true;
}
static bool is_hex64(const char *s);
static bool hex_to_bytes32(const char *hex,uint8_t out[32]) {
    if(!is_hex64(hex)) return false;
    for(size_t i=0;i<32;i++){unsigned x=0;if(sscanf(hex+2*i,"%2x",&x)!=1)return false;out[i]=(uint8_t)x;}
    return true;
}
static bool chain_hash_for(const char *parent_chain_sha,const char *state_sha,char out[65]) {
    static const uint8_t domain[]="K501-AIONARC-MEMORY-CHAIN-V3A1\0";
    uint8_t parent[32]={0},state[32],digest[32];
    if(parent_chain_sha && strcmp(parent_chain_sha,"-")!=0 && !hex_to_bytes32(parent_chain_sha,parent))return false;
    if(!hex_to_bytes32(state_sha,state))return false;
    sha256_ctx ctx;sha256_init(&ctx);sha256_update(&ctx,domain,sizeof(domain)-1);sha256_update(&ctx,parent,32);sha256_update(&ctx,state,32);sha256_final(&ctx,digest);bytes_to_hex(digest,32,out);return true;
}
static bool sha256_file(const char *path,char out[65]) {
    FILE *f=fopen(path,"rb");if(!f)return false;sha256_ctx ctx;sha256_init(&ctx);uint8_t buf[IO_BUF_SIZE];size_t n;
    while((n=fread(buf,1,sizeof(buf),f))>0)sha256_update(&ctx,buf,n);
    bool ok=!ferror(f);if(fclose(f)!=0)ok=false;if(!ok)return false;uint8_t md[32];sha256_final(&ctx,md);bytes_to_hex(md,32,out);return true;
}
static const char B64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static void base64_encode(const unsigned char *src,size_t n,char **out) {
    if(n>(SIZE_MAX-4)/4*3)die("ERROR: base64 input too large");
    size_t cap=4*((n+2)/3)+1;char *b=xmalloc(cap);size_t i=0,j=0;
    while(i<n){size_t rem=n-i;uint32_t a=src[i++],bb=rem>1?src[i++]:0,cc=rem>2?src[i++]:0;uint32_t x=(a<<16)|(bb<<8)|cc;b[j++]=B64[(x>>18)&63];b[j++]=B64[(x>>12)&63];b[j++]=rem>1?B64[(x>>6)&63]:'=';b[j++]=rem>2?B64[x&63]:'=';}
    b[j]='\0';*out=b;
}
static int b64_value(unsigned char c) {
    if(c>='A'&&c<='Z')return (int)(c-'A');if(c>='a'&&c<='z')return (int)(c-'a')+26;if(c>='0'&&c<='9')return (int)(c-'0')+52;if(c=='+')return 62;if(c=='/')return 63;return -1;
}
static unsigned char *base64_decode(const char *s,size_t *outn) {
    size_t n=strlen(s);if(n%4!=0)return NULL;size_t pad=(n&&s[n-1]=='=')?1:0;if(n>1&&s[n-2]=='=')pad++;
    if(pad>2)return NULL;for(size_t i=0;i<n-pad;i++)if(s[i]=='=')return NULL;
    unsigned char *out=xmalloc((n/4)*3+1);size_t j=0;
    for(size_t i=0;i<n;i+=4){int a=b64_value((unsigned char)s[i]),b=b64_value((unsigned char)s[i+1]);int c=s[i+2]=='='?0:b64_value((unsigned char)s[i+2]);int d=s[i+3]=='='?0:b64_value((unsigned char)s[i+3]);if(a<0||b<0||c<0||d<0){free(out);return NULL;}uint32_t x=((uint32_t)a<<18)|((uint32_t)b<<12)|((uint32_t)c<<6)|(uint32_t)d;out[j++]=(unsigned char)(x>>16);if(s[i+2]!='=')out[j++]=(unsigned char)(x>>8);if(s[i+3]!='=')out[j++]=(unsigned char)x;}
    if((pad==1&&(b64_value((unsigned char)s[n-2])<0||(b64_value((unsigned char)s[n-2])&3)!=0))||(pad==2&&(b64_value((unsigned char)s[n-3])<0||(b64_value((unsigned char)s[n-3])&15)!=0))){free(out);return NULL;}
    *outn=j;return out;
}
static bool validate_path_base64_roundtrip(const ObjectVec *v) {
    for(size_t i=0;i<v->n;i++){
        const char *path=v->v[i].path;
        char *encoded=NULL;size_t decoded_len=0;
        base64_encode((const unsigned char*)path,strlen(path),&encoded);
        unsigned char *decoded=base64_decode(encoded,&decoded_len);
        bool ok=decoded&&decoded_len==strlen(path)&&memcmp(decoded,path,decoded_len)==0;
        free(encoded);free(decoded);
        if(!ok)return false;
    }
    return true;
}
static void print_escaped_path(const char *s) {
    for(const unsigned char *p=(const unsigned char*)s;*p;p++){
        if(*p=='\\')fputs("\\\\",stdout);else if(*p=='\t')fputs("\\t",stdout);else if(*p=='\n')fputs("\\n",stdout);else if(*p=='\r')fputs("\\r",stdout);else if(*p<32||*p==127)printf("\\x%02x",*p);else putchar(*p);
    }
}
static void hash_domain4(const char *domain,const char *root,const char *path,const char *type,char out[65]) {
    sha256_ctx ctx;sha256_init(&ctx);const uint8_t z=0;uint8_t md[32];
    sha256_update(&ctx,(const uint8_t*)domain,strlen(domain));sha256_update(&ctx,&z,1);sha256_update(&ctx,(const uint8_t*)root,strlen(root));sha256_update(&ctx,&z,1);sha256_update(&ctx,(const uint8_t*)path,strlen(path));sha256_update(&ctx,&z,1);sha256_update(&ctx,(const uint8_t*)type,strlen(type));sha256_final(&ctx,md);bytes_to_hex(md,32,out);
}
static bool is_hex64(const char *s) { if(!s||strlen(s)!=64) return false; for(int i=0;i<64;i++) if(!((s[i]>='0'&&s[i]<='9')||(s[i]>='a'&&s[i]<='f'))) return false; return true; }
static void record_hash_for(const Object *o, const char *root_id, char out[65]) {
    char *path64=NULL; base64_encode((const unsigned char*)o->path,strlen(o->path),&path64);
    char payload[8192];
    int n=snprintf(payload,sizeof(payload),"%s\nroot_id\t%s\nrelative_path_b64\t%s\nobject_type\t%s\nmode\t%04o\nsize\t%" PRIu64 "\nmtime_sec\t%" PRId64 "\nmtime_nsec\t%u\ncontent_sha256\t%s\n",
        RECORD_MAGIC,root_id,path64,o->type,o->mode,o->size,o->mtime_sec,o->mtime_nsec,o->content_sha);
    free(path64); if(n<0||(size_t)n>=sizeof(payload)) die("ERROR: canonical record overflow");
    if(!sha256_buf(payload,(size_t)n,out)) die("ERROR: cannot hash object record");
}
static int cmp_obj_path(const void *a,const void *b) { const Object *x=a,*y=b; return strcmp(x->path,y->path); }
static int cmp_strptr(const void *a,const void *b) { const char *const *x=a, *const *y=b; return strcmp(*x,*y); }
static void vec_push(ObjectVec *v, Object *o) {
    if(v->n>=MAX_OBJECTS) die("ERROR: object safety limit (%u) exceeded",MAX_OBJECTS);
    if(v->n==v->cap){v->cap=v->cap?v->cap*2:128;v->v=xrealloc(v->v,v->cap*sizeof(*v->v));}
    v->v[v->n++]=*o; memset(o,0,sizeof(*o));
}
static void event_push(EventVec *v, const char *kind, const Object *old, const Object *now) {
    if(v->n==v->cap){v->cap=v->cap?v->cap*2:128;v->v=xrealloc(v->v,v->cap*sizeof(*v->v));}
    Event *e=&v->v[v->n++]; memset(e,0,sizeof(*e)); snprintf(e->kind,sizeof(e->kind),"%s",kind);
    const Object *base=now?now:old; snprintf(e->object_id,sizeof(e->object_id),"%s",base->object_id);
    snprintf(e->old_record_hash,sizeof(e->old_record_hash),"%s",old?old->record_hash:"-");
    snprintf(e->new_record_hash,sizeof(e->new_record_hash),"%s",now?now->record_hash:"-"); e->path=xstrdup(base->path);
    if(strcmp(kind,"ADDED")==0)v->added++; else if(strcmp(kind,"REMOVED")==0)v->removed++;
    else if(strcmp(kind,"CONTENT_CHANGED")==0)v->content_changed++; else if(strcmp(kind,"METADATA_CHANGED")==0)v->metadata_changed++; else if(strcmp(kind,"UNCHANGED")==0)v->unchanged++;
}
static int cmp_event_path(const void *a,const void *b) { const Event *x=a,*y=b; int c=strcmp(x->path,y->path); if(c) return c; return strcmp(x->kind,y->kind); }
static void vec_free(ObjectVec *v) { for(size_t i=0;i<v->n;i++){free(v->v[i].path);free(v->v[i].text);} free(v->v); memset(v,0,sizeof(*v)); }
static void events_free(EventVec *v) { for(size_t i=0;i<v->n;i++)free(v->v[i].path);free(v->v);memset(v,0,sizeof(*v)); }
static void texts_free(TextVec *v) { for(size_t i=0;i<v->n;i++){free(v->v[i].path);free(v->v[i].text);}free(v->v);memset(v,0,sizeof(*v)); }
static bool utf8_valid(const unsigned char *s,size_t n) {
    size_t i=0; while(i<n){ unsigned char c=s[i++]; if(c<0x80) continue;
        unsigned need; uint32_t cp;
        if((c&0xE0)==0xC0){need=1;cp=c&0x1F;if(cp<2)return false;}
        else if((c&0xF0)==0xE0){need=2;cp=c&0x0F;}
        else if((c&0xF8)==0xF0){need=3;cp=c&0x07;if(cp>4)return false;}
        else return false;
        if(i+need>n)return false;
        for(unsigned j=0;j<need;j++){unsigned char d=s[i++];if((d&0xC0)!=0x80)return false;cp=(cp<<6)|(d&0x3F);}
        if((need==2&&cp<0x800)||(need==3&&cp<0x10000)||cp>0x10FFFF||(cp>=0xD800&&cp<=0xDFFF))return false;
    } return true;
}
static unsigned char *read_file_limited(const char *path,size_t *n,bool *too_large) {
    FILE *f=fopen(path,"rb"); if(!f)return NULL; size_t cap=65536,used=0; unsigned char *b=xmalloc(cap); *too_large=false;
    for(;;){
        if(used==cap){
            if(cap>=MAX_TEXT_FILE){
                int extra=fgetc(f);
                if(extra!=EOF)*too_large=true;
                else if(ferror(f)){free(b);fclose(f);return NULL;}
                break;
            }
            size_t nc=cap*2; if(nc>MAX_TEXT_FILE)nc=MAX_TEXT_FILE; b=xrealloc(b,nc);cap=nc;
        }
        size_t k=fread(b+used,1,cap-used,f); used+=k;
        if(k==0){if(ferror(f)){free(b);fclose(f);return NULL;}break;}
    }
    if(*too_large){unsigned char drain[IO_BUF_SIZE];while(fread(drain,1,sizeof(drain),f)>0){} }
    fclose(f); if(*too_large){free(b);*n=0;return NULL;} *n=used; return b;
}
static unsigned char *pdf_to_text(const char *path,size_t *n,bool *too_large,
                                  bool *extractor_succeeded,bool *read_failed) {
    int fd[2];
    pid_t pid;
    size_t cap=65536,used=0;
    unsigned char *b;
    int status=0;
    pid_t waited;
    *n=0;
    *too_large=false;
    *extractor_succeeded=false;
    *read_failed=false;
    if(pipe(fd)!=0)return NULL;
    pid=fork();
    if(pid<0){close(fd[0]);close(fd[1]);return NULL;}
    if(pid==0){
        close(fd[0]);
        if(dup2(fd[1],STDOUT_FILENO)<0)_exit(126);
        close(fd[1]);
        int devnull=open("/dev/null",O_WRONLY);
        if(devnull>=0){dup2(devnull,STDERR_FILENO);close(devnull);}
        execlp("pdftotext","pdftotext","-enc","UTF-8","-layout",path,"-",(char*)NULL);
        _exit(127);
    }
    close(fd[1]);
    b=xmalloc(cap);
    for(;;){
        unsigned char tmp[IO_BUF_SIZE];
        ssize_t k=read(fd[0],tmp,sizeof(tmp));
        if(k<0){
            if(errno==EINTR)continue;
            free(b);b=NULL;*read_failed=true;break;
        }
        if(k==0)break;
        if(used+(size_t)k>MAX_TEXT_FILE){*too_large=true;continue;}
        if(used+(size_t)k>cap){
            size_t nc=cap;
            while(nc<used+(size_t)k)nc*=2;
            if(nc>MAX_TEXT_FILE)nc=MAX_TEXT_FILE;
            b=xrealloc(b,nc);cap=nc;
        }
        memcpy(b+used,tmp,(size_t)k);used+=(size_t)k;
    }
    close(fd[0]);
    do { waited=waitpid(pid,&status,0); } while(waited<0&&errno==EINTR);
    if(waited==pid&&WIFEXITED(status)&&WEXITSTATUS(status)==0)
        *extractor_succeeded=true;
    if(!b||!*extractor_succeeded||*too_large){free(b);return NULL;}
    *n=used;
    return b;
}
static bool same_fs_metadata(const struct stat *a, const struct stat *b) {
    return a->st_mode == b->st_mode && a->st_size == b->st_size &&
           a->st_mtim.tv_sec == b->st_mtim.tv_sec &&
           a->st_mtim.tv_nsec == b->st_mtim.tv_nsec;
}
static void walk_one(const char *root_abs,const char *rel,ObjectVec *v) {
    char *abs = strcmp(rel,".")==0 ? xstrdup(root_abs) : join_path(root_abs,rel);
    struct stat st;
    if (lstat(abs,&st)!=0) die("ERROR: cannot stat source object '%s': %s",rel,strerror(errno));
    Object o; memset(&o,0,sizeof(o));
    o.path=xstrdup(rel); o.mode=(uint32_t)(st.st_mode&07777); o.size=(uint64_t)st.st_size;
    o.mtime_sec=(int64_t)st.st_mtim.tv_sec; o.mtime_nsec=(uint32_t)st.st_mtim.tv_nsec;
    strcpy(o.content_sha,"-"); strcpy(o.text_sha,"-"); strcpy(o.extractor,"-"); strcpy(o.text_status,"NOT_CANDIDATE");
    if (S_ISREG(st.st_mode)) {
        strcpy(o.type,"file"); char first[65], second[65]; struct stat after;
        if (!sha256_file(abs,first)) { free(abs);free(o.path);die("ERROR: cannot hash file '%s': %s",rel,strerror(errno)); }
        if (lstat(abs,&after)!=0 || !same_fs_metadata(&st,&after)) { free(abs);free(o.path);die("NO_DRIFT FAIL: file metadata changed during observation: %s",rel); }
        if (!sha256_file(abs,second) || strcmp(first,second)!=0) { free(abs);free(o.path);die("NO_DRIFT FAIL: file bytes changed during observation: %s",rel); }
        strcpy(o.content_sha,first);
    } else if (S_ISDIR(st.st_mode)) {
        strcpy(o.type,"directory");
    } else if (S_ISLNK(st.st_mode)) {
        strcpy(o.type,"symlink"); size_t n=(size_t)st.st_size+2; if(n<16)n=16;
        char *target=xmalloc(n), *target2=xmalloc(n); struct stat after;
        if(!target||!target2){free(target);free(target2);free(abs);free(o.path);die("ERROR: cannot allocate symlink buffer");}
        ssize_t k=readlink(abs,target,n-1);ssize_t k2=readlink(abs,target2,n-1);
        if(k<0||k2<0||k!=k2||memcmp(target,target2,(size_t)(k<0?0:k))!=0||lstat(abs,&after)!=0||!same_fs_metadata(&st,&after)) {
            free(target);free(target2);free(abs);free(o.path);die("NO_DRIFT FAIL: symlink changed during observation: %s",rel);
        }
        target[k]='\0'; if(!sha256_buf(target,(size_t)k,o.content_sha)){free(target);free(target2);free(abs);free(o.path);die("ERROR: cannot hash symlink target");}
        o.size=(uint64_t)k; free(target);free(target2);
    } else {
        strcpy(o.type,"other");
    }
    vec_push(v,&o);
    if (strcmp(v->v[v->n-1].type,"directory")==0) {
        DIR *d=opendir(abs);
        if(!d){free(abs);die("ERROR: cannot open directory '%s': %s",rel,strerror(errno));}
        struct dirent *de;
        while((de=readdir(d))!=NULL){
            if(strcmp(de->d_name,".")==0||strcmp(de->d_name,"..")==0)continue;
            if(strchr(de->d_name,'/')){closedir(d);free(abs);die("ERROR: invalid directory entry name");}
            char *child=strcmp(rel,".")==0?xstrdup(de->d_name):join_path(rel,de->d_name);
            if(strlen(child)>MAX_PATH_BYTES){closedir(d);free(child);free(abs);die("ERROR: source path exceeds safety limit");}
            walk_one(root_abs,child,v);free(child);
        }
        closedir(d);
        struct stat after;
        if(lstat(abs,&after)!=0||!same_fs_metadata(&st,&after)){free(abs);die("NO_DRIFT FAIL: directory entry set or metadata changed during observation: %s",rel);}
    }
    free(abs);
}
static void scan_tree(const char *source,const char *store,const char *root_id,ObjectVec *v) {
    char src[PATH_MAX], sto[PATH_MAX];canonical_abs(source,src);canonical_abs(store,sto);
    if(path_is_within(src,sto)||path_is_within(sto,src))die("ERROR: source and store must be separate, non-overlapping directory trees");
    struct stat st;if(stat(src,&st)!=0||!S_ISDIR(st.st_mode))die("ERROR: source must be an existing directory");
    (void)root_id;walk_one(src,".",v);qsort(v->v,v->n,sizeof(*v->v),cmp_obj_path);
    for(size_t i=0;i<v->n;i++){
        if(i && strcmp(v->v[i-1].path,v->v[i].path)==0)die("ERROR: duplicate relative path detected");
        hash_domain4(OBJECT_ID_DOMAIN,root_id,v->v[i].path,v->v[i].type,v->v[i].object_id);
        record_hash_for(&v->v[i],root_id,v->v[i].record_hash);
    }
}
static void extract_texts(ObjectVec *v,const char *source,size_t *total) {
    *total=0;
    for(size_t i=0;i<v->n;i++){
        Object *o=&v->v[i];
        if(strcmp(o->type,"file")!=0)continue;
        bool pdf=has_suffix_ci(o->path,".pdf");
        bool cand=is_text_candidate(o->path)||pdf;
        if(!cand){strcpy(o->text_status,"NOT_CANDIDATE");continue;}
        if(*total>=MAX_TEXT_TOTAL){strcpy(o->text_status,"SKIPPED_TOTAL_LIMIT");continue;}

        char *abs=join_path(source,strcmp(o->path,".")==0?"":o->path);
        size_t n=0;
        bool too=false,pdf_succeeded=false,read_failed=false;
        unsigned char *data=NULL;
        const char *extractor="";
        if(pdf){
            data=pdf_to_text(abs,&n,&too,&pdf_succeeded,&read_failed);
            extractor="pdftotext-layout-utf8-v1";
        } else {
            data=read_file_limited(abs,&n,&too);
            extractor="plain-utf8-bytes-v1";
        }
        free(abs);

        if(pdf&&pdf_succeeded&&too){strcpy(o->text_status,"PDF_OUTPUT_FILE_LIMIT");free(data);continue;}
        if(pdf&&pdf_succeeded&&read_failed){strcpy(o->text_status,"PDF_CAPTURE_READ_FAILED");free(data);continue;}
        if(too){strcpy(o->text_status,"SKIPPED_FILE_LIMIT");free(data);continue;}
        if(!data){strcpy(o->text_status,pdf?"PDF_EXTRACTOR_UNAVAILABLE_OR_FAILED":"READ_FAILED");continue;}
        if(n>MAX_TEXT_TOTAL-*total){
            free(data);
            strcpy(o->text_status,pdf?"PDF_CAPTURE_SKIPPED_TOTAL_LIMIT":"SKIPPED_TOTAL_LIMIT");
            continue;
        }
        /* INVALID_UTF8 is reserved for a failed UTF-8 validation. NUL is valid UTF-8
           code point U+0000 but cannot be retained by this C-string-oriented index. */
        if(!utf8_valid(data,n)){free(data);strcpy(o->text_status,"INVALID_UTF8");continue;}
        if(memchr(data,0,n)!=NULL){free(data);strcpy(o->text_status,"EMBEDDED_NUL");continue;}

        char now_sha[65];
        char *verify_path=strcmp(o->path,".")==0?xstrdup(source):join_path(source,o->path);
        bool hash_ok=sha256_file(verify_path,now_sha);
        free(verify_path);
        if(!hash_ok){free(data);die("ERROR: cannot re-verify source bytes for '%s'",o->path);}
        if(strcmp(now_sha,o->content_sha)!=0){free(data);die("NO_DRIFT FAIL: source changed during scan: %s",o->path);}
        o->text=data;
        o->text_len=n;
        strcpy(o->extractor,extractor);
        if(!sha256_buf(data,n,o->text_sha))die("ERROR: cannot hash extracted text");
        *total+=n;
        strcpy(o->text_status,n?"CAPTURED":"CAPTURED_EMPTY");
    }
}
static void write_printf_file(const char *path,const char *fmt,...) {
    int fd=open(path,O_WRONLY|O_CREAT|O_EXCL,0644);if(fd<0)die("ERROR: cannot create '%s': %s",path,strerror(errno));FILE *f=fdopen(fd,"w");if(!f){close(fd);die("ERROR: fdopen failed");}
    va_list ap;va_start(ap,fmt);int r=vfprintf(f,fmt,ap);va_end(ap);if(r<0||fflush(f)!=0||fsync(fd)!=0){fclose(f);die("ERROR: write/fsync failed for '%s'",path);}if(fclose(f)!=0)die("ERROR: close failed for '%s'",path);
}
static char *frame_dir(const char *store,const char *frame) { char *a=join_path(store,"frames");char *b=join_path(a,frame);free(a);return b; }
static char *frame_file(const char *store,const char *frame,const char *file) { char *a=frame_dir(store,frame);char *b=join_path(a,file);free(a);return b; }
static bool copy_field(char *out,size_t cap,const char *src) { size_t n=strlen(src); if(n>=cap)return false; memcpy(out,src,n+1); return true; }
static bool read_key_file(const char *path,const char *key,char *out,size_t cap) {
    FILE *f=fopen(path,"rb");if(!f)return false;char *line=NULL;size_t n=0;ssize_t k;bool found=false;
    while((k=getline(&line,&n,f))!=-1){if(k&&line[k-1]=='\n')line[--k]='\0';char *t=strchr(line,'\t');if(!t)continue;*t++='\0';if(strcmp(line,key)==0){if(strlen(t)>=cap){free(line);fclose(f);return false;}strcpy(out,t);found=true;break;}}
    free(line);fclose(f);return found;
}
static bool parse_state(const char *path,const char *expected_frame,ObjectVec *v,FrameMeta *m) {
    FILE *f=fopen(path,"rb");if(!f)return false;memset(m,0,sizeof(*m));char *line=NULL;size_t cap=0;ssize_t n;bool magic=false,objects=false;
    while((n=getline(&line,&cap,f))!=-1){if(n && line[n-1]=='\n')line[--n]='\0';if(n && line[n-1]=='\r')line[--n]='\0';if(!magic){if(strcmp(line,STATE_MAGIC)!=0)goto bad;magic=true;continue;}if(strcmp(line,"objects")==0){objects=true;continue;}
        if(!objects){char *t=strchr(line,'\t');if(!t)goto bad;*t++='\0';
            if(strcmp(line,"root_id")==0)snprintf(m->root_id,sizeof(m->root_id),"%s",t);
            else if(strcmp(line,"frame_id")==0)snprintf(m->frame_id,sizeof(m->frame_id),"%s",t);
            else if(strcmp(line,"parent_frame_id")==0)snprintf(m->parent_frame_id,sizeof(m->parent_frame_id),"%s",t);
            else if(strcmp(line,"parent_state_sha256")==0)snprintf(m->parent_state_sha,sizeof(m->parent_state_sha),"%s",t);
            else if(strcmp(line,"sequence")==0)m->sequence=strtoull(t,NULL,10);
            else if(strcmp(line,"object_count")==0)m->object_count=strtoull(t,NULL,10);
            else goto bad;
        }else{
            char *fields[12];size_t nf=0;char *save=NULL;for(char *p=strtok_r(line,"\t",&save);p&&nf<12;p=strtok_r(NULL,"\t",&save))fields[nf++]=p;
            if(nf!=10||strcmp(fields[0],"O")!=0||v->n>=MAX_OBJECTS)goto bad;
            Object o;memset(&o,0,sizeof(o));
            if(!is_hex64(fields[1])||!is_hex64(fields[2])||!(strcmp(fields[3],"-")==0||is_hex64(fields[3])))goto bad;
            strcpy(o.object_id,fields[1]);strcpy(o.record_hash,fields[2]);strcpy(o.content_sha,fields[3]);snprintf(o.type,sizeof(o.type),"%s",fields[4]);o.mode=(uint32_t)strtoul(fields[5],NULL,8);o.size=strtoull(fields[6],NULL,10);o.mtime_sec=(int64_t)strtoll(fields[7],NULL,10);o.mtime_nsec=(uint32_t)strtoul(fields[8],NULL,10);
            size_t pn=0;unsigned char *p=base64_decode(fields[9],&pn);if(!p||memchr(p,0,pn)!=NULL){free(p);goto bad;}o.path=xmalloc(pn+1);memcpy(o.path,p,pn);o.path[pn]='\0';free(p);
            if(strcmp(o.type,"file")&&strcmp(o.type,"directory")&&strcmp(o.type,"symlink")&&strcmp(o.type,"other")){free(o.path);goto bad;}
            if(v->n==v->cap){v->cap=v->cap?v->cap*2:128;v->v=xrealloc(v->v,v->cap*sizeof(*v->v));}v->v[v->n++]=o;
        }
    }
    free(line);fclose(f);if(!magic||!objects||!m->frame_id[0]||!m->root_id[0]||m->object_count!=v->n) { return false; }
    if(expected_frame&&strcmp(expected_frame,m->frame_id)!=0)return false;
    for(size_t i=1;i<v->n;i++)if(strcmp(v->v[i-1].path,v->v[i].path)>=0)return false;
    return true;
bad: free(line);fclose(f);return false;
}
static void write_state_candidate(const char *path,const FrameMeta *m,const ObjectVec *v) {
    int fd=open(path,O_WRONLY|O_CREAT|O_EXCL,0644);if(fd<0)die("ERROR: cannot create candidate state: %s",strerror(errno));FILE *f=fdopen(fd,"w");if(!f){close(fd);die("ERROR: fdopen failed");}
    fprintf(f,"%s\nroot_id\t%s\nframe_id\t%s\nsequence\t%" PRIu64 "\nparent_frame_id\t%s\nparent_state_sha256\t%s\nobject_count\t%zu\nobjects\n",STATE_MAGIC,m->root_id,m->frame_id,m->sequence,m->parent_frame_id[0]?m->parent_frame_id:"-",m->parent_state_sha[0]?m->parent_state_sha:"-",v->n);
    for(size_t i=0;i<v->n;i++){const Object *o=&v->v[i];char *p64=NULL;base64_encode((const unsigned char*)o->path,strlen(o->path),&p64);fprintf(f,"O\t%s\t%s\t%s\t%s\t%04o\t%" PRIu64 "\t%" PRId64 "\t%u\t%s\n",o->object_id,o->record_hash,o->content_sha,o->type,o->mode,o->size,o->mtime_sec,o->mtime_nsec,p64);free(p64);}
    if(fflush(f)!=0||fsync(fd)!=0){fclose(f);die("ERROR: candidate state flush failed");}if(fclose(f)!=0)die("ERROR: candidate state close failed");
}
static int find_id(const ObjectVec *v,const char *id) { for(size_t i=0;i<v->n;i++)if(strcmp(v->v[i].object_id,id)==0)return (int)i;return -1; }
static void derive_transition(const ObjectVec *old,const ObjectVec *now,EventVec *ev) {
    if(!old){for(size_t i=0;i<now->n;i++)event_push(ev,"ADDED",NULL,&now->v[i]);qsort(ev->v,ev->n,sizeof(*ev->v),cmp_event_path);return;}
    for(size_t i=0;i<now->n;i++){const Object *n=&now->v[i];int j=find_id(old,n->object_id);if(j<0)event_push(ev,"ADDED",NULL,n);else {const Object *o=&old->v[j];if(strcmp(o->content_sha,n->content_sha)!=0)event_push(ev,"CONTENT_CHANGED",o,n);else if(strcmp(o->record_hash,n->record_hash)!=0)event_push(ev,"METADATA_CHANGED",o,n);else event_push(ev,"UNCHANGED",o,n);}}
    for(size_t i=0;i<old->n;i++)if(find_id(now,old->v[i].object_id)<0)event_push(ev,"REMOVED",&old->v[i],NULL);
    qsort(ev->v,ev->n,sizeof(*ev->v),cmp_event_path);
}
static bool validate_transition_semantics(const char *path,const char *parent_id,const ObjectVec *old,const ObjectVec *now) {
    EventVec expected={0};derive_transition(old,now,&expected);char b[256];bool ok=true;
    if(!read_key_file(path,"parent_frame_id",b,sizeof(b))||strcmp(b,parent_id&&*parent_id?parent_id:"-"))ok=false;
    if(!read_key_file(path,"event_count",b,sizeof(b))||strtoull(b,NULL,10)!=expected.n)ok=false;
    if(!read_key_file(path,"added",b,sizeof(b))||strtoull(b,NULL,10)!=expected.added)ok=false;
    if(!read_key_file(path,"removed",b,sizeof(b))||strtoull(b,NULL,10)!=expected.removed)ok=false;
    if(!read_key_file(path,"content_changed",b,sizeof(b))||strtoull(b,NULL,10)!=expected.content_changed)ok=false;
    if(!read_key_file(path,"metadata_changed",b,sizeof(b))||strtoull(b,NULL,10)!=expected.metadata_changed)ok=false;
    if(!read_key_file(path,"unchanged",b,sizeof(b))||strtoull(b,NULL,10)!=expected.unchanged)ok=false;
    FILE *f=fopen(path,"rb");if(!f){events_free(&expected);return false;}char *line=NULL;size_t cap=0;ssize_t n;bool in_events=false;size_t ix=0;int first=0;
    while(ok&&(n=getline(&line,&cap,f))!=-1){if(n&&line[n-1]=='\n')line[--n]='\0';if(n&&line[n-1]=='\r')line[--n]='\0';if(first++==0){if(strcmp(line,TRANS_MAGIC))ok=false;continue;}if(!in_events){if(strcmp(line,"events")==0)in_events=true;continue;}
        if(ix>=expected.n){ok=false;break;}char *fields[8];size_t nf=0;char *save=NULL;for(char *p=strtok_r(line,"\t",&save);p&&nf<8;p=strtok_r(NULL,"\t",&save))fields[nf++]=p;
        Event *e=&expected.v[ix];char *p64=NULL;base64_encode((const unsigned char*)e->path,strlen(e->path),&p64);
        if(nf!=6||strcmp(fields[0],"E")||strcmp(fields[1],e->kind)||strcmp(fields[2],e->object_id)||strcmp(fields[3],e->old_record_hash)||strcmp(fields[4],e->new_record_hash)||strcmp(fields[5],p64))ok=false;
        free(p64);if(!ok)break;ix++;
    }
    if(ferror(f)||!in_events||ix!=expected.n)ok=false;free(line);fclose(f);events_free(&expected);return ok;
}
static void write_transition(const char *path,const EventVec *ev,const char *parent) {
    int fd=open(path,O_WRONLY|O_CREAT|O_EXCL,0644);if(fd<0)die("ERROR: cannot create transition: %s",strerror(errno));FILE *f=fdopen(fd,"w");if(!f){close(fd);die("ERROR: fdopen failed");}
    fprintf(f,"%s\nparent_frame_id\t%s\nevent_count\t%zu\nadded\t%" PRIu64 "\nremoved\t%" PRIu64 "\ncontent_changed\t%" PRIu64 "\nmetadata_changed\t%" PRIu64 "\nunchanged\t%" PRIu64 "\nevents\n",TRANS_MAGIC,parent&&*parent?parent:"-",ev->n,ev->added,ev->removed,ev->content_changed,ev->metadata_changed,ev->unchanged);
    for(size_t i=0;i<ev->n;i++){char *p64=NULL;base64_encode((const unsigned char*)ev->v[i].path,strlen(ev->v[i].path),&p64);fprintf(f,"E\t%s\t%s\t%s\t%s\t%s\n",ev->v[i].kind,ev->v[i].object_id,ev->v[i].old_record_hash,ev->v[i].new_record_hash,p64);free(p64);}
    if(fflush(f)!=0||fsync(fd)!=0){fclose(f);die("ERROR: transition flush failed");}fclose(f);
}
static bool is_text_nonempty(const Object *o){return o->text && o->text_len>0;}
static bool has_non_ascii(const char *s){for(const unsigned char*p=(const unsigned char*)s;*p;p++)if(*p>=128)return true;return false;}
static bool is_hidden_path(const char *s){const char *p=s;while(*p){if((*p=='.')&&(p==s||p[-1]=='/'))return true;p++;}return false;}
static bool is_zero_file(const Object *o){return strcmp(o->type,"file")==0&&o->size==0;}
static bool qh_text_candidate(const Object *o){return strcmp(o->type,"file")==0&&(is_text_candidate(o->path)||has_suffix_ci(o->path,".pdf"));}
static bool is_regular(const Object *o){return strcmp(o->type,"file")==0;}
static bool status_is(const char *status,const char *expected){return strcmp(status,expected)==0;}
static bool pdf_extractor_succeeded_status(const char *status){
    return status_is(status,"CAPTURED")||status_is(status,"CAPTURED_EMPTY")||
           status_is(status,"INVALID_UTF8")||status_is(status,"EMBEDDED_NUL")||
           status_is(status,"PDF_OUTPUT_FILE_LIMIT")||
           status_is(status,"PDF_CAPTURE_SKIPPED_TOTAL_LIMIT")||
           status_is(status,"PDF_CAPTURE_READ_FAILED");
}
static bool text_status_valid_for_object(const Object *o,const char *status){
    bool candidate=qh_text_candidate(o);
    bool pdf=is_regular(o)&&has_suffix_ci(o->path,".pdf");
    if(!candidate)return !o->text&&status_is(status,"NOT_CANDIDATE");
    if(o->text){
        if(o->text_len==0)return status_is(status,"CAPTURED_EMPTY");
        return status_is(status,"CAPTURED");
    }
    if(status_is(status,"NOT_CANDIDATE")||status_is(status,"CAPTURED")||status_is(status,"CAPTURED_EMPTY"))return false;
    if(status_is(status,"SKIPPED_TOTAL_LIMIT")||status_is(status,"INVALID_UTF8")||status_is(status,"EMBEDDED_NUL"))return true;
    if(pdf){
        return status_is(status,"PDF_EXTRACTOR_UNAVAILABLE_OR_FAILED")||
               status_is(status,"PDF_OUTPUT_FILE_LIMIT")||
               status_is(status,"PDF_CAPTURE_SKIPPED_TOTAL_LIMIT")||
               status_is(status,"PDF_CAPTURE_READ_FAILED");
    }
    return status_is(status,"SKIPPED_FILE_LIMIT")||status_is(status,"READ_FAILED");
}
static bool has_duplicate_content(const ObjectVec *v){char **hs=xmalloc((v->n?v->n:1)*sizeof(char*));size_t n=0;for(size_t i=0;i<v->n;i++)if(is_hex64(v->v[i].content_sha))hs[n++]=v->v[i].content_sha;qsort(hs,n,sizeof(char*),cmp_strptr);bool dup=false;for(size_t i=1;i<n;i++)if(strcmp(hs[i-1],hs[i])==0){dup=true;break;}free(hs);return dup;}
static void qh_set(qh256_t *q,unsigned cell,qh_state_t state){if(!qh256_set_cell(q,cell,state))die("ERROR: QH256 cell index/state invalid");}
static qh_state_t qh_bool(bool v){return v?QH_TRUE:QH_FALSE;}
static bool qh_tests(void) {
    const qh_state_t states[4]={QH_UNKNOWN,QH_FALSE,QH_TRUE,QH_GUARD};
    for(size_t i=0;i<QH256_CELLS;i++)for(size_t j=0;j<4;j++){
        qh256_t q=qh256_unknown();qh_state_t got=QH_GUARD;
        if(!qh256_set_cell(&q,i,states[j])||!qh256_get_cell(&q,i,&got)||got!=states[j])return false;
        uint8_t wire[QH256_BYTES];qh256_t round;
        if(!qh256_serialize(&q,wire)||!qh256_deserialize(&round,wire)||!qh256_equal(&q,&round))return false;
    }
    for(size_t i=0;i<QH256_CELLS;i++){
        qh256_t f=qh256_unknown(),t=qh256_unknown(),g;
        if(!qh256_set_cell(&f,i,QH_FALSE)||!qh256_set_cell(&t,i,QH_TRUE))return false;
        g=qh256_merge(f,t);qh_state_t got=QH_UNKNOWN;
        if(!qh256_get_cell(&g,i,&got)||got!=QH_GUARD)return false;
        qh256_t ft=qh256_merge(f,t),tf=qh256_merge(t,f),ff=qh256_merge(f,f);
        if(!qh256_equal(&ft,&tf))return false;
        if(!qh256_equal(&ff,&f))return false;
    }
    qh256_t q=qh256_unknown();qh_state_t got;
    if(qh256_get_cell(&q,QH256_CELLS,&got))return false;
    return true;
}
static qh256_t map_object_qh(const Object *o) {
    qh256_t q=qh256_unknown();
    bool file=strcmp(o->type,"file")==0;
    bool dir=strcmp(o->type,"directory")==0;
    bool link=strcmp(o->type,"symlink")==0;
    bool other=strcmp(o->type,"other")==0;
    bool pdf=file&&has_suffix_ci(o->path,".pdf");

    /* Cells 0..15 preserve the legacy coded values documented by the profile. */
    qh_set(&q,0,QH_TRUE);
    if(file){qh_set(&q,1,QH_TRUE);qh_set(&q,2,QH_FALSE);}
    else if(dir){qh_set(&q,1,QH_FALSE);qh_set(&q,2,QH_TRUE);}
    qh_set(&q,3,qh_bool(!other));
    qh_set(&q,4,QH_TRUE);
    if(file)qh_set(&q,5,QH_TRUE);
    qh_set(&q,6,QH_UNKNOWN);
    qh_set(&q,7,QH_TRUE);
    qh_set(&q,8,qh_bool(o->size==0));
    qh_set(&q,9,qh_bool(o->size!=0));
    if(file)qh_set(&q,10,QH_TRUE);
    qh_set(&q,11,QH_TRUE);
    qh_set(&q,12,QH_TRUE);
    qh_set(&q,13,QH_UNKNOWN);
    qh_set(&q,14,qh_bool(file||dir));
    qh_set(&q,15,QH_TRUE);

    /* Cells 16..31 are application-level observations; QH256 core is unchanged. */
    qh_set(&q,16,qh_bool(link));
    qh_set(&q,17,qh_bool(other));
    qh_set(&q,18,QH_TRUE);
    if(is_hex64(o->content_sha))qh_set(&q,19,QH_TRUE);
    if(file||link)qh_set(&q,20,QH_TRUE);

    /* A candidate with no capture result is FALSE; non-candidates remain UNKNOWN. */
    if(qh_text_candidate(o))qh_set(&q,21,qh_bool(o->text!=NULL));
    if(o->text)qh_set(&q,22,qh_bool(o->text_len>0));

    /* PDF cell 23 records child extractor outcome, not whether text was later admitted. */
    if(pdf){
        if(pdf_extractor_succeeded_status(o->text_status))qh_set(&q,23,QH_TRUE);
        else if(status_is(o->text_status,"PDF_EXTRACTOR_UNAVAILABLE_OR_FAILED"))qh_set(&q,23,QH_FALSE);
        /* UNKNOWN means the extractor was not run or no determinate outcome was recorded. */
    }
    if(file)qh_set(&q,24,qh_bool(qh_text_candidate(o)));
    if(o->text)qh_set(&q,25,QH_TRUE);
    else if(status_is(o->text_status,"INVALID_UTF8"))qh_set(&q,25,QH_FALSE);
    if(o->text)qh_set(&q,26,QH_TRUE);
    qh_set(&q,27,QH_TRUE);
    qh_set(&q,28,QH_TRUE);
    qh_set(&q,29,QH_TRUE);
    qh_set(&q,30,QH_TRUE);
    if(link)qh_set(&q,31,QH_TRUE);
    return q;
}
static bool get_cell_equals(const qh256_t *q,unsigned cell,qh_state_t expected){
    qh_state_t actual=QH_UNKNOWN;
    return qh256_get_cell(q,cell,&actual)&&actual==expected;
}
static bool mapping_profile_tests(void){
    Object o;qh256_t q;
    memset(&o,0,sizeof(o));strcpy(o.type,"file");o.path="source.txt";strcpy(o.content_sha,"-");
    strcpy(o.text_status,"READ_FAILED");q=map_object_qh(&o);
    if(!get_cell_equals(&q,21,QH_FALSE)||!get_cell_equals(&q,25,QH_UNKNOWN))return false;

    strcpy(o.text_status,"INVALID_UTF8");q=map_object_qh(&o);
    if(!get_cell_equals(&q,21,QH_FALSE)||!get_cell_equals(&q,25,QH_FALSE))return false;

    strcpy(o.text_status,"EMBEDDED_NUL");q=map_object_qh(&o);
    if(!get_cell_equals(&q,21,QH_FALSE)||!get_cell_equals(&q,25,QH_UNKNOWN))return false;

    o.text=(unsigned char*)"";o.text_len=0;strcpy(o.text_status,"CAPTURED_EMPTY");q=map_object_qh(&o);
    if(!get_cell_equals(&q,21,QH_TRUE)||!get_cell_equals(&q,22,QH_FALSE)||!get_cell_equals(&q,25,QH_TRUE))return false;
    o.text=NULL;o.text_len=0;strcpy(o.text_status,"NOT_CANDIDATE");o.path="binary.dat";q=map_object_qh(&o);
    if(!get_cell_equals(&q,21,QH_UNKNOWN)||!get_cell_equals(&q,25,QH_UNKNOWN))return false;

    o.path="document.pdf";strcpy(o.text_status,"PDF_EXTRACTOR_UNAVAILABLE_OR_FAILED");q=map_object_qh(&o);
    if(!get_cell_equals(&q,21,QH_FALSE)||!get_cell_equals(&q,23,QH_FALSE)||!get_cell_equals(&q,25,QH_UNKNOWN))return false;
    strcpy(o.text_status,"SKIPPED_TOTAL_LIMIT");q=map_object_qh(&o);
    if(!get_cell_equals(&q,23,QH_UNKNOWN))return false;
    strcpy(o.text_status,"PDF_OUTPUT_FILE_LIMIT");q=map_object_qh(&o);
    if(!get_cell_equals(&q,23,QH_TRUE))return false;
    strcpy(o.text_status,"PDF_CAPTURE_SKIPPED_TOTAL_LIMIT");q=map_object_qh(&o);
    if(!get_cell_equals(&q,23,QH_TRUE))return false;
    strcpy(o.text_status,"PDF_CAPTURE_READ_FAILED");q=map_object_qh(&o);
    if(!get_cell_equals(&q,23,QH_TRUE))return false;
    strcpy(o.text_status,"INVALID_UTF8");q=map_object_qh(&o);
    if(!get_cell_equals(&q,23,QH_TRUE)||!get_cell_equals(&q,25,QH_FALSE))return false;
    strcpy(o.text_status,"EMBEDDED_NUL");q=map_object_qh(&o);
    if(!get_cell_equals(&q,23,QH_TRUE)||!get_cell_equals(&q,25,QH_UNKNOWN))return false;
    o.text=(unsigned char*)"PDF";o.text_len=3;strcpy(o.text_status,"CAPTURED");q=map_object_qh(&o);
    if(!get_cell_equals(&q,21,QH_TRUE)||!get_cell_equals(&q,22,QH_TRUE)||
       !get_cell_equals(&q,23,QH_TRUE)||!get_cell_equals(&q,25,QH_TRUE))return false;
    o.text=NULL;o.text_len=0;
    return true;
}
static bool sha256_tests(void){
    static const char empty_expected[]="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    static const char abc_expected[]="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    char got[65];sha256_ctx ctx;uint8_t digest[32];
    if(!sha256_buf("",0,got)||strcmp(got,empty_expected)!=0)return false;
    if(!sha256_buf("abc",3,got)||strcmp(got,abc_expected)!=0)return false;
    sha256_init(&ctx);sha256_update(&ctx,(const uint8_t*)"a",1);sha256_update(&ctx,(const uint8_t*)"bc",2);sha256_final(&ctx,digest);bytes_to_hex(digest,32,got);
    return strcmp(got,abc_expected)==0;
}
static qh256_t compute_qh(const ObjectVec *v,const EventVec *ev,bool source_scope_checked,bool parent_verified,bool frame_id_absent,bool candidate_validated,bool index_validated,bool gateway_validated) {
    qh256_t aggregate=qh256_unknown();
    size_t reg=0,dirs=0,links=0,others=0,textcand=0,textcount=0,nonempty=0,pdfs=0,zero=0,markdown=0,config=0,sourcecode=0,pdffiles=0,docx=0,hidden=0,nonascii=0,large=0,skipped=0,extensionless=0;
    for(size_t i=0;i<v->n;i++){const Object *o=&v->v[i];qh256_t oq=map_object_qh(o);aggregate=qh256_merge(aggregate,oq);reg+=is_regular(o);dirs+=strcmp(o->type,"directory")==0;links+=strcmp(o->type,"symlink")==0;others+=strcmp(o->type,"other")==0;textcand+=qh_text_candidate(o);textcount+=o->text!=NULL;nonempty+=is_text_nonempty(o);pdfs+=o->text&&has_suffix_ci(o->path,".pdf");zero+=is_zero_file(o);config+=is_regular(o)&&is_config_candidate(o->path);sourcecode+=is_regular(o)&&is_source_candidate(o->path);markdown+=is_regular(o)&&(has_suffix_ci(o->path,".md")||has_suffix_ci(o->path,".txt")||has_suffix_ci(o->path,".markdown"));pdffiles+=is_regular(o)&&has_suffix_ci(o->path,".pdf");docx+=is_regular(o)&&has_suffix_ci(o->path,".docx");hidden+=is_hidden_path(o->path);nonascii+=has_non_ascii(o->path);large+=is_regular(o)&&o->size>8u*1024u*1024u;skipped+=qh_text_candidate(o)&&o->text==NULL;const char *base=strrchr(o->path,'/');base=base?base+1:o->path;extensionless+=is_regular(o)&&strchr(base,'.')==NULL;}
    /* Cells 32..47 describe one complete transition/proof operation, not individual objects. */
    qh256_t run=qh256_unknown();qh_set(&run,32,qh_bool(ev->added>0));qh_set(&run,33,qh_bool(ev->removed>0));qh_set(&run,34,qh_bool(ev->content_changed>0));qh_set(&run,35,qh_bool(ev->metadata_changed>0));qh_set(&run,36,qh_bool(ev->unchanged>0));
    bool initial=ev->added==v->n&&ev->removed==0&&ev->content_changed==0&&ev->metadata_changed==0&&ev->unchanged==0;
    qh_set(&run,37,qh_bool(initial));qh_set(&run,38,qh_bool(!initial));if(parent_verified)qh_set(&run,39,QH_TRUE);
    qh_set(&run,40,QH_TRUE);qh_set(&run,41,QH_TRUE);qh_set(&run,42,qh_bool(candidate_validated));qh_set(&run,43,qh_bool(index_validated));qh_set(&run,44,qh_bool(gateway_validated));qh_set(&run,45,qh_bool(qh_tests()));qh_set(&run,46,qh_bool(frame_id_absent));qh_set(&run,47,qh_bool(source_scope_checked));
    /* Cells 48..63 report complete-inventory features; FALSE is used only when absence was observed over a full scan. */
    qh_set(&run,48,qh_bool(docx>0));qh_set(&run,49,qh_bool(markdown>0));qh_set(&run,50,qh_bool(config>0));qh_set(&run,51,qh_bool(sourcecode>0));qh_set(&run,52,qh_bool(pdffiles>0));qh_set(&run,53,qh_bool(zero>0));qh_set(&run,54,qh_bool(hidden>0));qh_set(&run,55,qh_bool(nonascii>0));qh_set(&run,56,qh_bool(large>0));qh_set(&run,57,qh_bool(others>0));qh_set(&run,58,qh_bool(skipped>0));qh_set(&run,59,qh_bool(extensionless>0));qh_set(&run,60,qh_bool(has_duplicate_content(v)));qh_set(&run,61,qh_bool(reg>=2));qh_set(&run,62,qh_bool(dirs>0));qh_set(&run,63,qh_bool(links>0));
    aggregate=qh256_merge(aggregate,run);(void)links;(void)others;(void)textcand;(void)textcount;(void)nonempty;(void)pdfs;(void)dirs;return aggregate;
}
static void qh_to_hex(const qh256_t *qh,char out[65]){uint8_t wire[QH256_BYTES];if(!qh256_serialize(qh,wire))die("ERROR: QH256 serialize failed");bytes_to_hex(wire,QH256_BYTES,out);}
static bool qh_from_hex(const char *s,qh256_t *out){if(strlen(s)!=QH256_BYTES*2)return false;uint8_t wire[QH256_BYTES];for(size_t i=0;i<QH256_BYTES;i++){int a=s[i*2],b=s[i*2+1];if(a>='0'&&a<='9')a-='0';else if(a>='a'&&a<='f')a=a-'a'+10;else return false;if(b>='0'&&b<='9')b-='0';else if(b>='a'&&b<='f')b=b-'a'+10;else return false;wire[i]=(uint8_t)((a<<4)|b);}return qh256_deserialize(out,wire);}
static void write_qh(const char *path,const qh256_t *qh){char h[65];qh_to_hex(qh,h);write_printf_file(path,"QH256-V2\nprofile\t%s\nhex\t%s\n",MAPPING_PROFILE,h);}
static void write_text_capture(const char *path,const ObjectVec *v) {
    int fd=open(path,O_WRONLY|O_CREAT|O_EXCL,0644);if(fd<0)die("ERROR: cannot create text capture: %s",strerror(errno));FILE *f=fdopen(fd,"w");if(!f){close(fd);die("ERROR: fdopen failed");}fprintf(f,"%s\n",TEXT_MAGIC);
    for(size_t i=0;i<v->n;i++){const Object *o=&v->v[i];if(!o->text)continue;char *p64=NULL,*t64=NULL;base64_encode((const unsigned char*)o->path,strlen(o->path),&p64);base64_encode(o->text,o->text_len,&t64);fprintf(f,"T\t%s\t%s\t%s\t%s\t%s\t%zu\t%s\n",o->object_id,o->content_sha,o->text_sha,o->extractor,p64,o->text_len,t64);free(p64);free(t64);}
    if(fflush(f)!=0||fsync(fd)!=0){fclose(f);die("ERROR: text capture flush failed");}fclose(f);
}
static void write_index(const char *path,const ObjectVec *v) {
    int fd=open(path,O_WRONLY|O_CREAT|O_EXCL,0644);if(fd<0)die("ERROR: cannot create index: %s",strerror(errno));FILE *f=fdopen(fd,"w");if(!f){close(fd);die("ERROR: fdopen failed");}fprintf(f,"%s\nindex_status\tDERIVED\nmapping_profile\t%s\n",INDEX_MAGIC,MAPPING_PROFILE);
    for(size_t i=0;i<v->n;i++){const Object *o=&v->v[i];char *p64=NULL,qhhex[65];base64_encode((const unsigned char*)o->path,strlen(o->path),&p64);qh256_t oq=map_object_qh(o);qh_to_hex(&oq,qhhex);fprintf(f,"I\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",o->object_id,o->record_hash,o->content_sha,o->type,o->text?o->text_sha:"-",o->text?o->extractor:"-",(o->text&&o->text_len>0)?"SEARCHABLE":"STRUCTURAL_ONLY",p64,o->text_status,qhhex);free(p64);}
    if(fflush(f)!=0||fsync(fd)!=0){fclose(f);die("ERROR: index flush failed");}fclose(f);
}
static size_t split_tabs_preserve_empty(char *line,char **fields,size_t capacity){
    size_t count=1;
    if(capacity==0)return 0;
    fields[0]=line;
    for(char *p=line;*p;p++){
        if(*p=='\t'){
            *p='\0';
            if(count>=capacity)return capacity+1;
            fields[count++]=p+1;
        }
    }
    return count;
}
static bool validate_index_file(const char *path,const ObjectVec *v) {
    FILE *f=fopen(path,"rb");
    if(!f)return false;
    char *line=NULL;size_t cap=0;ssize_t n;size_t ix=0;int header=0;bool ok=true;
    while((n=getline(&line,&cap,f))!=-1){
        if(n&&line[n-1]=='\n')line[--n]='\0';
        if(n&&line[n-1]=='\r')line[--n]='\0';
        if(header==0){if(strcmp(line,INDEX_MAGIC))ok=false;header++;continue;}
        if(header==1){if(strcmp(line,"index_status\tDERIVED"))ok=false;header++;if(!ok)break;continue;}
        if(header==2){char expected[256];snprintf(expected,sizeof(expected),"mapping_profile\t%s",MAPPING_PROFILE);if(strcmp(line,expected))ok=false;header++;if(!ok)break;continue;}
        if(!*line)continue;
        char *fields[12];size_t nf=split_tabs_preserve_empty(line,fields,12);
        if(nf!=11||strcmp(fields[0],"I")||ix>=v->n){ok=false;break;}
        const Object *o=&v->v[ix];
        char *p64=NULL;char objqh[65];
        base64_encode((const unsigned char*)o->path,strlen(o->path),&p64);
        const char *want[8]={o->object_id,o->record_hash,o->content_sha,o->type,
            o->text?o->text_sha:"-",o->text?o->extractor:"-",
            (o->text&&o->text_len>0)?"SEARCHABLE":"STRUCTURAL_ONLY",p64};
        for(size_t k=0;k<8;k++)if(strcmp(fields[k+1],want[k])!=0){ok=false;break;}
        if(ok&&!text_status_valid_for_object(o,fields[9]))ok=false;
        if(ok){qh256_t qobj=map_object_qh(o);qh_to_hex(&qobj,objqh);if(strcmp(fields[10],objqh))ok=false;}
        free(p64);
        if(!ok)break;
        ix++;
    }
    if(ferror(f))ok=false;
    free(line);fclose(f);
    return ok&&header==3&&ix==v->n;
}
static void write_gateway(const char *path,const FrameMeta *m,const ObjectVec *v) {
    int fd=open(path,O_WRONLY|O_CREAT|O_EXCL,0644);if(fd<0)die("ERROR: cannot create Gateway: %s",strerror(errno));FILE *f=fdopen(fd,"w");if(!f){close(fd);die("ERROR: fdopen failed");}
    fprintf(f,"%s\nrole\tHUMAN_AND_MACHINE_ORIENTATION\nstatus\tDERIVED\nframe_id\t%s\nroot_id\t%s\nsequence\t%" PRIu64 "\nstate_file\tSTATE.tsv\nproof_file\tPROOF.tsv\ntransition_file\tTRANSITION.tsv\nindex_file\tINDEX.tsv\ntext_file\tTEXT.tsv\nqh256_file\tQH256.hex\n",GATEWAY_MAGIC,m->frame_id,m->root_id,m->sequence);
    for(size_t i=0;i<v->n;i++){const Object *o=&v->v[i];if(strchr(o->path,'/')!=NULL||strcmp(o->path,".")==0)continue;char *p64=NULL;base64_encode((const unsigned char*)o->path,strlen(o->path),&p64);fprintf(f,"ENTRY\t%s\t%s\t%s\n",o->object_id,o->type,p64);free(p64);}
    if(fflush(f)!=0||fsync(fd)!=0){fclose(f);die("ERROR: Gateway flush failed");}fclose(f);
}
static bool validate_gateway_file(const char *path,const FrameMeta *m,const ObjectVec *v) {
    FILE *f=fopen(path,"rb");if(!f)return false;char *line=NULL;size_t cap=0;ssize_t n;int header=0;size_t ix=0;bool ok=true;
    char expected[12][512];
    snprintf(expected[0],sizeof(expected[0]),"%s",GATEWAY_MAGIC);
    snprintf(expected[1],sizeof(expected[1]),"role\tHUMAN_AND_MACHINE_ORIENTATION");
    snprintf(expected[2],sizeof(expected[2]),"status\tDERIVED");
    snprintf(expected[3],sizeof(expected[3]),"frame_id\t%s",m->frame_id);
    snprintf(expected[4],sizeof(expected[4]),"root_id\t%s",m->root_id);
    snprintf(expected[5],sizeof(expected[5]),"sequence\t%" PRIu64,m->sequence);
    snprintf(expected[6],sizeof(expected[6]),"state_file\tSTATE.tsv");snprintf(expected[7],sizeof(expected[7]),"proof_file\tPROOF.tsv");snprintf(expected[8],sizeof(expected[8]),"transition_file\tTRANSITION.tsv");snprintf(expected[9],sizeof(expected[9]),"index_file\tINDEX.tsv");snprintf(expected[10],sizeof(expected[10]),"text_file\tTEXT.tsv");snprintf(expected[11],sizeof(expected[11]),"qh256_file\tQH256.hex");
    while((n=getline(&line,&cap,f))!=-1){if(n&&line[n-1]=='\n')line[--n]='\0';if(n&&line[n-1]=='\r')line[--n]='\0';if(header<12){if(strcmp(line,expected[header])!=0)ok=false;header++;if(!ok)break;continue;}
        while(ix<v->n&&(strchr(v->v[ix].path,'/')!=NULL||strcmp(v->v[ix].path,".")==0))ix++;
        if(ix>=v->n){ok=false;break;}char *fields[6];size_t nf=0;char *save=NULL;for(char *p=strtok_r(line,"\t",&save);p&&nf<6;p=strtok_r(NULL,"\t",&save))fields[nf++]=p;
        const Object *o=&v->v[ix];char *p64=NULL;base64_encode((const unsigned char*)o->path,strlen(o->path),&p64);
        if(nf!=4||strcmp(fields[0],"ENTRY")||strcmp(fields[1],o->object_id)||strcmp(fields[2],o->type)||strcmp(fields[3],p64))ok=false;
        free(p64);if(!ok)break;ix++;
    }
    while(ix<v->n&&(strchr(v->v[ix].path,'/')!=NULL||strcmp(v->v[ix].path,".")==0))ix++;
    if(ferror(f)||header!=12||ix!=v->n)ok=false;free(line);fclose(f);return ok;
}
static void format_utc(time_t t,char out[64]){struct tm tmv;if(!gmtime_r(&t,&tmv)){strcpy(out,"UNKNOWN");return;}strftime(out,64,"%Y-%m-%dT%H:%M:%SZ",&tmv);}
static void write_frame(const char *path,const FrameMeta *m,const char *created,const char *state_sha,const char *chain_sha,const char *framehash_unused) {
    (void)framehash_unused;write_printf_file(path,"%s\nimplementation\t%s\nstatus\tDERIVED_CANDIDATE\nframe_id\t%s\ncreated_utc\t%s\ntime_source\tPOSIX_CLOCK_REALTIME_UTC\nroot_id\t%s\nsequence\t%" PRIu64 "\nparent_frame_id\t%s\nparent_state_sha256\t%s\nstate_sha256\t%s\nchain_sha256\t%s\nobject_count\t%" PRIu64 "\nadded\t%" PRIu64 "\nremoved\t%" PRIu64 "\ncontent_changed\t%" PRIu64 "\nmetadata_changed\t%" PRIu64 "\nunchanged\t%" PRIu64 "\nstate_file\tSTATE.tsv\nproof_file\tPROOF.tsv\nindex_file\tINDEX.tsv\ntext_file\tTEXT.tsv\ntransition_file\tTRANSITION.tsv\ngateway_file\tGATEWAY.tsv\nqh256_file\tQH256.hex\nmapping_profile\t%s\n",FRAME_MAGIC,VERSION,m->frame_id,created,m->root_id,m->sequence,m->parent_frame_id[0]?m->parent_frame_id:"-",m->parent_state_sha[0]?m->parent_state_sha:"-",state_sha,chain_sha,m->object_count,m->added,m->removed,m->content_changed,m->metadata_changed,m->unchanged,MAPPING_PROFILE);
}
static void write_proof(const char *path,const FrameMeta *m,const char *state_sha,const char *chain_sha,const char *frame_sha,const char *index_sha,const char *text_sha,const char *transition_sha,const char *gateway_sha,const char *qhsha,const char *manifest_excluded) {
    (void)manifest_excluded;write_printf_file(path,"%s\nstatus\tPASS\ncontract\tK501-PROOF-BEFORE-STATE-V3A1\nrule\tCANDIDATE_REVALIDATED_AND_PROOF_DURABLE_BEFORE_STATE_COMMIT\nframe_id\t%s\nroot_id\t%s\nsequence\t%" PRIu64 "\nparent_frame_id\t%s\nparent_state_sha256\t%s\nstate_sha256\t%s\nchain_sha256\t%s\nframe_sha256\t%s\nindex_sha256\t%s\ntext_sha256\t%s\ntransition_sha256\t%s\ngateway_sha256\t%s\nqh256_file_sha256\t%s\ncheck_01\tPASS:SOURCE_AND_STORE_NON_OVERLAP\ncheck_02\tPASS:PREDECESSOR_VALIDATED_OR_INITIAL_FRAME\ncheck_03\tPASS:DETERMINISTIC_PATH_ORDER\ncheck_04\tPASS:OBJECT_ID_RECOMPUTED\ncheck_05\tPASS:OBJECT_RECORD_HASH_RECOMPUTED\ncheck_06\tPASS:STATE_CANDIDATE_REHASHED\ncheck_07\tPASS:STATE_CANDIDATE_PARSED_AND_COUNT_MATCHED\ncheck_08\tPASS:INDEX_DERIVED_FROM_CANDIDATE_STATE\ncheck_09\tPASS:TEXT_CAPTURE_BOUND_TO_SOURCE_CONTENT_HASH\ncheck_10\tPASS:QH256_SERIALIZATION_CHECKED\ncheck_11\tPASS:FRAME_AND_GATEWAY_REFERENCES_BOUND\ncheck_12\tPASS:OUTPUT_FRAME_ID_ABSENT\ncheck_13\tPASS:PATH_BASE64_ROUNDTRIP\ncheck_14\tPASS:INDEX_STATUS_VALIDATED_BEFORE_QH256\ncommit_rule\tPROOF_FILE_FSYNC_COMPLETES_BEFORE_STATE_CANDIDATE_RENAMED_TO_STATE_TSV\nnon_claim\tPASS_PROVES_DECLARED_LOCAL_CHECKS_ONLY_NOT_EXTERNAL_TRUTH_OR_RUNTIME_SECURITY\n",PROOF_MAGIC,m->frame_id,m->root_id,m->sequence,m->parent_frame_id[0]?m->parent_frame_id:"-",m->parent_state_sha[0]?m->parent_state_sha:"-",state_sha,chain_sha,frame_sha,index_sha,text_sha,transition_sha,gateway_sha,qhsha);
}
static void write_manifest(const char *dir) {
    static const char *files[]={"FRAME.tsv","GATEWAY.tsv","INDEX.tsv","PROOF.tsv","QH256.hex","STATE.tsv","TEXT.tsv","TRANSITION.tsv"};
    char *p=join_path(dir,"MANIFEST.sha256");int fd=open(p,O_WRONLY|O_CREAT|O_EXCL,0644);if(fd<0){free(p);die("ERROR: cannot create manifest");}FILE *f=fdopen(fd,"w");if(!f){free(p);close(fd);die("ERROR: fdopen failed");}
    for(size_t i=0;i<sizeof(files)/sizeof(files[0]);i++){char *q=join_path(dir,files[i]),h[65];if(!sha256_file(q,h)){free(q);fclose(f);free(p);die("ERROR: cannot hash artifact for manifest");}fprintf(f,"%s  %s\n",h,files[i]);free(q);}
    if(fflush(f)!=0||fsync(fd)!=0){fclose(f);free(p);die("ERROR: manifest flush failed");}fclose(f);free(p);
}
static bool validate_object_hashes(const char *root_id,ObjectVec *v) {
    for(size_t i=0;i<v->n;i++){Object *o=&v->v[i];char id[65],rh[65];hash_domain4(OBJECT_ID_DOMAIN,root_id,o->path,o->type,id);record_hash_for(o,root_id,rh);if(strcmp(id,o->object_id)||strcmp(rh,o->record_hash))return false;}
    return true;
}
static bool load_frame_meta(const char *dir,FrameMeta *m) {
    char *p=join_path(dir,"FRAME.tsv");memset(m,0,sizeof(*m));char b[1024];bool ok=true;
#define READK(k,field) do{if(!read_key_file(p,k,b,sizeof(b))||!copy_field(m->field,sizeof(m->field),b)){ok=false;}}while(0)
    READK("frame_id",frame_id);READK("root_id",root_id);READK("parent_frame_id",parent_frame_id);READK("parent_state_sha256",parent_state_sha);
#undef READK
    if(read_key_file(p,"sequence",b,sizeof(b)))m->sequence=strtoull(b,NULL,10);else ok=false;
    if(read_key_file(p,"state_sha256",b,sizeof(b)))strcpy(m->state_sha,b);else ok=false;
    if(read_key_file(p,"chain_sha256",b,sizeof(b)))strcpy(m->chain_sha,b);else ok=false;
    if(read_key_file(p,"object_count",b,sizeof(b)))m->object_count=strtoull(b,NULL,10);else ok=false;
    if(read_key_file(p,"added",b,sizeof(b)))m->added=strtoull(b,NULL,10);else ok=false;
    if(read_key_file(p,"removed",b,sizeof(b)))m->removed=strtoull(b,NULL,10);else ok=false;
    if(read_key_file(p,"content_changed",b,sizeof(b)))m->content_changed=strtoull(b,NULL,10);else ok=false;
    if(read_key_file(p,"metadata_changed",b,sizeof(b)))m->metadata_changed=strtoull(b,NULL,10);else ok=false;
    if(read_key_file(p,"unchanged",b,sizeof(b)))m->unchanged=strtoull(b,NULL,10);else ok=false;
    if(read_key_file(p,"status",b,sizeof(b))&&strcmp(b,"DERIVED_CANDIDATE")!=0)ok=false;
    free(p);return ok;
}
static bool validate_frame_file(const char *path,const FrameMeta *m) {
    char b[1024],expect[256];
    if(!read_key_file(path,"implementation",b,sizeof(b))||strcmp(b,VERSION))return false;
    if(!read_key_file(path,"status",b,sizeof(b))||strcmp(b,"DERIVED_CANDIDATE"))return false;
    if(!read_key_file(path,"time_source",b,sizeof(b))||strcmp(b,"POSIX_CLOCK_REALTIME_UTC"))return false;
    if(!read_key_file(path,"state_file",b,sizeof(b))||strcmp(b,"STATE.tsv"))return false;
    if(!read_key_file(path,"proof_file",b,sizeof(b))||strcmp(b,"PROOF.tsv"))return false;
    if(!read_key_file(path,"index_file",b,sizeof(b))||strcmp(b,"INDEX.tsv"))return false;
    if(!read_key_file(path,"text_file",b,sizeof(b))||strcmp(b,"TEXT.tsv"))return false;
    if(!read_key_file(path,"transition_file",b,sizeof(b))||strcmp(b,"TRANSITION.tsv"))return false;
    if(!read_key_file(path,"gateway_file",b,sizeof(b))||strcmp(b,"GATEWAY.tsv"))return false;
    if(!read_key_file(path,"qh256_file",b,sizeof(b))||strcmp(b,"QH256.hex"))return false;
    snprintf(expect,sizeof(expect),"%s",MAPPING_PROFILE);if(!read_key_file(path,"mapping_profile",b,sizeof(b))||strcmp(b,expect))return false;
    if(!read_key_file(path,"frame_id",b,sizeof(b))||strcmp(b,m->frame_id))return false;
    if(!read_key_file(path,"root_id",b,sizeof(b))||strcmp(b,m->root_id))return false;
    if(!read_key_file(path,"sequence",b,sizeof(b))||strtoull(b,NULL,10)!=m->sequence)return false;
    if(!read_key_file(path,"parent_frame_id",b,sizeof(b))||strcmp(b,m->parent_frame_id))return false;
    if(!read_key_file(path,"parent_state_sha256",b,sizeof(b))||strcmp(b,m->parent_state_sha))return false;
    if(!read_key_file(path,"state_sha256",b,sizeof(b))||strcmp(b,m->state_sha))return false;
    if(!read_key_file(path,"chain_sha256",b,sizeof(b))||strcmp(b,m->chain_sha))return false;
    if(!read_key_file(path,"object_count",b,sizeof(b))||strtoull(b,NULL,10)!=m->object_count)return false;
    if(!read_key_file(path,"added",b,sizeof(b))||strtoull(b,NULL,10)!=m->added)return false;
    if(!read_key_file(path,"removed",b,sizeof(b))||strtoull(b,NULL,10)!=m->removed)return false;
    if(!read_key_file(path,"content_changed",b,sizeof(b))||strtoull(b,NULL,10)!=m->content_changed)return false;
    if(!read_key_file(path,"metadata_changed",b,sizeof(b))||strtoull(b,NULL,10)!=m->metadata_changed)return false;
    if(!read_key_file(path,"unchanged",b,sizeof(b))||strtoull(b,NULL,10)!=m->unchanged)return false;
    return true;
}
static bool validate_proof_contract(const char *path,const FrameMeta *m) {
    char b[1024];
    if(!read_key_file(path,"contract",b,sizeof(b))||strcmp(b,"K501-PROOF-BEFORE-STATE-V3A1"))return false;
    if(!read_key_file(path,"rule",b,sizeof(b))||strcmp(b,"CANDIDATE_REVALIDATED_AND_PROOF_DURABLE_BEFORE_STATE_COMMIT"))return false;
    if(!read_key_file(path,"commit_rule",b,sizeof(b))||strcmp(b,"PROOF_FILE_FSYNC_COMPLETES_BEFORE_STATE_CANDIDATE_RENAMED_TO_STATE_TSV"))return false;
    if(!read_key_file(path,"non_claim",b,sizeof(b))||strcmp(b,"PASS_PROVES_DECLARED_LOCAL_CHECKS_ONLY_NOT_EXTERNAL_TRUTH_OR_RUNTIME_SECURITY"))return false;
    const char *keys[]={"check_01","check_02","check_03","check_04","check_05","check_06","check_07","check_08","check_09","check_10","check_11","check_12","check_13","check_14"};
    const char *values[]={"PASS:SOURCE_AND_STORE_NON_OVERLAP","PASS:PREDECESSOR_VALIDATED_OR_INITIAL_FRAME","PASS:DETERMINISTIC_PATH_ORDER","PASS:OBJECT_ID_RECOMPUTED","PASS:OBJECT_RECORD_HASH_RECOMPUTED","PASS:STATE_CANDIDATE_REHASHED","PASS:STATE_CANDIDATE_PARSED_AND_COUNT_MATCHED","PASS:INDEX_DERIVED_FROM_CANDIDATE_STATE","PASS:TEXT_CAPTURE_BOUND_TO_SOURCE_CONTENT_HASH","PASS:QH256_SERIALIZATION_CHECKED","PASS:FRAME_AND_GATEWAY_REFERENCES_BOUND","PASS:OUTPUT_FRAME_ID_ABSENT","PASS:PATH_BASE64_ROUNDTRIP","PASS:INDEX_STATUS_VALIDATED_BEFORE_QH256"};
    for(size_t i=0;i<14;i++)if(!read_key_file(path,keys[i],b,sizeof(b))||strcmp(b,values[i]))return false;
    if(!read_key_file(path,"frame_id",b,sizeof(b))||strcmp(b,m->frame_id))return false;
    if(!read_key_file(path,"root_id",b,sizeof(b))||strcmp(b,m->root_id))return false;
    if(!read_key_file(path,"sequence",b,sizeof(b))||strtoull(b,NULL,10)!=m->sequence)return false;
    return true;
}
static bool verify_manifest(const char *dir) {
    static const char *files[]={"FRAME.tsv","GATEWAY.tsv","INDEX.tsv","PROOF.tsv","QH256.hex","STATE.tsv","TEXT.tsv","TRANSITION.tsv"};
    char *mp=join_path(dir,"MANIFEST.sha256");FILE *f=fopen(mp,"rb");free(mp);if(!f)return false;char *line=NULL;size_t cap=0;ssize_t n;size_t count=0;bool ok=true;
    while((n=getline(&line,&cap,f))!=-1){if(n&&line[n-1]=='\n')line[--n]='\0';if(n&&line[n-1]=='\r')line[--n]='\0';if(count>=8||n<67||line[64]!=' '||line[65]!=' ') {ok=false;break;}char want[65];memcpy(want,line,64);want[64]=0;const char *name=line+66;if(!is_hex64(want)||strcmp(name,files[count])!=0){ok=false;break;}char *p=join_path(dir,name),got[65];if(!sha256_file(p,got)||strcmp(got,want)!=0)ok=false;free(p);if(!ok)break;count++;}
    free(line);fclose(f);return ok&&count==8;
}
static bool load_text_entries(const char *path,TextVec *v);
static bool verify_frame_depth(const char *store,const char *frame,FrameMeta *m,ObjectVec *v,unsigned depth) {
    if(depth>4096||!safe_id(frame))return false;
    char *dir=frame_dir(store,frame); bool ok=load_frame_meta(dir,m); if(!ok){free(dir);return false;}
    char parent_chain[65]="-"; bool parent_verified=false;ObjectVec parent_objects={0};
    if(strcmp(m->parent_frame_id,"-")==0) {
        if(m->sequence!=1||strcmp(m->parent_state_sha,"-")!=0)ok=false;
    } else {
        if(!safe_id(m->parent_frame_id)||!is_hex64(m->parent_state_sha))ok=false;
        if(ok){FrameMeta pm;ObjectVec pv={0};ok=verify_frame_depth(store,m->parent_frame_id,&pm,&pv,depth+1);
            if(ok){ok=(strcmp(pm.root_id,m->root_id)==0 && pm.sequence+1==m->sequence && strcmp(pm.state_sha,m->parent_state_sha)==0);if(ok){strcpy(parent_chain,pm.chain_sha);parent_verified=true;parent_objects=pv;memset(&pv,0,sizeof(pv));}}
            vec_free(&pv);
        }
    }
    char expected_chain[65]; if(!chain_hash_for(parent_chain,m->state_sha,expected_chain)||strcmp(expected_chain,m->chain_sha)!=0)ok=false;
    char *sp=join_path(dir,"STATE.tsv"),*fp=join_path(dir,"FRAME.tsv"),*pp=join_path(dir,"PROOF.tsv"),*ip=join_path(dir,"INDEX.tsv"),*tp=join_path(dir,"TEXT.tsv"),*trp=join_path(dir,"TRANSITION.tsv"),*gp=join_path(dir,"GATEWAY.tsv"),*qhp=join_path(dir,"QH256.hex");
    char h[65],b[1024],proofstate[65],proof_frame[65],proof_index[65],proof_text[65],proof_transition[65],proof_gateway[65],proof_qh[65];FrameMeta sm;ObjectVec sv={0};TextVec tv={0};
    ok=ok&&sha256_file(sp,h)&&strcmp(h,m->state_sha)==0;
    ok=ok&&parse_state(sp,frame,&sv,&sm)&&strcmp(sm.root_id,m->root_id)==0&&sm.sequence==m->sequence&&sm.object_count==m->object_count&&strcmp(sm.parent_frame_id,m->parent_frame_id)==0&&strcmp(sm.parent_state_sha,m->parent_state_sha)==0;
    ok=ok&&validate_object_hashes(m->root_id,&sv);
    char fh[65],ih[65],th[65],trh[65],gh[65],qhh[65];
    ok=ok&&sha256_file(fp,fh)&&sha256_file(ip,ih)&&sha256_file(tp,th)&&sha256_file(trp,trh)&&sha256_file(gp,gh)&&sha256_file(qhp,qhh);
    ok=ok&&validate_frame_file(fp,m);
    ok=ok&&read_key_file(pp,"status",b,sizeof(b))&&strcmp(b,"PASS")==0;
    ok=ok&&validate_proof_contract(pp,m);
    ok=ok&&read_key_file(pp,"frame_id",b,sizeof(b))&&strcmp(b,m->frame_id)==0;
    ok=ok&&read_key_file(pp,"root_id",b,sizeof(b))&&strcmp(b,m->root_id)==0;
    ok=ok&&read_key_file(pp,"sequence",b,sizeof(b))&&strtoull(b,NULL,10)==m->sequence;
    ok=ok&&read_key_file(pp,"state_sha256",proofstate,sizeof(proofstate))&&strcmp(proofstate,m->state_sha)==0;
    ok=ok&&read_key_file(pp,"chain_sha256",b,sizeof(b))&&strcmp(b,m->chain_sha)==0;
    ok=ok&&read_key_file(pp,"frame_sha256",proof_frame,sizeof(proof_frame))&&strcmp(proof_frame,fh)==0;
    ok=ok&&read_key_file(pp,"index_sha256",proof_index,sizeof(proof_index))&&strcmp(proof_index,ih)==0;
    ok=ok&&read_key_file(pp,"text_sha256",proof_text,sizeof(proof_text))&&strcmp(proof_text,th)==0;
    ok=ok&&read_key_file(pp,"transition_sha256",proof_transition,sizeof(proof_transition))&&strcmp(proof_transition,trh)==0;
    ok=ok&&read_key_file(pp,"gateway_sha256",proof_gateway,sizeof(proof_gateway))&&strcmp(proof_gateway,gh)==0;
    ok=ok&&read_key_file(pp,"qh256_file_sha256",proof_qh,sizeof(proof_qh))&&strcmp(proof_qh,qhh)==0;
    ok=ok&&load_text_entries(tp,&tv);
    /* Each derived text record must bind to exactly one current object and its source digest/path. */
    if(ok){for(size_t i=0;i<tv.n;i++){int oi=find_id(&sv,tv.v[i].object_id);if(oi<0){ok=false;break;}Object *o=&sv.v[oi];if(strcmp(o->type,"file")!=0||strcmp(o->content_sha,tv.v[i].content_sha)!=0||strcmp(o->path,tv.v[i].path)!=0){ok=false;break;}for(size_t j=0;j<i;j++)if(strcmp(tv.v[j].object_id,tv.v[i].object_id)==0){ok=false;break;}if(!ok)break;o->text=xmalloc(1);o->text[0]=0;o->text_len=tv.v[i].text_len;strcpy(o->text_sha,tv.v[i].text_sha);strcpy(o->extractor,tv.v[i].extractor);}}
    /* Verify transition counts and parent binding. */
    uint64_t ta=0,tr=0,tc=0,tmc=0,tu=0;char trans_parent[256];
    ok=ok&&read_key_file(trp,"parent_frame_id",trans_parent,sizeof(trans_parent))&&strcmp(trans_parent,m->parent_frame_id)==0;
    if(ok){if(!read_key_file(trp,"added",b,sizeof(b)))ok=false;else ta=strtoull(b,NULL,10);}
    if(ok){if(!read_key_file(trp,"removed",b,sizeof(b)))ok=false;else tr=strtoull(b,NULL,10);}
    if(ok){if(!read_key_file(trp,"content_changed",b,sizeof(b)))ok=false;else tc=strtoull(b,NULL,10);}
    if(ok){if(!read_key_file(trp,"metadata_changed",b,sizeof(b)))ok=false;else tmc=strtoull(b,NULL,10);}
    if(ok){if(!read_key_file(trp,"unchanged",b,sizeof(b)))ok=false;else tu=strtoull(b,NULL,10);}
    if(ok&&(ta!=m->added||tr!=m->removed||tc!=m->content_changed||tmc!=m->metadata_changed||tu!=m->unchanged))ok=false;
    if(ok&&!validate_transition_semantics(trp,m->parent_frame_id,&parent_objects,&sv))ok=false;
    /* Reconstruct validated text status before recomputing per-object QH256. */
    if(ok){
        FILE *f=fopen(ip,"rb");
        if(!f)ok=false;
        else{
            char *line=NULL;size_t lcap=0;ssize_t n;int header=0;size_t ix=0;
            while((n=getline(&line,&lcap,f))!=-1){
                if(n&&line[n-1]=='\n')line[--n]='\0';
                if(n&&line[n-1]=='\r')line[--n]='\0';
                if(header==0){if(strcmp(line,INDEX_MAGIC)!=0)ok=false;header++;if(!ok)break;continue;}
                if(header==1){if(strcmp(line,"index_status\tDERIVED")!=0)ok=false;header++;if(!ok)break;continue;}
                if(header==2){char expected[256];snprintf(expected,sizeof(expected),"mapping_profile\t%s",MAPPING_PROFILE);if(strcmp(line,expected)!=0)ok=false;header++;if(!ok)break;continue;}
                if(!*line)continue;
                char *fields[12];size_t nf=split_tabs_preserve_empty(line,fields,12);
                if(nf!=11||strcmp(fields[0],"I")||ix>=sv.n||
                   strcmp(fields[1],sv.v[ix].object_id)||strcmp(fields[2],sv.v[ix].record_hash)||
                   strcmp(fields[3],sv.v[ix].content_sha)||strcmp(fields[4],sv.v[ix].type)){
                    ok=false;break;
                }
                size_t pn=0;unsigned char *pb=base64_decode(fields[8],&pn);
                if(!pb||strlen(sv.v[ix].path)!=pn||memcmp(pb,sv.v[ix].path,pn)!=0)ok=false;
                free(pb);
                if(!ok)break;

                if(!copy_field(sv.v[ix].text_status,sizeof(sv.v[ix].text_status),fields[9])||
                   !text_status_valid_for_object(&sv.v[ix],fields[9])){ok=false;break;}

                int found=-1;
                for(size_t j=0;j<tv.n;j++)if(strcmp(tv.v[j].object_id,fields[1])==0){found=(int)j;break;}
                if(found>=0){
                    const char *expected_status=tv.v[found].text_len?"SEARCHABLE":"STRUCTURAL_ONLY";
                    if(strcmp(fields[5],tv.v[found].text_sha)||strcmp(fields[6],tv.v[found].extractor)||
                       strcmp(fields[7],expected_status))ok=false;
                    if(strcmp(fields[9],tv.v[found].text_len?"CAPTURED":"CAPTURED_EMPTY"))ok=false;
                }else{
                    if(strcmp(fields[5],"-")||strcmp(fields[6],"-")||strcmp(fields[7],"STRUCTURAL_ONLY"))ok=false;
                }
                if(!ok)break;
                qh256_t qobj=map_object_qh(&sv.v[ix]);char qobjhex[65];qh_to_hex(&qobj,qobjhex);
                if(strcmp(fields[10],qobjhex)!=0){ok=false;break;}
                ix++;
            }
            if(ferror(f)||ix!=sv.n||header!=3)ok=false;
            free(line);fclose(f);
        }
    }
    /* The index is a derived projection; validate every field against state, text, and status. */
    if(ok&&!validate_index_file(ip,&sv))ok=false;
    /* Check machine Gateway bindings. */
    if(ok&&!validate_gateway_file(gp,m,&sv))ok=false;
    /* Recompute the QH256 aggregate from state + transition counts + bound text data. */
    if(ok){EventVec fake={0};fake.added=ta;fake.removed=tr;fake.content_changed=tc;fake.metadata_changed=tmc;fake.unchanged=tu;fake.n=(size_t)(ta+tr+tc+tmc+tu);qh256_t want_qh,got_qh;want_qh=compute_qh(&sv,&fake,true,parent_verified,true,true,true,true);ok=read_key_file(qhp,"hex",b,sizeof(b))&&qh_from_hex(b,&got_qh)&&qh256_equal(&want_qh,&got_qh)&&read_key_file(qhp,"profile",b,sizeof(b))&&strcmp(b,MAPPING_PROFILE)==0;}
    ok=ok&&verify_manifest(dir);
    if(ok){*v=sv;memset(&sv,0,sizeof(sv));*m=sm;/* restore frame metadata, not state-only metadata */load_frame_meta(dir,m);}
    texts_free(&tv);vec_free(&sv);vec_free(&parent_objects);free(sp);free(fp);free(pp);free(ip);free(tp);free(trp);free(gp);free(qhp);free(dir);return ok;
}
static bool verify_frame(const char *store,const char *frame,FrameMeta *m,ObjectVec *v){return verify_frame_depth(store,frame,m,v,0);}
static int leaf_frame(const char *store,char out[256]) {
    char *fd=join_path(store,"frames");DIR *d=opendir(fd);if(!d){free(fd);return -1;}char **ids=NULL;size_t n=0,cap=0;struct dirent *de;
    while((de=readdir(d))){if(de->d_name[0]=='.'||!safe_id(de->d_name))continue;char *p=join_path(fd,de->d_name);struct stat st;if(stat(p,&st)==0&&S_ISDIR(st.st_mode)){if(n==cap){cap=cap?cap*2:16;ids=xrealloc(ids,cap*sizeof(char*));}ids[n++]=xstrdup(de->d_name);}free(p);}closedir(d);
    if(n==0){free(ids);free(fd);return 0;}
    char **leaves=xmalloc(n*sizeof(char*));size_t nl=0;
    for(size_t i=0;i<n;i++){bool child=false;for(size_t j=0;j<n;j++){if(i==j)continue;char *p=join_path(fd,ids[j]),*q=join_path(p,"FRAME.tsv");char parent[256];if(read_key_file(q,"parent_frame_id",parent,sizeof(parent))&&strcmp(parent,ids[i])==0)child=true;free(q);free(p);}
        if(!child)leaves[nl++]=ids[i];}
    int rc=0;if(nl==1){snprintf(out,256,"%s",leaves[0]);rc=1;}else if(nl>1)rc=-2;
    for(size_t i=0;i<n;i++)free(ids[i]);free(ids);free(leaves);free(fd);return rc;
}
static bool ensure_store(const char *store) {
    char *p=join_path(store,"STORE_FORMAT.txt");FILE *f=fopen(p,"rb");free(p);if(!f)return false;char line[256];bool ok=fgets(line,sizeof(line),f)!=NULL;fclose(f);if(ok){line[strcspn(line,"\r\n")]=0;ok=strcmp(line,STORE_MAGIC)==0;}return ok;
}
static void cmd_init(const char *store) {
    struct stat st;if(lstat(store,&st)==0)die("REFUSE: store path already exists; init never overwrites: %s",store);if(errno!=ENOENT)die("ERROR: cannot inspect store path: %s",strerror(errno));
    if(mkdir(store,0755)!=0)die("ERROR: cannot create store '%s': %s",store,strerror(errno));char *fr=join_path(store,"frames");if(mkdir(fr,0755)!=0)die("ERROR: cannot create frames directory: %s",strerror(errno));free(fr);char *p=join_path(store,"STORE_FORMAT.txt");write_printf_file(p,"%s\ncreated_by\t%s\ncanonical_status\tNON_CANONICAL_IMPLEMENTATION_CANDIDATE\n",STORE_MAGIC,VERSION);free(p);
    p=join_path(store,"GATEWAY.md");write_printf_file(p,"# K501 Memory Gateway v3\n\nStore schema: %s\nRole: orientation only; it does not assert truth or change canonical sources.\nUse `k501-memory status %s`, `latest %s`, `verify %s FRAME_ID`, and `query %s latest \"terms\"`.\n\nFrames are append-only directories. A repeated frame ID is refused. Hidden `.stage-*` directories are interrupted candidates and are never promoted automatically.\n",STORE_MAGIC,store,store,store,store);free(p);printf("INIT PASS\nstore=%s\nschema=%s\n",store,STORE_MAGIC);
}
static bool load_text_entries(const char *path,TextVec *v) {
    FILE *f=fopen(path,"rb");
    if(!f)return false;
    char *line=NULL;size_t cap=0;ssize_t n;bool magic=false,ok=true;
    while((n=getline(&line,&cap,f))!=-1){
        if(n&&line[n-1]=='\n')line[--n]='\0';
        if(n&&line[n-1]=='\r')line[--n]='\0';
        if(!magic){magic=strcmp(line,TEXT_MAGIC)==0;if(!magic)ok=false;continue;}
        if(!*line)continue;
        char *fields[10];size_t nf=split_tabs_preserve_empty(line,fields,10);
        if(nf!=8||strcmp(fields[0],"T")||!is_hex64(fields[1])||!is_hex64(fields[2])||!is_hex64(fields[3])){ok=false;break;}
        if(v->n==v->cap){v->cap=v->cap?v->cap*2:64;v->v=xrealloc(v->v,v->cap*sizeof(*v->v));}
        TextEntry *e=&v->v[v->n++];memset(e,0,sizeof(*e));
        strcpy(e->object_id,fields[1]);strcpy(e->content_sha,fields[2]);strcpy(e->text_sha,fields[3]);
        if(!copy_field(e->extractor,sizeof(e->extractor),fields[4])){ok=false;break;}
        size_t pn=0,tn=0;unsigned char *pb=base64_decode(fields[5],&pn);e->text=base64_decode(fields[7],&tn);
        if(!pb||!e->text||tn!=strtoull(fields[6],NULL,10)||memchr(pb,0,pn)){
            free(pb);free(e->text);e->text=NULL;ok=false;break;
        }
        e->path=xmalloc(pn+1);memcpy(e->path,pb,pn);e->path[pn]='\0';free(pb);
        e->text_len=tn;char th[65];
        if(!sha256_buf(e->text,e->text_len,th)||strcmp(th,e->text_sha)!=0){ok=false;break;}
    }
    if(ferror(f))ok=false;
    free(line);fclose(f);return ok&&magic;
}
static void show_help(void){puts("K501 AIONARC Pipeline v3 AI Memory Runtime C — candidate, not canonical\n\nUsage:\n  k501-memory help\n  k501-memory init STORE\n  k501-memory scan SOURCE STORE ROOT_ID [FRAME_ID] [PARENT_ID|none|latest]\n  k501-memory latest STORE\n  k501-memory status STORE\n  k501-memory verify STORE FRAME_ID\n  k501-memory verify-source STORE FRAME_ID SOURCE\n  k501-memory query STORE FRAME_ID|latest \"search terms\"\n  k501-memory trace STORE FRAME_ID|latest OBJECT_ID\n  k501-memory history STORE OBJECT_ID\n  k501-memory diff STORE FRAME_ID\n  k501-memory qh256-test\n  k501-memory sha256-test\n  k501-memory mapping-test\n\nNo llama.cpp client is included. Retrieval is exact lexical matching, not semantic search. See docs/OPERATOR_MANUAL.md.");}
static void cmd_latest(const char *store){if(!ensure_store(store))die("ERROR: not a K501 v3 memory store");char id[256];int r=leaf_frame(store,id);if(r==0){puts("LATEST\tNONE");return;}if(r==-1)die("ERROR: cannot read frames directory");if(r==-2)die("OPEN: multiple frame tips found; specify FRAME_ID explicitly; no automatic branch choice");char *p=frame_dir(store,id),*fp=join_path(p,"FRAME.tsv");FrameMeta m;ObjectVec v={0};if(!verify_frame(store,id,&m,&v)){free(p);free(fp);vec_free(&v);die("VERIFY FAIL: latest candidate frame did not verify");}printf("LATEST\t%s\nSEQUENCE\t%" PRIu64 "\nSTATE_SHA256\t%s\nOBJECTS\t%" PRIu64 "\n",m.frame_id,m.sequence,m.state_sha,m.object_count);vec_free(&v);free(p);free(fp);}
static void cmd_status(const char *store){if(!ensure_store(store))die("ERROR: not a K501 v3 memory store");char *fd=join_path(store,"frames");DIR *d=opendir(fd);if(!d)die("ERROR: cannot list frames");printf("K501_MEMORY_STATUS\t%s\n",VERSION);struct dirent *de;size_t n=0;while((de=readdir(d))){if(de->d_name[0]=='.'||!safe_id(de->d_name))continue;char *p=join_path(fd,de->d_name),*q=join_path(p,"FRAME.tsv");FrameMeta m;if(load_frame_meta(p,&m)){printf("FRAME\t%s\tSEQ=%" PRIu64 "\tOBJECTS=%" PRIu64 "\tSTATE=%s\tPARENT=%s\n",m.frame_id,m.sequence,m.object_count,m.state_sha,m.parent_frame_id[0]?m.parent_frame_id:"-");n++;}else printf("FRAME\t%s\tSTATUS=UNREADABLE\n",de->d_name);free(q);free(p);}closedir(d);free(fd);if(!n)puts("FRAMES\t0");char latest[256];int lr=leaf_frame(store,latest);if(lr==1)printf("UNIQUE_TIP\t%s\n",latest);else if(lr==0)puts("UNIQUE_TIP\tNONE");else puts("UNIQUE_TIP\tAMBIGUOUS_OR_UNREADABLE");}
static void cmd_verify(const char *store,const char *frame){if(!ensure_store(store))die("ERROR: not a K501 v3 memory store");FrameMeta m;ObjectVec v={0};if(!verify_frame(store,frame,&m,&v)){vec_free(&v);die("VERIFY FAIL: frame artifact/hash/record validation failed: %s",frame);}printf("VERIFY PASS\nframe_id=%s\nsequence=%" PRIu64 "\nobjects=%zu\nstate_sha256=%s\nchain_sha256=%s\n",m.frame_id,m.sequence,v.n,m.state_sha,m.chain_sha);vec_free(&v);}
static void cmd_scan(const char *source,const char *store,const char *root_id,const char *frame_in,const char *parent_in) {
    if(!ensure_store(store))die("ERROR: initialize a new memory store first with `init STORE`");if(!safe_id(root_id))die("ERROR: ROOT_ID must use ASCII letters/digits/dot/underscore/hyphen and start alphanumeric");
    char frame[256];if(frame_in&&*frame_in){if(!safe_id(frame_in))die("ERROR: invalid FRAME_ID");snprintf(frame,sizeof(frame),"%s",frame_in);}else{snprintf(frame,sizeof(frame),"e%" PRIdMAX,(intmax_t)time(NULL));}
    char *final=frame_dir(store,frame);struct stat st;if(lstat(final,&st)==0)die("REFUSE: frame ID already exists; append-only store never overwrites: %s",frame);if(errno!=ENOENT)die("ERROR: cannot check output frame path");
    char parent[256]="";if(parent_in&&strcmp(parent_in,"none")==0){char *fd=join_path(store,"frames");DIR *d=opendir(fd);if(!d){free(fd);die("ERROR: cannot open frames directory");}struct dirent *de;bool exists=false;while((de=readdir(d)))if(de->d_name[0]!='.'&&safe_id(de->d_name))exists=true;closedir(d);free(fd);if(exists)die("REFUSE: parent=none is allowed only for an empty store; select a parent frame or a separate store");}
    else {if(parent_in&&strcmp(parent_in,"latest")!=0)snprintf(parent,sizeof(parent),"%s",parent_in);else {char l[256];int r=leaf_frame(store,l);if(r==-2)die("OPEN: store has multiple tips; choose parent FRAME_ID explicitly");if(r==1)snprintf(parent,sizeof(parent),"%s",l);}}
    FrameMeta pm;ObjectVec old={0};char parent_sha[65]="-",parent_chain[65]="-";uint64_t seq=1;
    if(parent[0]){if(!verify_frame(store,parent,&pm,&old))die("REFUSE: predecessor frame failed validation: %s",parent);if(strcmp(pm.root_id,root_id)!=0)die("REFUSE: ROOT_ID differs from predecessor; use a separate store or explicit root migration contract");snprintf(parent_sha,sizeof(parent_sha),"%s",pm.state_sha);snprintf(parent_chain,sizeof(parent_chain),"%s",pm.chain_sha);seq=pm.sequence+1;}
    ObjectVec now={0};scan_tree(source,store,root_id,&now);size_t texttotal=0;char src[PATH_MAX];canonical_abs(source,src);extract_texts(&now,src,&texttotal);
    EventVec ev={0};derive_transition(parent[0]?&old:NULL,&now,&ev);
    FrameMeta m;memset(&m,0,sizeof(m));snprintf(m.frame_id,sizeof(m.frame_id),"%s",frame);snprintf(m.root_id,sizeof(m.root_id),"%s",root_id);if(parent[0]){snprintf(m.parent_frame_id,sizeof(m.parent_frame_id),"%s",parent);snprintf(m.parent_state_sha,sizeof(m.parent_state_sha),"%s",parent_sha);}m.sequence=seq;m.object_count=now.n;m.added=ev.added;m.removed=ev.removed;m.content_changed=ev.content_changed;m.metadata_changed=ev.metadata_changed;m.unchanged=ev.unchanged;
    char *frames=join_path(store,"frames");char stage_name[512];snprintf(stage_name,sizeof(stage_name),".stage-%s-%ld",frame,(long)getpid());char *stage=join_path(frames,stage_name);if(mkdir(stage,0700)!=0)die("ERROR: cannot create unique staging directory: %s",strerror(errno));
    char *statecand=join_path(stage,"STATE.candidate"),*textp=join_path(stage,"TEXT.tsv"),*indexp=join_path(stage,"INDEX.tsv"),*transp=join_path(stage,"TRANSITION.tsv"),*gatewayp=join_path(stage,"GATEWAY.tsv"),*qhp=join_path(stage,"QH256.hex"),*framep=join_path(stage,"FRAME.tsv"),*proofp=join_path(stage,"PROOF.tsv");
    write_state_candidate(statecand,&m,&now);write_text_capture(textp,&now);write_transition(transp,&ev,parent);if(!validate_transition_semantics(transp,parent[0]?parent:"-",parent[0]?&old:NULL,&now))die("PROOF BEFORE STATE FAIL: transition does not reconstruct from predecessor and candidate");write_gateway(gatewayp,&m,&now);if(!validate_gateway_file(gatewayp,&m,&now))die("PROOF BEFORE STATE FAIL: Gateway does not reconstruct from candidate scope");
    char state_sha[65],index_sha[65],text_sha[65],trans_sha[65],gate_sha[65],qh_file_sha[65],frame_sha[65],chain_sha[65],created[64];
    if(!sha256_file(statecand,state_sha))die("ERROR: cannot hash candidate state");
    if(!chain_hash_for(parent_chain,state_sha,chain_sha))die("ERROR: chain hash failed");
    if(!sha256_file(textp,text_sha)||!sha256_file(transp,trans_sha)||!sha256_file(gatewayp,gate_sha))die("ERROR: derived artifact hash failed");
    ObjectVec check={0};FrameMeta checkm;char state_rehash[65];
    if(!parse_state(statecand,frame,&check,&checkm)||!validate_object_hashes(root_id,&check)||checkm.object_count!=now.n||!sha256_file(statecand,state_rehash)||strcmp(state_rehash,state_sha)!=0)die("PROOF BEFORE STATE FAIL: candidate state did not reconstruct");
    if(check.n!=now.n)die("PROOF BEFORE STATE FAIL: candidate object count differs from observation");
    for(size_t i=0;i<now.n;i++)if(strcmp(check.v[i].path,now.v[i].path)||strcmp(check.v[i].object_id,now.v[i].object_id)||strcmp(check.v[i].record_hash,now.v[i].record_hash)||strcmp(check.v[i].content_sha,now.v[i].content_sha))die("PROOF BEFORE STATE FAIL: serialized candidate differs from observed object set");
    TextVec textcheck={0};if(!load_text_entries(textp,&textcheck))die("PROOF BEFORE STATE FAIL: TEXT.tsv did not round-trip");
    for(size_t i=0;i<textcheck.n;i++){int oi=find_id(&now,textcheck.v[i].object_id);if(oi<0||strcmp(now.v[oi].content_sha,textcheck.v[i].content_sha)||strcmp(now.v[oi].path,textcheck.v[i].path)||strcmp(now.v[oi].text_sha,textcheck.v[i].text_sha))die("PROOF BEFORE STATE FAIL: text capture does not bind to source record");}
    size_t expected_text=0;for(size_t i=0;i<now.n;i++)if(now.v[i].text)expected_text++;if(expected_text!=textcheck.n)die("PROOF BEFORE STATE FAIL: text capture count mismatch");
    for(size_t i=0;i<now.n;i++)if(now.v[i].text){char h[65];if(!sha256_buf(now.v[i].text,now.v[i].text_len,h)||strcmp(h,now.v[i].text_sha)!=0)die("PROOF BEFORE STATE FAIL: extracted text checksum mismatch");}
    if(!validate_path_base64_roundtrip(&check))die("PROOF BEFORE STATE FAIL: relative paths did not survive Base64 roundtrip");
    write_index(indexp,&now);
    if(!validate_index_file(indexp,&now))die("PROOF BEFORE STATE FAIL: derived index does not reconstruct from validated candidate objects");
    if(!sha256_file(indexp,index_sha)||!sha256_file(textp,text_sha)||!sha256_file(transp,trans_sha)||!sha256_file(gatewayp,gate_sha))die("PROOF BEFORE STATE FAIL: derived output hash missing");
    char bind[256];if(!read_key_file(gatewayp,"frame_id",bind,sizeof(bind))||strcmp(bind,frame)||!read_key_file(gatewayp,"root_id",bind,sizeof(bind))||strcmp(bind,root_id))die("PROOF BEFORE STATE FAIL: Gateway binding does not match frame");
    EventVec checked_transition={0};checked_transition.added=ev.added;checked_transition.removed=ev.removed;checked_transition.content_changed=ev.content_changed;checked_transition.metadata_changed=ev.metadata_changed;checked_transition.unchanged=ev.unchanged;checked_transition.n=ev.n;
    qh256_t qh=compute_qh(&now,&checked_transition,true,parent[0]!='\0',true,true,true,true);write_qh(qhp,&qh);if(!sha256_file(qhp,qh_file_sha))die("PROOF BEFORE STATE FAIL: QH256 hash failed");
    char qhcheckline[256];qh256_t qh_round;if(!read_key_file(qhp,"hex",qhcheckline,sizeof(qhcheckline))||!qh_from_hex(qhcheckline,&qh_round)||!qh256_equal(&qh,&qh_round))die("PROOF BEFORE STATE FAIL: QH256 serialization did not round-trip");
    format_utc(time(NULL),created);write_frame(framep,&m,created,state_sha,chain_sha,NULL);if(!sha256_file(framep,frame_sha))die("PROOF BEFORE STATE FAIL: frame digest missing");
    write_proof(proofp,&m,state_sha,chain_sha,frame_sha,index_sha,text_sha,trans_sha,gate_sha,qh_file_sha,NULL);
    /* Proof-before-state contract: PROOF.tsv is fsync'ed before candidate state becomes STATE.tsv. */
    char *statefinal=join_path(stage,"STATE.tsv");if(rename(statecand,statefinal)!=0)die("ERROR: cannot commit state after proof: %s",strerror(errno));
    /* Chain hash and state hash are now bound; file creation is exclusively inside the new staging frame. */
    write_manifest(stage);
    int dfd=open(stage,O_RDONLY|O_DIRECTORY);if(dfd>=0){fsync(dfd);close(dfd);}if(rename(stage,final)!=0)die("ERROR: frame package promotion failed; stage retained at %s: %s",stage,strerror(errno));
    int ffd=open(frames,O_RDONLY|O_DIRECTORY);if(ffd>=0){fsync(ffd);close(ffd);}size_t ntext=0;for(size_t i=0;i<now.n;i++)if(now.v[i].text)ntext++;
    printf("K501 MEMORY SCAN COMMIT PASS\nframe_id=%s\nsequence=%" PRIu64 "\nobjects=%zu\nsearchable_documents=%zu\ntext_bytes=%zu\nadded=%" PRIu64 "\nremoved=%" PRIu64 "\ncontent_changed=%" PRIu64 "\nmetadata_changed=%" PRIu64 "\nunchanged=%" PRIu64 "\nstate_sha256=%s\nchain_sha256=%s\nproof_before_state=PASS\nframe_path=%s\n",frame,seq,now.n,ntext,texttotal,ev.added,ev.removed,ev.content_changed,ev.metadata_changed,ev.unchanged,state_sha,chain_sha,final);
    texts_free(&textcheck);vec_free(&check);vec_free(&now);vec_free(&old);events_free(&ev);free(frames);free(stage);free(final);free(statecand);free(textp);free(indexp);free(transp);free(gatewayp);free(qhp);free(framep);free(proofp);free(statefinal);
}
static char *resolve_frame_arg(const char *store,const char *arg) { if(strcmp(arg,"latest")!=0)return xstrdup(arg);char id[256];int r=leaf_frame(store,id);if(r==0)die("ERROR: store has no frame");if(r==-2)die("OPEN: multiple frame tips; specify exact frame ID");if(r<0)die("ERROR: cannot resolve latest frame");return xstrdup(id); }
static void cmd_query(const char *store,const char *frame_arg,const char *query) {
    char *frame=resolve_frame_arg(store,frame_arg);FrameMeta m;ObjectVec os={0};if(!verify_frame(store,frame,&m,&os))die("VERIFY FAIL: query refused because frame failed validation");char *tp=frame_file(store,frame,"TEXT.tsv");TextVec tv={0};if(!load_text_entries(tp,&tv))die("ERROR: TEXT.tsv cannot be parsed");free(tp);
    char *q=xstrdup(query);char *terms[64];size_t nt=0;char *p=q;while(*p&&nt<64){while(*p&&!((*p>='A'&&*p<='Z')||(*p>='a'&&*p<='z')||(*p>='0'&&*p<='9')||*p=='_'||*p=='-'))p++;if(!*p)break;char *s=p;while(*p&&((*p>='A'&&*p<='Z')||(*p>='a'&&*p<='z')||(*p>='0'&&*p<='9')||*p=='_'||*p=='-')){if(*p>='A'&&*p<='Z')*p=(char)(*p+32);p++;}if(*p)*p++='\0';if(strlen(s)>=2)terms[nt++]=s;}
    if(nt==0)die("ERROR: query contains no searchable tokens");
    typedef struct{size_t ix;unsigned score;} Hit;Hit *hits=xmalloc((tv.n?tv.n:1)*sizeof(Hit));size_t nh=0;
    for(size_t i=0;i<tv.n;i++){char *hay=xmalloc(tv.v[i].text_len+1);memcpy(hay,tv.v[i].text,tv.v[i].text_len);hay[tv.v[i].text_len]=0;for(size_t z=0;z<tv.v[i].text_len;z++)if(hay[z]>='A'&&hay[z]<='Z')hay[z]=(char)(hay[z]+32);unsigned score=0;bool all=true;for(size_t t=0;t<nt;t++){unsigned cnt=0;char *at=hay;while((at=strstr(at,terms[t]))!=NULL){cnt++;at+=strlen(terms[t]);}if(cnt==0){all=false;break;}score+=cnt;}free(hay);if(all){hits[nh].ix=i;hits[nh].score=score;nh++;}}
    /* Stable selection ordering: higher exact term count first, then path byte order. */
    for(size_t i=0;i<nh;i++)for(size_t j=i+1;j<nh;j++)if(hits[j].score>hits[i].score||(hits[j].score==hits[i].score&&strcmp(tv.v[hits[j].ix].path,tv.v[hits[i].ix].path)<0)){Hit h=hits[i];hits[i]=hits[j];hits[j]=h;}
    printf("QUERY\t%s\nMODE\tEXACT_LEXICAL_AND\nTERMS\t%zu\nRESULTS\t%zu\n",query,nt,nh);
    for(size_t i=0;i<nh;i++){TextEntry *e=&tv.v[hits[i].ix];char *p64=NULL;base64_encode((const unsigned char*)e->path,strlen(e->path),&p64);printf("HIT\t%zu\tSCORE=%u\tOBJECT_ID=%s\tCONTENT_SHA256=%s\tTEXT_SHA256=%s\tPATH=",i+1,hits[i].score,e->object_id,e->content_sha,e->text_sha);print_escaped_path(e->path);printf("\tPATH_B64=%s\n",p64);free(p64);
        /* One-line, bounded excerpt centred at first token match. */
        char *hay=xmalloc(e->text_len+1);memcpy(hay,e->text,e->text_len);hay[e->text_len]=0;for(size_t z=0;z<e->text_len;z++)if(hay[z]>='A'&&hay[z]<='Z')hay[z]=(char)(hay[z]+32);char *at=strstr(hay,terms[0]);size_t pos=at?(size_t)(at-hay):0,start=pos>80?pos-80:0,end=pos+200<e->text_len?pos+200:e->text_len;printf("EXCERPT\t");for(size_t z=start;z<end;z++){unsigned char c=e->text[z];putchar((c=='\n'||c=='\r'||c=='\t')?' ':((c<32)?' ':c));}putchar('\n');free(hay);
    }
    free(hits);free(q);texts_free(&tv);vec_free(&os);free(frame);
}
static void cmd_trace(const char *store,const char *frame_arg,const char *id) {
    char *frame=resolve_frame_arg(store,frame_arg);FrameMeta m;ObjectVec v={0};if(!verify_frame(store,frame,&m,&v))die("VERIFY FAIL: trace refused");int i=find_id(&v,id);if(i<0)die("NOT FOUND: object_id not present in frame");Object *o=&v.v[i];char *p64=NULL;base64_encode((const unsigned char*)o->path,strlen(o->path),&p64);printf("TRACE\nframe_id=%s\nsequence=%" PRIu64 "\nobject_id=%s\nrecord_hash=%s\ncontent_sha256=%s\nobject_type=%s\nrelative_path=",frame,m.sequence,o->object_id,o->record_hash,o->content_sha,o->type);print_escaped_path(o->path);printf("\nrelative_path_b64=%s\nmode=%04o\nsize=%" PRIu64 "\nmtime_sec=%" PRId64 "\nmtime_nsec=%u\n",p64,o->mode,o->size,o->mtime_sec,o->mtime_nsec);free(p64);
    char *tp=frame_file(store,frame,"TEXT.tsv");TextVec tv={0};if(load_text_entries(tp,&tv)){for(size_t j=0;j<tv.n;j++)if(strcmp(tv.v[j].object_id,id)==0)printf("text_sha256=%s\nextractor=%s\ntext_bytes=%zu\n",tv.v[j].text_sha,tv.v[j].extractor,tv.v[j].text_len);}free(tp);texts_free(&tv);vec_free(&v);free(frame);
}
static void cmd_history(const char *store,const char *id) {
    if(!ensure_store(store))die("ERROR: not a store");if(!is_hex64(id))die("ERROR: OBJECT_ID must be 64 lowercase hex characters");char *fd=join_path(store,"frames");DIR *d=opendir(fd);if(!d)die("ERROR: cannot list frames");typedef struct{FrameMeta m;char *id;char object_sha[65],record[65],*path;} Hist;Hist *hs=NULL;size_t n=0,cap=0;struct dirent *de;
    while((de=readdir(d))){if(de->d_name[0]=='.'||!safe_id(de->d_name))continue;ObjectVec v={0};FrameMeta sm;if(!verify_frame(store,de->d_name,&sm,&v)){vec_free(&v);closedir(d);free(fd);die("VERIFY FAIL: history refused because stored frame is invalid: %s",de->d_name);}int i=find_id(&v,id);if(i>=0){if(n==cap){cap=cap?cap*2:8;hs=xrealloc(hs,cap*sizeof(*hs));}Hist *h=&hs[n++];memset(h,0,sizeof(*h));h->m=sm;h->id=xstrdup(de->d_name);strcpy(h->object_sha,v.v[i].content_sha);strcpy(h->record,v.v[i].record_hash);h->path=xstrdup(v.v[i].path);}vec_free(&v);}closedir(d);free(fd);
    for(size_t i=0;i<n;i++)for(size_t j=i+1;j<n;j++)if(hs[j].m.sequence<hs[i].m.sequence){Hist h=hs[i];hs[i]=hs[j];hs[j]=h;}
    printf("OBJECT_HISTORY\t%s\nFRAMES\t%zu\n",id,n);for(size_t i=0;i<n;i++){char *b=NULL;base64_encode((unsigned char*)hs[i].path,strlen(hs[i].path),&b);printf("FRAME\t%s\tSEQ=%" PRIu64 "\tCONTENT_SHA256=%s\tRECORD_HASH=%s\tPATH=",hs[i].id,hs[i].m.sequence,hs[i].object_sha,hs[i].record);print_escaped_path(hs[i].path);printf("\tPATH_B64=%s\n",b);free(b);}for(size_t i=0;i<n;i++){free(hs[i].id);free(hs[i].path);}free(hs);
}
static void cmd_diff(const char *store,const char *frame) {
    FrameMeta m;ObjectVec v={0};if(!verify_frame(store,frame,&m,&v)){vec_free(&v);die("VERIFY FAIL: diff refused because frame failed validation: %s",frame);}vec_free(&v);
    char *p=frame_file(store,frame,"TRANSITION.tsv");FILE *f=fopen(p,"rb");if(!f)die("ERROR: cannot read transition file");char *line=NULL;size_t cap=0;ssize_t n;while((n=getline(&line,&cap,f))!=-1){fwrite(line,1,(size_t)n,stdout);}free(line);fclose(f);free(p);
}
static void cmd_verify_source(const char *store,const char *frame,const char *source) {
    FrameMeta m;ObjectVec baseline={0};if(!verify_frame(store,frame,&m,&baseline))die("VERIFY FAIL: stored frame cannot be validated");ObjectVec current={0};scan_tree(source,store,m.root_id,&current);if(current.n!=baseline.n){vec_free(&baseline);vec_free(&current);die("SOURCE DRIFT: object count differs");}
    for(size_t i=0;i<current.n;i++){if(strcmp(current.v[i].path,baseline.v[i].path)||strcmp(current.v[i].object_id,baseline.v[i].object_id)||strcmp(current.v[i].record_hash,baseline.v[i].record_hash)||strcmp(current.v[i].content_sha,baseline.v[i].content_sha)){fprintf(stderr,"SOURCE DRIFT at relative path '%s'\n",current.v[i].path);vec_free(&baseline);vec_free(&current);exit(3);}}
    printf("VERIFY SOURCE PASS\nframe_id=%s\nobjects=%zu\nsource_matches_snapshot=yes\n",frame,current.n);vec_free(&baseline);vec_free(&current);
}
int main(int argc,char **argv) {
    if(argc<2){show_help();return 0;}const char *cmd=argv[1];
    if(strcmp(cmd,"help")==0||strcmp(cmd,"--help")==0||strcmp(cmd,"-h")==0){show_help();return 0;}
    if(strcmp(cmd,"qh256-test")==0){if(qh_tests()){puts("QH256 CORE TEST PASS\nchecked: UNKNOWN/FALSE/TRUE/GUARD representation and MERGE_K TRUE+FALSE=GUARD");return 0;}puts("QH256 CORE TEST FAIL");return 1;}
    if(strcmp(cmd,"sha256-test")==0){if(sha256_tests()){puts("SHA256 TEST PASS\nchecked: empty string, abc known vector, incremental update");return 0;}puts("SHA256 TEST FAIL");return 1;}
    if(strcmp(cmd,"mapping-test")==0){if(mapping_profile_tests()){puts("QH256 MAPPING PROFILE V3A1 TEST PASS\nchecked: cells 21, 22, 23, 25 across capture and failure statuses");return 0;}puts("QH256 MAPPING PROFILE V3A1 TEST FAIL");return 1;}
    if(strcmp(cmd,"init")==0){if(argc!=3){show_help();return 2;}cmd_init(argv[2]);return 0;}
    if(strcmp(cmd,"latest")==0){if(argc!=3){show_help();return 2;}cmd_latest(argv[2]);return 0;}
    if(strcmp(cmd,"status")==0){if(argc!=3){show_help();return 2;}cmd_status(argv[2]);return 0;}
    if(strcmp(cmd,"verify")==0){if(argc!=4){show_help();return 2;}cmd_verify(argv[2],argv[3]);return 0;}
    if(strcmp(cmd,"verify-source")==0){if(argc!=5){show_help();return 2;}cmd_verify_source(argv[2],argv[3],argv[4]);return 0;}
    if(strcmp(cmd,"query")==0){if(argc!=5){show_help();return 2;}cmd_query(argv[2],argv[3],argv[4]);return 0;}
    if(strcmp(cmd,"trace")==0){if(argc!=5){show_help();return 2;}cmd_trace(argv[2],argv[3],argv[4]);return 0;}
    if(strcmp(cmd,"history")==0){if(argc!=4){show_help();return 2;}cmd_history(argv[2],argv[3]);return 0;}
    if(strcmp(cmd,"diff")==0){if(argc!=4){show_help();return 2;}cmd_diff(argv[2],argv[3]);return 0;}
    if(strcmp(cmd,"scan")==0){if(argc<5||argc>7){show_help();return 2;}const char *fid=argc>=6?argv[5]:NULL;const char *par=argc>=7?argv[6]:"latest";cmd_scan(argv[2],argv[3],argv[4],fid,par);return 0;}
    fprintf(stderr,"ERROR: unknown command '%s'\n",cmd);show_help();return 2;
}
