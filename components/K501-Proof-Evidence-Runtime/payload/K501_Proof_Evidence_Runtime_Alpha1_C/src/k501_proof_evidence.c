#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include "qh256.h"
#include "sha256.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define K501_VERSION "0.1.0-alpha.1"
#define K501_NAME "K501 Proof and Evidence Runtime Alpha 1"
#define RUN_PROTOCOL "K501-CONTROLLED-RUN-0.1"
#define EVIDENCE_SCHEMA_VERSION "0.1.0"
#define MAX_CAPTURE_BYTES (16u * 1024u * 1024u)
#define READ_BUFFER_SIZE 65536u

static const char *const ALLOWED_AREAS[] = {"system", "pipeline", "eArc", "QH256", "AIONARC_CHANNEL"};
static const size_t ALLOWED_AREA_COUNT = sizeof(ALLOWED_AREAS) / sizeof(ALLOWED_AREAS[0]);
static const char *const EVIDENCE_FILES[] = {
    "COMMAND.txt", "EVIDENCE_RECORD.json", "FRAME.json", "GIT_POST.txt",
    "GIT_PRE.txt", "HOST.txt", "RESULT.txt", "STATUS.txt", "TRANSCRIPT.log"
};

struct buffer { char *data; size_t len; size_t cap; bool overflow; };

static void buffer_free(struct buffer *b) { free(b->data); b->data = NULL; b->len = 0u; b->cap = 0u; b->overflow = false; }

static bool buffer_append(struct buffer *b, const char *p, size_t n) {
    if (n > MAX_CAPTURE_BYTES || b->len > MAX_CAPTURE_BYTES - n) { b->overflow = true; return false; }
    if (b->len + n + 1u > b->cap) {
        size_t cap = b->cap == 0u ? 1024u : b->cap;
        while (cap < b->len + n + 1u) {
            if (cap > MAX_CAPTURE_BYTES / 2u) { cap = MAX_CAPTURE_BYTES + 1u; break; }
            cap *= 2u;
        }
        char *q = realloc(b->data, cap);
        if (q == NULL) return false;
        b->data = q; b->cap = cap;
    }
    memcpy(b->data + b->len, p, n); b->len += n; b->data[b->len] = '\0';
    return true;
}

static bool write_all(int fd, const void *data, size_t len) {
    const unsigned char *p = (const unsigned char *)data;
    while (len > 0u) {
        ssize_t n = write(fd, p, len);
        if (n < 0) { if (errno == EINTR) continue; return false; }
        p += (size_t)n; len -= (size_t)n;
    }
    return true;
}

static bool write_exclusive_text(const char *path, const char *text) {
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) return false;
    size_t n = strlen(text); bool ok = write_all(fd, text, n);
    if (close(fd) != 0) ok = false;
    return ok;
}

static bool path_join(char *out, size_t size, const char *a, const char *b) {
    int n = snprintf(out, size, "%s/%s", a, b);
    return n >= 0 && (size_t)n < size;
}

static bool is_dir(const char *p) { struct stat st; return stat(p, &st) == 0 && S_ISDIR(st.st_mode); }

static bool mkdir_one(const char *p) {
    if (mkdir(p, 0700) == 0) return true;
    if (errno == EEXIST && is_dir(p)) return true;
    return false;
}

static bool area_valid(const char *a) {
    for (size_t i = 0; i < ALLOWED_AREA_COUNT; ++i) if (strcmp(a, ALLOWED_AREAS[i]) == 0) return true;
    return false;
}

static bool slug_valid(const char *s) {
    size_t n = strlen(s);
    if (n < 1u || n > 64u || !isalnum((unsigned char)s[0])) return false;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}

static bool json_string_valid_utf8(const unsigned char *s) {
    size_t i = 0u;
    while (s[i] != 0u) {
        unsigned char c = s[i];
        if (c < 0x80u) { ++i; continue; }
        size_t need; uint32_t cp;
        if ((c & 0xe0u) == 0xc0u) { need = 1u; cp = (uint32_t)(c & 0x1fu); if (cp < 2u) return false; }
        else if ((c & 0xf0u) == 0xe0u) { need = 2u; cp = (uint32_t)(c & 0x0fu); }
        else if ((c & 0xf8u) == 0xf0u) { need = 3u; cp = (uint32_t)(c & 0x07u); }
        else return false;
        for (size_t j = 1u; j <= need; ++j) {
            unsigned char d = s[i + j];
            if (d == 0u || (d & 0xc0u) != 0x80u) return false;
            cp = (cp << 6u) | (uint32_t)(d & 0x3fu);
        }
        if ((need == 2u && cp < 0x800u) || (need == 3u && cp < 0x10000u) ||
            cp > 0x10ffffu || (cp >= 0xd800u && cp <= 0xdfffu)) return false;
        i += need + 1u;
    }
    return true;
}

static void json_write_string(FILE *f, const char *s) {
    fputc('"', f);
    for (const unsigned char *p = (const unsigned char *)s; *p != 0u; ++p) {
        switch (*p) {
            case '"': fputs("\\\"", f); break;
            case '\\': fputs("\\\\", f); break;
            case '\b': fputs("\\b", f); break;
            case '\f': fputs("\\f", f); break;
            case '\n': fputs("\\n", f); break;
            case '\r': fputs("\\r", f); break;
            case '\t': fputs("\\t", f); break;
            default:
                if (*p < 0x20u) fprintf(f, "\\u%04x", (unsigned)*p);
                else fputc((int)*p, f);
        }
    }
    fputc('"', f);
}

static bool format_utc(time_t t, char *out, size_t n, bool compact) {
    struct tm tmv;
    if (gmtime_r(&t, &tmv) == NULL) return false;
    return strftime(out, n, compact ? "%Y%m%dT%H%M%SZ" : "%Y-%m-%dT%H:%M:%SZ", &tmv) > 0u;
}

static bool format_berlin(time_t t, char *out, size_t n) {
    const char *old = getenv("TZ"); char *saved = old == NULL ? NULL : strdup(old);
    if (setenv("TZ", "Europe/Berlin", 1) != 0) { free(saved); return false; }
    tzset(); struct tm tmv; bool ok = localtime_r(&t, &tmv) != NULL;
    char raw[64] = "";
    if (ok) ok = strftime(raw, sizeof(raw), "%Y-%m-%dT%H:%M:%S%z", &tmv) > 0u;
    if (saved != NULL) { (void)setenv("TZ", saved, 1); free(saved); } else (void)unsetenv("TZ");
    tzset();
    if (!ok) return false;
    size_t len = strlen(raw);
    if (len >= 5u && (raw[len-5u] == '+' || raw[len-5u] == '-')) {
        int z = snprintf(out, n, "%.*s%c%c%c:%c%c", (int)(len-5u), raw, raw[len-5u], raw[len-4u], raw[len-3u], raw[len-2u], raw[len-1u]);
        return z >= 0 && (size_t)z < n;
    }
    int z = snprintf(out, n, "%s", raw);
    return z >= 0 && (size_t)z < n;
}

static bool sha256_file(const char *path, char hex[65], uint64_t *size_out) {
    FILE *f = fopen(path, "rb"); if (f == NULL) return false;
    sha256_ctx ctx; sha256_init(&ctx); uint8_t buf[READ_BUFFER_SIZE]; uint64_t total = 0u; bool ok = true;
    for (;;) {
        size_t n = fread(buf, 1u, sizeof(buf), f);
        if (n > 0u) { sha256_update(&ctx, buf, n); total += (uint64_t)n; }
        if (n < sizeof(buf)) { if (ferror(f)) ok = false; break; }
    }
    if (fclose(f) != 0) ok = false;
    if (!ok) return false;
    uint8_t digest[32]; sha256_final(&ctx, digest);
    static const char h[] = "0123456789abcdef";
    for (size_t i = 0; i < 32u; ++i) { hex[i*2u] = h[digest[i] >> 4u]; hex[i*2u+1u] = h[digest[i] & 15u]; }
    hex[64] = '\0'; if (size_out != NULL) *size_out = total; return true;
}

static bool sha256_text(const char *s, char hex[65]) {
    uint8_t digest[32]; sha256_bytes((const uint8_t *)s, strlen(s), digest);
    static const char h[] = "0123456789abcdef";
    for (size_t i=0u;i<32u;++i) { hex[i*2u]=h[digest[i]>>4u]; hex[i*2u+1u]=h[digest[i]&15u]; }
    hex[64]='\0'; return true;
}

static bool file_digest_or_null(const char *dir, const char *name, char hex[65], uint64_t *size) {
    char p[PATH_MAX]; return path_join(p, sizeof(p), dir, name) && sha256_file(p, hex, size);
}

static void print_usage(FILE *f) {
    fputs("K501 Proof and Evidence Runtime Alpha 1 (C11)\n\n"
          "Usage:\n"
          "  k501-proof-evidence version\n"
          "  k501-proof-evidence self-test\n"
          "  k501-proof-evidence identity <file>\n"
          "  k501-proof-evidence run <workspace-root> <area> <slug> [--expect-exit N] -- <command> [arg ...]\n\n"
          "Areas: system | pipeline | eArc | QH256 | AIONARC_CHANNEL\n"
          "Default expected exit code: 0. The actual child exit code is returned.\n"
          "No shell is inserted: command and arguments are passed to execvp as argv.\n"
          "Do not record secrets, passwords, access tokens or private keys in argv.\n", f);
}

static int qh256_self_test(void) {
    qh256_t q = qh256_unknown(), a = qh256_unknown(), b = qh256_unknown(), m;
    qh_state_t state; uint8_t bytes[QH256_BYTES]; qh256_t recovered;
    if (sizeof(q) != QH256_BYTES || qh256_size_bytes() != QH256_BYTES) return 1;
    if (!qh256_set_cell(&q, 0u, QH_UNKNOWN) || !qh256_set_cell(&q, 1u, QH_FALSE) ||
        !qh256_set_cell(&q, 2u, QH_TRUE) || !qh256_set_cell(&q, 3u, QH_GUARD)) return 1;
    if (!qh256_get_cell(&q, 0u, &state) || state != QH_UNKNOWN) return 1;
    if (!qh256_get_cell(&q, 1u, &state) || state != QH_FALSE) return 1;
    if (!qh256_get_cell(&q, 2u, &state) || state != QH_TRUE) return 1;
    if (!qh256_get_cell(&q, 3u, &state) || state != QH_GUARD) return 1;
    if (!qh256_set_cell(&a, 7u, QH_TRUE) || !qh256_set_cell(&b, 7u, QH_FALSE)) return 1;
    m = qh256_merge(a, b);
    if (!qh256_get_cell(&m, 7u, &state) || state != QH_GUARD) return 1;
    qh256_t aa = qh256_merge(a, a);
    if (!qh256_equal(&aa, &a)) return 1;
    if (!qh256_serialize(&q, bytes) || !qh256_deserialize(&recovered, bytes) || !qh256_equal(&q, &recovered)) return 1;
    if (qh256_set_cell(&q, QH256_CELLS, QH_TRUE)) return 1;
    return 0;
}

static int sha256_self_test(void) {
    char hex[65];
    sha256_text("", hex);
    if (strcmp(hex, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855") != 0) return 1;
    sha256_text("abc", hex);
    if (strcmp(hex, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") != 0) return 1;
    return 0;
}

static int capture_argv(char *const argv[], struct buffer *out) {
    int fds[2]; if (pipe(fds) != 0) return -1;
    pid_t pid = fork();
    if (pid < 0) { close(fds[0]); close(fds[1]); return -1; }
    if (pid == 0) {
        close(fds[0]); (void)dup2(fds[1], STDOUT_FILENO); (void)dup2(fds[1], STDERR_FILENO); close(fds[1]);
        execvp(argv[0], argv); _exit(errno == ENOENT ? 127 : 126);
    }
    close(fds[1]); char buf[4096]; int status = 0;
    for (;;) {
        ssize_t n = read(fds[0], buf, sizeof(buf));
        if (n < 0) { if (errno == EINTR) continue; status = -1; break; }
        if (n == 0) break;
        if (!buffer_append(out, buf, (size_t)n)) { status = -1; break; }
    }
    close(fds[0]);
    int ws = 0; if (waitpid(pid, &ws, 0) < 0) return -1;
    if (status < 0) return -1;
    if (WIFEXITED(ws)) return WEXITSTATUS(ws);
    if (WIFSIGNALED(ws)) return 128 + WTERMSIG(ws);
    return 125;
}

static char *trim_trailing_newlines(char *s) {
    if (s == NULL) return NULL;
    size_t n = strlen(s); while (n > 0u && (s[n-1u] == '\n' || s[n-1u] == '\r')) s[--n] = '\0'; return s;
}

static char *resolve_executable(const char *argv0) {
    char temp[PATH_MAX];
    if (strchr(argv0, '/') != NULL) {
        if (realpath(argv0, temp) != NULL) return strdup(temp);
        return strdup(argv0);
    }
    const char *path = getenv("PATH"); if (path == NULL) path = "/usr/bin:/bin";
    char *copy = strdup(path); if (copy == NULL) return NULL;
    char *save = NULL;
    for (char *part = strtok_r(copy, ":", &save); part != NULL; part = strtok_r(NULL, ":", &save)) {
        const char *base = part[0] == '\0' ? "." : part; int n = snprintf(temp, sizeof(temp), "%s/%s", base, argv0);
        char resolved_path[PATH_MAX];
        if (n >= 0 && (size_t)n < sizeof(temp) && access(temp, X_OK) == 0 && realpath(temp, resolved_path) != NULL) {
            char *resolved = strdup(resolved_path); free(copy); return resolved;
        }
    }
    free(copy); return strdup(argv0);
}

static bool append_buffer_to_file(FILE *f, const struct buffer *b) {
    if (b->len > 0u && fwrite(b->data, 1u, b->len, f) != b->len) return false;
    return true;
}

static bool write_host_file(const char *path, const char *root, const char *cwd, const char *host, const char *user, time_t start, const char *utc, const char *berlin) {
    struct utsname un; bool have_uname = uname(&un) == 0;
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600); if (fd < 0) return false;
    FILE *f = fdopen(fd, "w"); if (f == NULL) { close(fd); return false; }
    fprintf(f, "HOST=%s\nUSER=%s\nPWD=%s\nWORKSPACE_ROOT=%s\n", host, user, cwd, root);
    fprintf(f, "RECORDER=K501-Proof-Evidence-Runtime-Alpha1/%s\nQH256_CORE=2.0\nEVIDENCE_PROTOCOL=0.1.0-proposal\n", K501_VERSION);
    fprintf(f, "DATE_UTC=%s\nDATE_EUROPE_BERLIN=%s\nSTART_EPOCH_UNIX=%lld\n", utc, berlin, (long long)start);
    if (have_uname) fprintf(f, "UNAME_SYS=%s\nUNAME_RELEASE=%s\nUNAME_MACHINE=%s\n", un.sysname, un.release, un.machine);
    else fputs("UNAME=UNAVAILABLE\n", f);
    fprintf(f, "C_STANDARD=C11\nPID=%ld\n", (long)getpid());
    bool ok = ferror(f) == 0; if (fclose(f) != 0) ok = false; return ok;
}

static bool write_git_snapshot(const char *path, const char *cwd, const char *utc) {
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600); if (fd < 0) return false;
    FILE *f = fdopen(fd, "w"); if (f == NULL) { close(fd); return false; }
    fprintf(f, "captured_utc=%s\n", utc);
    char *rev[] = {"git", "-C", (char *)cwd, "rev-parse", "--show-toplevel", NULL};
    struct buffer b = {0}; int rc = capture_argv(rev, &b);
    if (rc != 0 || b.data == NULL) {
        fputs("git_status=NOT_IN_GIT_WORKTREE\n", f);
        buffer_free(&b);
    } else {
        trim_trailing_newlines(b.data);
        fprintf(f, "git_root=%s\n", b.data); buffer_free(&b);
        char *head[] = {"git", "-C", (char *)cwd, "rev-parse", "HEAD", NULL};
        char *branch[] = {"git", "-C", (char *)cwd, "branch", "--show-current", NULL};
        char *status[] = {"git", "-C", (char *)cwd, "status", "--porcelain=v2", "--branch", NULL};
        char *diff[] = {"git", "-C", (char *)cwd, "diff", "--stat", NULL};
        char *cached[] = {"git", "-C", (char *)cwd, "diff", "--cached", "--stat", NULL};
        struct { const char *label; char **argv; } cmds[] = {
            {"git_head",head},{"git_branch",branch},{"\n--- status --porcelain=v2 --branch ---\n",status},
            {"\n--- diff --stat ---\n",diff},{"\n--- cached diff --stat ---\n",cached}
        };
        for (size_t i=0u;i<sizeof(cmds)/sizeof(cmds[0]);++i) {
            if (cmds[i].label[0] != '\n') fprintf(f, "%s=", cmds[i].label); else fputs(cmds[i].label, f);
            struct buffer o = {0}; int cr = capture_argv(cmds[i].argv, &o);
            if (o.data != NULL) append_buffer_to_file(f, &o);
            if (o.len == 0u || o.data == NULL || o.data[o.len-1u] != '\n') fputc('\n', f);
            if (cr != 0) fprintf(f, "command_exit=%d\n", cr);
            buffer_free(&o);
        }
    }
    bool ok = ferror(f) == 0; if (fclose(f) != 0) ok = false; return ok;
}

static bool write_frame(const char *path, const char *run_id, const char *area, const char *slug,
                        const char *root, const char *cwd, const char *host, time_t start,
                        const char *utc, const char *berlin, char *const cmdv[], int cmdc, int expected) {
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600); if (fd < 0) return false;
    FILE *f = fdopen(fd, "w"); if (f == NULL) { close(fd); return false; }
    fputs("{\n  \"frame_id\": ", f); json_write_string(f, run_id);
    fputs(",\n  \"protocol\": ", f); json_write_string(f, RUN_PROTOCOL);
    fputs(",\n  \"recorder_id\": ", f); json_write_string(f, "K501-Proof-Evidence-Runtime-Alpha1/0.1.0");
    fputs(",\n  \"status_at_creation\": \"RUNNING\",\n  \"area\": ", f); json_write_string(f, area);
    fputs(",\n  \"slug\": ", f); json_write_string(f, slug);
    fputs(",\n  \"workspace_root\": ", f); json_write_string(f, root);
    fputs(",\n  \"working_directory\": ", f); json_write_string(f, cwd);
    fputs(",\n  \"host\": ", f); json_write_string(f, host);
    fputs(",\n  \"time_source\": \"host_system_clock\",\n  \"start_epoch_unix\": ", f); fprintf(f, "%lld", (long long)start);
    fputs(",\n  \"start_utc\": ", f); json_write_string(f, utc);
    fputs(",\n  \"start_europe_berlin\": ", f); json_write_string(f, berlin);
    fprintf(f, ",\n  \"expected_exit_code\": %d,\n  \"command_argv\": [", expected);
    for (int i=0; i<cmdc; ++i) { if (i != 0) fputs(", ", f); json_write_string(f, cmdv[i]); }
    fputs("]\n}\n", f);
    bool ok = ferror(f) == 0; if (fclose(f) != 0) ok = false; return ok;
}

static bool write_command_file(const char *path, const char *run_id, const char *area, const char *slug, char *const cmdv[], int cmdc, int expected) {
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600); if (fd < 0) return false;
    FILE *f = fdopen(fd, "w"); if (f == NULL) { close(fd); return false; }
    fprintf(f, "frame_id=%s\narea=%s\nslug=%s\nexpected_exit_code=%d\nargv_count=%d\n", run_id, area, slug, expected, cmdc);
    for (int i=0; i<cmdc; ++i) { fprintf(f, "argv[%d]=", i); json_write_string(f, cmdv[i]); fputc('\n', f); }
    bool ok = ferror(f) == 0; if (fclose(f) != 0) ok = false; return ok;
}

static bool path_digest_record(const char *run_dir, const char *name, char hex[65], uint64_t *size) {
    return file_digest_or_null(run_dir, name, hex, size);
}

static bool write_evidence_record(const char *path, const char *run_id, const char *area, const char *slug,
                                  const char *cwd, const char *utc_start, const char *utc_end,
                                  time_t start, time_t end, char *const cmdv[], int cmdc, int expected,
                                  int actual, const char *subject, const char *subject_sha, uint64_t subject_size,
                                  bool subject_hash_ok, bool subject_hash_stable, bool capture_ok, const char *run_dir) {
    char command_sha[65], transcript_sha[65], result_sha[65]; uint64_t command_size=0u, transcript_size=0u, result_size=0u;
    if (!path_digest_record(run_dir, "COMMAND.txt", command_sha, &command_size) ||
        !path_digest_record(run_dir, "TRANSCRIPT.log", transcript_sha, &transcript_size) ||
        !path_digest_record(run_dir, "RESULT.txt", result_sha, &result_size)) return false;
    bool match = expected == actual;
    const char *execution_status = !match ? "FAIL" : (capture_ok ? "PASS" : "PARTIAL");
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600); if (fd < 0) return false;
    FILE *f = fdopen(fd, "w"); if (f == NULL) { close(fd); return false; }
    fputs("{\n  \"schema_version\": ", f); json_write_string(f, EVIDENCE_SCHEMA_VERSION);
    fputs(",\n  \"evidence_id\": ", f); char evid[512]; (void)snprintf(evid,sizeof(evid),"EVIDENCE_%s",run_id); json_write_string(f,evid);
    fputs(",\n  \"run_id\": ", f); json_write_string(f, run_id);
    fprintf(f, ",\n  \"related_run_ids\": [],\n  \"area\": "); json_write_string(f, area);
    fputs(",\n  \"scope\": ", f); char scope[256]; (void)snprintf(scope,sizeof(scope),"Controlled process execution for slug %s; process evidence only, not project gate acceptance.",slug); json_write_string(f, scope);
    fputs(",\n  \"gate_id\": null,\n  \"check_id\": \"CONTROLLED-COMMAND-EXIT-001\",\n  \"criteria_version\": null,\n  \"acceptance_criteria\": [", f);
    char ac[128]; (void)snprintf(ac,sizeof(ac),"Observed process exit code equals expected exit code %d",expected); json_write_string(f,ac);
    fputs("],\n  \"subject_artifacts\": [{\"identity\": ", f); json_write_string(f, subject);
    fputs(", \"version\": null, \"sha256\": ", f); if (subject_hash_ok) json_write_string(f,subject_sha); else fputs("null",f);
    fputs(", \"size_bytes\": ", f); if (subject_hash_ok) fprintf(f,"%" PRIu64,subject_size); else fputs("null",f);
    fputs(", \"repository_root\": null, \"git_commit\": null, \"git_ref\": null}],\n  \"procedure\": {\n    \"kind\": \"command\",\n    \"description\": \"Execute the explicit argv vector without an intermediary shell; stdout and stderr are merged in TRANSCRIPT.log.\",\n    \"argv\": [", f);
    for (int i=0; i<cmdc; ++i) { if (i != 0) fputs(", ",f); json_write_string(f,cmdv[i]); }
    fputs("],\n    \"working_directory\": ",f); json_write_string(f,cwd);
    fputs(",\n    \"environment_ref\": null,\n    \"tool_versions\": {\"recorder\": \"0.1.0-alpha.1\", \"qh256_core\": \"2.0\"},\n    \"expected_exit_code\": ",f); fprintf(f,"%d\n  },\n",expected);
    fputs("  \"timing\": {\n    \"started_utc\": ",f); json_write_string(f,utc_start);
    fputs(",\n    \"ended_utc\": ",f); json_write_string(f,utc_end);
    fprintf(f,",\n    \"started_epoch\": %lld,\n    \"ended_epoch\": %lld\n  },\n",(long long)start,(long long)end);
    fputs("  \"execution\": {\n    \"status\": ",f); json_write_string(f,execution_status);
    fprintf(f,",\n    \"actual_exit_code\": %d,\n    \"expected_behavior\": ",actual); json_write_string(f,ac);
    char ob[256]; (void)snprintf(ob,sizeof(ob),"Observed process exit code: %d",actual); fputs(",\n    \"observed_behavior\": ",f); json_write_string(f,ob);
    fputs(",\n    \"preconditions_met\": true\n  },\n  \"outcome\": {\n    \"summary\": ",f);
    if (!match) (void)snprintf(ob,sizeof(ob),"Process exit criterion not met; actual=%d expected=%d. This is not a gate disposition.",actual,expected);
    else if (!capture_ok) (void)snprintf(ob,sizeof(ob),"Process exit criterion matched (actual=%d expected=%d), but transcript capture was incomplete; assessment is PARTIAL. This is not a gate disposition.",actual,expected);
    else (void)snprintf(ob,sizeof(ob),"Process exit criterion met; actual=%d expected=%d. This is not a gate disposition.",actual,expected);
    json_write_string(f,ob);
    fputs(",\n    \"gate_effect\": \"NO_GATE_EFFECT_UNTIL_REVIEW\",\n    \"reviewer\": null,\n    \"reviewed_utc\": null\n  },\n  \"raw_evidence\": [\n",f);
    const struct { const char *role,*name,*hash; uint64_t size; } raw[] = {
        {"input","COMMAND.txt",command_sha,command_size},
        {"transcript","TRANSCRIPT.log",transcript_sha,transcript_size},
        {"exit_status","RESULT.txt",result_sha,result_size}
    };
    for (size_t i=0u;i<sizeof(raw)/sizeof(raw[0]);++i) {
        if (i != 0u) fputs(",\n",f);
        fputs("    {\"role\": ",f); json_write_string(f,raw[i].role);
        fputs(", \"path\": ",f); json_write_string(f,raw[i].name);
        fputs(", \"sha256\": ",f); json_write_string(f,raw[i].hash);
        fprintf(f,", \"size_bytes\": %" PRIu64 "}",raw[i].size);
    }
    fputs("\n  ],\n  \"derived_evidence\": [],\n  \"limitations\": [\n"
          "    \"Execution status describes only the declared process-exit criterion; it does not establish project gate PASS.\",\n"
          "    \"stdout and stderr are merged; stream separation is not retained.\",\n"
          "    \"Environment capture is bounded to HOST.txt and named K501 variables.\",\n"
          "    \"The subject executable hash is null if executable identity could not be resolved before execution.\"",f);
    if (subject_hash_ok && !subject_hash_stable) fputs(",\n    \"The subject executable post-run identity or digest check failed or changed; identity stability is OPEN.\"",f);
    if (!capture_ok) fputs(",\n    \"Transcript capture encountered an I/O error; captured output may be incomplete and the assessment is PARTIAL unless a failed exit criterion already establishes FAIL.\"",f);
    fputs("\n  ]\n}\n",f);
    bool ok = ferror(f) == 0; if (fclose(f) != 0) ok = false; return ok;
}

static int write_manifest(const char *run_dir) {
    char path[PATH_MAX]; if (!path_join(path,sizeof(path),run_dir,"SHA256SUMS.txt")) return 1;
    int fd = open(path,O_WRONLY|O_CREAT|O_EXCL,0600); if (fd < 0) return 1;
    FILE *f = fdopen(fd,"w"); if (f == NULL) { close(fd); return 1; }
    bool ok = true;
    for (size_t i=0u;i<sizeof(EVIDENCE_FILES)/sizeof(EVIDENCE_FILES[0]);++i) {
        char hex[65]; uint64_t size=0u; char fp[PATH_MAX];
        if (!path_join(fp,sizeof(fp),run_dir,EVIDENCE_FILES[i]) || !sha256_file(fp,hex,&size)) { ok=false; break; }
        if (fprintf(f,"%s  %s\n",hex,EVIDENCE_FILES[i]) < 0) { ok=false; break; }
    }
    if (fclose(f) != 0) ok=false;
    return ok?0:1;
}

static bool write_result(const char *path, const char *run_id, int actual, int expected, time_t start, time_t end,
                         const char *start_utc, const char *end_utc, const char *start_local, const char *end_local) {
    int fd = open(path,O_WRONLY|O_CREAT|O_EXCL,0600); if(fd<0)return false;
    FILE *f=fdopen(fd,"w"); if(f==NULL){close(fd);return false;}
    const char *process_status=actual==0?"EXIT_ZERO":"NONZERO_EXIT";
    fprintf(f,"frame_id=%s\nprocess_status=%s\nexit_code=%d\nexpected_exit_code=%d\n",run_id,process_status,actual,expected);
    fprintf(f,"start_epoch_unix=%lld\nend_epoch_unix=%lld\nelapsed_seconds=%lld\n",(long long)start,(long long)end,(long long)(end-start));
    fprintf(f,"start_utc=%s\nend_utc=%s\nstart_europe_berlin=%s\nend_europe_berlin=%s\n",start_utc,end_utc,start_local,end_local);
    fprintf(f,"expected_exit_match=%s\ninterpretation=PROCESS_EXIT_ONLY_NOT_GATE_ACCEPTANCE\n",actual==expected?"true":"false");
    bool ok=ferror(f)==0;if(fclose(f)!=0)ok=false;return ok;
}

static int run_command(const char *root_arg, const char *area, const char *slug, int expected, char **cmdv, int cmdc) {
    if (!area_valid(area)) { fprintf(stderr,"ERROR: invalid area: %s\n",area); return 64; }
    if (!slug_valid(slug)) { fputs("ERROR: slug must be 1-64 chars from [A-Za-z0-9._-] and start alphanumeric\n",stderr); return 64; }
    for (int i=0;i<cmdc;++i) if (!json_string_valid_utf8((const unsigned char *)cmdv[i])) { fprintf(stderr,"ERROR: argv[%d] is not valid UTF-8; refusing ambiguous JSON evidence\n",i); return 64; }
    char rootbuf[PATH_MAX];
    if (realpath(root_arg,rootbuf)==NULL || !is_dir(rootbuf)) { fprintf(stderr,"ERROR: workspace root not found or not a directory: %s\n",root_arg); return 72; }
    char cwd[PATH_MAX]; if (getcwd(cwd,sizeof(cwd))==NULL) { perror("getcwd"); return 72; }
    char host[256]="unknown"; if (gethostname(host,sizeof(host)-1u)!=0) (void)snprintf(host,sizeof(host),"unknown"); host[sizeof(host)-1u]='\0';
    struct passwd *pw=getpwuid(geteuid()); const char *user=(pw!=NULL&&pw->pw_name!=NULL)?pw->pw_name:"unknown";
    time_t start=time(NULL); if(start==(time_t)-1){fputs("ERROR: cannot read system clock\n",stderr);return 72;}
    char start_utc[64],start_compact[64],start_local[64];
    if(!format_utc(start,start_utc,sizeof(start_utc),false)||!format_utc(start,start_compact,sizeof(start_compact),true)||!format_berlin(start,start_local,sizeof(start_local))){fputs("ERROR: timestamp formatting failed\n",stderr);return 72;}
    char runs_dir[PATH_MAX], area_dir[PATH_MAX], run_id[256], run_dir[PATH_MAX];
    if(!path_join(runs_dir,sizeof(runs_dir),rootbuf,"runs") || !mkdir_one(runs_dir)) { fprintf(stderr,"ERROR: cannot create runs directory under %s: %s\n",rootbuf,strerror(errno));return 73; }
    if(!path_join(area_dir,sizeof(area_dir),runs_dir,area)||!mkdir_one(area_dir)){fprintf(stderr,"ERROR: cannot create area directory: %s\n",strerror(errno));return 73;}
    int rn=snprintf(run_id,sizeof(run_id),"%lld_%s_%s",(long long)start,start_compact,slug);
    if(rn<0||(size_t)rn>=sizeof(run_id)||!path_join(run_dir,sizeof(run_dir),area_dir,run_id)){fputs("ERROR: run identifier too long\n",stderr);return 73;}
    if(mkdir(run_dir,0700)!=0){fprintf(stderr,"ERROR: run directory already exists or cannot be created; refusing overwrite: %s (%s)\n",run_dir,strerror(errno));return 73;}

    (void)setenv("K501_RUN_DIR",run_dir,1); (void)setenv("K501_ROOT",rootbuf,1); (void)setenv("K501_AREA",area,1);
    (void)setenv("K501_SLUG",slug,1); (void)setenv("K501_RUN_ID",run_id,1);
    char number[64]; (void)snprintf(number,sizeof(number),"%lld",(long long)start); (void)setenv("K501_START_EPOCH",number,1);
    (void)setenv("K501_START_UTC",start_utc,1); (void)setenv("K501_START_LOCAL",start_local,1); (void)setenv("K501_CWD",cwd,1); (void)setenv("K501_HOST",host,1);

    char fp[PATH_MAX];
    if(!path_join(fp,sizeof(fp),run_dir,"FRAME.json")||!write_frame(fp,run_id,area,slug,rootbuf,cwd,host,start,start_utc,start_local,cmdv,cmdc,expected)) goto create_error;
    if(!path_join(fp,sizeof(fp),run_dir,"COMMAND.txt")||!write_command_file(fp,run_id,area,slug,cmdv,cmdc,expected)) goto create_error;
    if(!path_join(fp,sizeof(fp),run_dir,"HOST.txt")||!write_host_file(fp,rootbuf,cwd,host,user,start,start_utc,start_local)) goto create_error;
    if(!path_join(fp,sizeof(fp),run_dir,"GIT_PRE.txt")||!write_git_snapshot(fp,cwd,start_utc)) goto create_error;
    if(!path_join(fp,sizeof(fp),run_dir,"STATUS.txt")||!write_exclusive_text(fp,"RUNNING\n")) goto create_error;

    char *subject=resolve_executable(cmdv[0]); char subject_sha[65]=""; uint64_t subject_size=0u;
    bool subject_hash_ok=subject!=NULL&&sha256_file(subject,subject_sha,&subject_size);
    char subject_sha_pre[65]=""; if(subject_hash_ok) memcpy(subject_sha_pre,subject_sha,sizeof(subject_sha_pre));
    puts("Starting K501 controlled run:"); printf("RUN_ID=%s\nEvidence directory: %s\n",run_id,run_dir);
    puts("--- COMMAND OUTPUT (stdout + stderr) ---"); fflush(stdout);
    int pipes[2]; if(pipe(pipes)!=0){perror("pipe");return 73;}
    pid_t pid=fork(); if(pid<0){perror("fork");close(pipes[0]);close(pipes[1]);return 73;}
    if(pid==0){close(pipes[0]);(void)dup2(pipes[1],STDOUT_FILENO);(void)dup2(pipes[1],STDERR_FILENO);close(pipes[1]);execvp(cmdv[0],cmdv);dprintf(STDERR_FILENO,"ERROR: execvp failed for %s: %s\n",cmdv[0],strerror(errno));_exit(errno==ENOENT?127:126);}
    close(pipes[1]);
    if(!path_join(fp,sizeof(fp),run_dir,"TRANSCRIPT.log")){close(pipes[0]);return 73;}
    int tfd=open(fp,O_WRONLY|O_CREAT|O_EXCL,0600); if(tfd<0){perror("TRANSCRIPT.log");close(pipes[0]);return 73;}
    unsigned char buf[READ_BUFFER_SIZE]; bool io_ok=true;
    for(;;){ssize_t n=read(pipes[0],buf,sizeof(buf));if(n<0){if(errno==EINTR)continue;io_ok=false;break;}if(n==0)break;
        if(!write_all(tfd,buf,(size_t)n)){io_ok=false;break;}
        size_t off=0u;while(off<(size_t)n){ssize_t w=write(STDOUT_FILENO,buf+off,(size_t)n-off);if(w<0){if(errno==EINTR)continue;break;}off+=(size_t)w;}
    }
    close(pipes[0]); if(close(tfd)!=0)io_ok=false;
    int ws=0; if(waitpid(pid,&ws,0)<0){perror("waitpid");return 73;}
    int actual=125; if(WIFEXITED(ws))actual=WEXITSTATUS(ws);else if(WIFSIGNALED(ws))actual=128+WTERMSIG(ws);
    time_t end=time(NULL); if(end==(time_t)-1)end=start;
    char end_utc[64],end_local[64]; if(!format_utc(end,end_utc,sizeof(end_utc),false)||!format_berlin(end,end_local,sizeof(end_local))){strcpy(end_utc,start_utc);strcpy(end_local,start_local);}
    if(!io_ok)fputs("WARNING: transcript/output write encountered an I/O error; evidence assessment is PARTIAL unless the exit criterion already establishes FAIL.\n",stderr);
    if(!path_join(fp,sizeof(fp),run_dir,"GIT_POST.txt")||!write_git_snapshot(fp,cwd,end_utc)) goto closeout_error;
    if(!path_join(fp,sizeof(fp),run_dir,"RESULT.txt")||!write_result(fp,run_id,actual,expected,start,end,start_utc,end_utc,start_local,end_local)) goto closeout_error;
    if(!path_join(fp,sizeof(fp),run_dir,"STATUS.txt")) goto closeout_error;
    char status_text[64]; (void)snprintf(status_text,sizeof(status_text),"%s\n",actual==0?"EXIT_ZERO":"NONZERO_EXIT");
    /* STATUS.txt already exists as RUNNING; replace only this new run's own status file in-place. */
    int sfd=open(fp,O_WRONLY|O_TRUNC); if(sfd<0||!write_all(sfd,status_text,strlen(status_text))){if(sfd>=0)close(sfd);goto closeout_error;} if(close(sfd)!=0)goto closeout_error;

    char subject_sha_post[65]=""; uint64_t subject_size_post=0u;
    bool subject_hash_post_ok=subject!=NULL&&sha256_file(subject,subject_sha_post,&subject_size_post);
    bool subject_hash_stable=subject_hash_ok&&subject_hash_post_ok&&strcmp(subject_sha_pre,subject_sha_post)==0&&subject_size==subject_size_post;
    if(!path_join(fp,sizeof(fp),run_dir,"EVIDENCE_RECORD.json")||!write_evidence_record(fp,run_id,area,slug,cwd,start_utc,end_utc,start,end,cmdv,cmdc,expected,actual,subject!=NULL?subject:cmdv[0],subject_sha_pre,subject_size,subject_hash_ok,subject_hash_stable,io_ok,run_dir)){free(subject);goto closeout_error;}
    free(subject);
    if(write_manifest(run_dir)!=0)goto closeout_error;
    printf("\nK501 run finished. PROCESS_STATUS=%s EXIT_CODE=%d EXPECTED_EXIT_CODE=%d EXPECTATION_MATCH=%s\n",actual==0?"EXIT_ZERO":"NONZERO_EXIT",actual,expected,actual==expected?"true":"false");
    printf("Evidence directory: %s\n",run_dir);
    puts("Interpretation: process-exit result only; no project gate is accepted by this command.");
    return io_ok ? actual : 73;

create_error:
    fprintf(stderr,"ERROR: could not create initial evidence set in %s: %s\n",run_dir,strerror(errno));
    return 73;
closeout_error:
    fprintf(stderr,"ERROR: closeout failed; incomplete run evidence retained at %s: %s\n",run_dir,strerror(errno));
    return 73;
}

static int command_identity(const char *path) {
    char resolved[PATH_MAX]; const char *use=realpath(path,resolved)!=NULL?resolved:path; char hex[65]; uint64_t size=0u;
    if(!sha256_file(use,hex,&size)){fprintf(stderr,"ERROR: cannot hash file: %s\n",path);return 66;}
    printf("identity=%s\nsize_bytes=%" PRIu64 "\nsha256=%s\n",use,size,hex);return 0;
}

int main(int argc, char **argv) {
    umask(0077);
    if(argc<2){print_usage(stderr);return 64;}
    if(strcmp(argv[1],"help")==0||strcmp(argv[1],"--help")==0||strcmp(argv[1],"-h")==0){print_usage(stdout);return 0;}
    if(strcmp(argv[1],"version")==0){printf("%s\nVersion: %s\nLanguage: C11\nRecorder protocol: %s\nEvidence schema: %s (source proposal; not ratified)\nQH256 core: v2.0 (separate state algebra; no evidence-status mapping)\n",K501_NAME,K501_VERSION,RUN_PROTOCOL,EVIDENCE_SCHEMA_VERSION);return 0;}
    if(strcmp(argv[1],"self-test")==0){int a=sha256_self_test(),b=qh256_self_test();if(a==0&&b==0){puts("SELF_TEST=PASS\nSHA256 known vectors: PASS\nQH256 v2 structural, MERGE_K and serialization tests: PASS\nEvidence-status mapping into QH256: NOT_DEFINED / NOT_PERFORMED");return 0;}fprintf(stderr,"SELF_TEST=FAIL sha256=%d qh256=%d\n",a,b);return 1;}
    if(strcmp(argv[1],"identity")==0){if(argc!=3){print_usage(stderr);return 64;}return command_identity(argv[2]);}
    if(strcmp(argv[1],"run")==0){if(argc<7){print_usage(stderr);return 64;}const char *root=argv[2],*area=argv[3],*slug=argv[4];int expected=0;int i=5;
        if(i+1<argc&&strcmp(argv[i],"--expect-exit")==0){char *end=NULL;errno=0;long x=strtol(argv[i+1],&end,10);if(errno!=0||end==argv[i+1]||*end!='\0'||x<0||x>255){fputs("ERROR: expected exit code must be an integer in 0..255\n",stderr);return 64;}expected=(int)x;i+=2;}
        if(i>=argc||strcmp(argv[i],"--")!=0||i+1>=argc){print_usage(stderr);return 64;}
        char **cmdv=&argv[i+1];int cmdc=argc-(i+1);return run_command(root,area,slug,expected,cmdv,cmdc);
    }
    fprintf(stderr,"ERROR: unknown command: %s\n",argv[1]);print_usage(stderr);return 64;
}
