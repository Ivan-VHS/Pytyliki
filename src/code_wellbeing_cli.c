#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

/*
 * Read-only Code Mental Health / Codebase Wellbeing inventory.
 * A software-maintenance metaphor only; never a human or clinical assessment.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define PATH_CAP 4096
#define FILE_LIMIT 5000UL
#define DEPTH_LIMIT 16
#define MAX_SOURCE_BYTES (1024UL * 1024UL)
#define MAX_TOTAL_SOURCE_BYTES (32UL * 1024UL * 1024UL)

typedef struct {
    unsigned long total_files, source_files, test_files, documentation_files;
    unsigned long readme_files, workflow_files, ownership_files, dependency_files;
    unsigned long recovery_files, incident_records, source_scanned, large_source_units;
    unsigned long todo, fixme, hack, xxx;
    unsigned long source_bytes_scanned;
    int source_scan_truncated;
    int truncated;
} report_state;

static report_state REPORT;

static const char *const SOURCE_EXTENSIONS[] = {
    ".c", ".h", ".cc", ".cpp", ".cxx", ".hpp", ".hxx", ".py",
    ".js", ".jsx", ".ts", ".tsx", ".rs", ".go", ".java", ".cs",
    ".rpr", ".qcs", ".lua"
};

static void lower_copy(char *out, size_t cap, const char *in) {
    size_t i;
    if (cap == 0U) return;
    for (i = 0U; i + 1U < cap && in[i] != '\0'; ++i)
        out[i] = (char)tolower((unsigned char)in[i]);
    out[i] = '\0';
}

static int contains_any(const char *text, const char *const *terms, size_t count) {
    size_t i;
    for (i = 0U; i < count; ++i)
        if (strstr(text, terms[i]) != NULL) return 1;
    return 0;
}

static int skipped_directory(const char *name) {
    static const char *const names[] = {
        ".git", "build", "dist", "target", "vendor", "node_modules",
        ".venv", "venv", "__pycache__", ".pytest_cache"
    };
    char lowered[256];
    size_t i;
    lower_copy(lowered, sizeof(lowered), name);
    for (i = 0U; i < sizeof(names) / sizeof(names[0]); ++i)
        if (strcmp(lowered, names[i]) == 0) return 1;
    return 0;
}

static int is_source(const char *lower_path) {
    const char *dot = strrchr(lower_path, '.');
    size_t i;
    if (dot == NULL) return 0;
    for (i = 0U; i < sizeof(SOURCE_EXTENSIONS) / sizeof(SOURCE_EXTENSIONS[0]); ++i)
        if (strcmp(dot, SOURCE_EXTENSIONS[i]) == 0) return 1;
    return 0;
}

static int is_document(const char *lower_path) {
    const char *dot = strrchr(lower_path, '.');
    return dot != NULL && (strcmp(dot, ".md") == 0 || strcmp(dot, ".mdx") == 0
        || strcmp(dot, ".rst") == 0 || strcmp(dot, ".adoc") == 0);
}

static int is_test(const char *lower_path) {
    const char *name = strrchr(lower_path, '/');
    name = name == NULL ? lower_path : name + 1;
    return strstr(lower_path, "tests/") != NULL || strstr(lower_path, "test/") != NULL
        || strstr(lower_path, "spec/") != NULL || strstr(lower_path, "specs/") != NULL
        || strncmp(name, "test_", 5U) == 0 || strncmp(name, "test.", 5U) == 0
        || strstr(name, "_test.py") != NULL || strstr(name, ".spec.") != NULL
        || strstr(name, ".test.") != NULL;
}

static int has_filename(const char *lower_path, const char *filename) {
    const char *name = strrchr(lower_path, '/');
    name = name == NULL ? lower_path : name + 1;
    return strcmp(name, filename) == 0;
}

static int is_dependency_manifest(const char *lower_path) {
    static const char *const names[] = {
        "pyproject.toml", "requirements.txt", "package.json", "package-lock.json",
        "pnpm-lock.yaml", "yarn.lock", "cargo.toml", "cargo.lock", "go.mod",
        "go.sum", "poetry.lock", "pipfile", "pipfile.lock", "vcpkg.json",
        "conanfile.py", "conanfile.txt"
    };
    const char *name = strrchr(lower_path, '/');
    name = name == NULL ? lower_path : name + 1;
    return contains_any(name, names, sizeof(names) / sizeof(names[0]));
}

static void scan_source_text(const char *absolute_path) {
    FILE *input;
    char line[8192];
    unsigned long lines = 0UL;
    unsigned long bytes = 0UL;
    if (REPORT.source_bytes_scanned >= MAX_TOTAL_SOURCE_BYTES) {
        REPORT.source_scan_truncated = 1;
        return;
    }
    input = fopen(absolute_path, "rb");
    if (input == NULL) return;
    while (fgets(line, (int)sizeof(line), input) != NULL) {
        size_t length = strlen(line);
        if (bytes + (unsigned long)length > MAX_SOURCE_BYTES
            || REPORT.source_bytes_scanned + (unsigned long)length > MAX_TOTAL_SOURCE_BYTES) {
            REPORT.source_scan_truncated = 1;
            break;
        }
        ++lines;
        bytes += (unsigned long)length;
        REPORT.source_bytes_scanned += (unsigned long)length;
        if (strstr(line, "TODO") != NULL) ++REPORT.todo;
        if (strstr(line, "FIXME") != NULL) ++REPORT.fixme;
        if (strstr(line, "HACK") != NULL) ++REPORT.hack;
        if (strstr(line, "XXX") != NULL) ++REPORT.xxx;
    }
    if (ferror(input) == 0) {
        ++REPORT.source_scanned;
        if (lines > 1000UL) ++REPORT.large_source_units;
    }
    fclose(input);
}

static void scan_file(const char *absolute_path, const char *relative_path) {
    char lower_path[PATH_CAP];
    static const char *const recovery_terms[] = {"rollback", "runbook", "recovery", "release-process"};
    static const char *const learning_terms[] = {"postmortem", "post-mortem", "incident", "adr"};
    lower_copy(lower_path, sizeof(lower_path), relative_path);

    ++REPORT.total_files;
    if (is_source(lower_path)) {
        ++REPORT.source_files;
        if (is_test(lower_path)) ++REPORT.test_files;
        scan_source_text(absolute_path);
    }
    if (is_document(lower_path)) ++REPORT.documentation_files;
    if (strncmp(strrchr(lower_path, '/') == NULL ? lower_path : strrchr(lower_path, '/') + 1,
                "readme", 6U) == 0) ++REPORT.readme_files;
    if (strstr(lower_path, ".github/workflows/") != NULL
        || has_filename(lower_path, ".gitlab-ci.yml")
        || has_filename(lower_path, "azure-pipelines.yml")
        || has_filename(lower_path, "bitbucket-pipelines.yml")) ++REPORT.workflow_files;
    if (has_filename(lower_path, "codeowners")) ++REPORT.ownership_files;
    if (is_dependency_manifest(lower_path)) ++REPORT.dependency_files;
    if (contains_any(lower_path, recovery_terms, sizeof(recovery_terms) / sizeof(recovery_terms[0])))
        ++REPORT.recovery_files;
    if (contains_any(lower_path, learning_terms, sizeof(learning_terms) / sizeof(learning_terms[0])))
        ++REPORT.incident_records;
}

static void make_path(char *out, size_t capacity, const char *root, const char *relative) {
#ifdef _WIN32
    const char *separator = "\\";
#else
    const char *separator = "/";
#endif
    if (relative[0] == '\0') (void)snprintf(out, capacity, "%s", root);
    else (void)snprintf(out, capacity, "%s%s%s", root, separator, relative);
}

#ifdef _WIN32
static void scan_directory(const char *root, const char *relative, int depth) {
    char directory[PATH_CAP], pattern[PATH_CAP];
    WIN32_FIND_DATAA data;
    HANDLE handle;
    if (REPORT.truncated) return;
    if (depth > DEPTH_LIMIT) { REPORT.truncated = 1; return; }
    make_path(directory, sizeof(directory), root, relative);
    (void)snprintf(pattern, sizeof(pattern), "%s\\*", directory);
    handle = FindFirstFileA(pattern, &data);
    if (handle == INVALID_HANDLE_VALUE) return;
    do {
        char child_relative[PATH_CAP], child_absolute[PATH_CAP];
        if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0) continue;
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) continue;
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
            if (skipped_directory(data.cFileName)) continue;
            if (relative[0] == '\0')
                (void)snprintf(child_relative, sizeof(child_relative), "%s", data.cFileName);
            else
                (void)snprintf(child_relative, sizeof(child_relative), "%s/%s", relative, data.cFileName);
            scan_directory(root, child_relative, depth + 1);
        } else {
            if (REPORT.total_files >= FILE_LIMIT) { REPORT.truncated = 1; break; }
            if (relative[0] == '\0')
                (void)snprintf(child_relative, sizeof(child_relative), "%s", data.cFileName);
            else
                (void)snprintf(child_relative, sizeof(child_relative), "%s/%s", relative, data.cFileName);
            make_path(child_absolute, sizeof(child_absolute), root, child_relative);
            scan_file(child_absolute, child_relative);
        }
    } while (FindNextFileA(handle, &data) != 0);
    FindClose(handle);
}
#else
static void scan_directory(const char *root, const char *relative, int depth) {
    char directory[PATH_CAP];
    DIR *handle;
    struct dirent *entry;
    if (REPORT.truncated || depth > DEPTH_LIMIT) return;
    make_path(directory, sizeof(directory), root, relative);
    handle = opendir(directory);
    if (handle == NULL) return;
    while ((entry = readdir(handle)) != NULL) {
        char child_relative[PATH_CAP], child_absolute[PATH_CAP];
        struct stat status;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if (relative[0] == '\0')
            (void)snprintf(child_relative, sizeof(child_relative), "%s", entry->d_name);
        else
            (void)snprintf(child_relative, sizeof(child_relative), "%s/%s", relative, entry->d_name);
        make_path(child_absolute, sizeof(child_absolute), root, child_relative);
        if (lstat(child_absolute, &status) != 0 || S_ISLNK(status.st_mode)) continue;
        if (S_ISDIR(status.st_mode)) {
            if (!skipped_directory(entry->d_name))
                scan_directory(root, child_relative, depth + 1);
        } else if (S_ISREG(status.st_mode)) {
            if (REPORT.total_files >= FILE_LIMIT) { REPORT.truncated = 1; break; }
            scan_file(child_absolute, child_relative);
        }
        if (REPORT.truncated) break;
    }
    closedir(handle);
}
#endif

static void print_report(int json, const char *project) {
    const char *verification = REPORT.test_files > 0UL && REPORT.workflow_files > 0UL ? "observed" : "missing";
    const char *documentation = REPORT.readme_files > 0UL ? "observed" : "missing";
    const char *ownership = REPORT.ownership_files > 0UL ? "observed" : "unknown";
    const char *dependencies = REPORT.dependency_files > 0UL ? "observed" : "unknown";
    const char *recovery = REPORT.recovery_files > 0UL ? "observed" : "unknown";
    const char *learning = REPORT.incident_records > 0UL ? "observed" : "unknown";
    if (json) {
        printf("{\n"
            "  \"schema_version\": 1,\n"
            "  \"concept\": \"Code Mental Health / Codebase Wellbeing v1\",\n"
            "  \"project\": \"selected\",\n"
            "  \"scan\": {\"read_only\": true, \"truncated\": %s, \"file_limit\": %lu, \"depth_limit\": %d},\n"
            "  \"inventory\": {"
            "\"total_files\": %lu, \"source_files\": %lu, \"test_files\": %lu, "
            "\"documentation_files\": %lu, \"readme_files\": %lu, \"ci_workflow_files\": %lu, "
            "\"ownership_files\": %lu, \"dependency_manifest_files\": %lu, \"recovery_guides\": %lu, "
            "\"incident_or_decision_records\": %lu, \"source_files_text_scanned\": %lu, "
            "\"source_bytes_scanned\": %lu, \"source_text_scan_truncated\": %s, "
            "\"large_source_units_over_1000_lines\": %lu, "
            "\"maintenance_markers\": {\"TODO\": %lu, \"FIXME\": %lu, \"HACK\": %lu, \"XXX\": %lu}},\n"
            "  \"signals\": ["
            "{\"id\":\"verification\",\"status\":\"%s\",\"note\":\"file presence does not prove test coverage or passing status\"},"
            "{\"id\":\"documentation\",\"status\":\"%s\",\"note\":\"README presence does not prove freshness\"},"
            "{\"id\":\"ownership-and-support\",\"status\":\"%s\",\"note\":\"no ownership file does not prove maintainers are unavailable\"},"
            "{\"id\":\"dependency-care\",\"status\":\"%s\",\"note\":\"manifest presence does not prove dependency freshness or safety\"},"
            "{\"id\":\"change-safety\",\"status\":\"%s\",\"note\":\"runbook presence does not prove rollback was rehearsed\"},"
            "{\"id\":\"incident-learning\",\"status\":\"%s\",\"note\":\"record presence does not prove lessons were applied\"},"
            "{\"id\":\"maintainability-review-cues\",\"status\":\"observed\",\"note\":\"markers and large files are review prompts, not defects\"}],\n"
            "  \"reporting_note\": \"No aggregate mental-health or code-quality score is computed.\",\n"
            "  \"privacy_note\": \"No contributor, commit-time, hardware, audio or device telemetry is inspected.\"\n"
            "}\n",
            REPORT.truncated ? "true" : "false", FILE_LIMIT, DEPTH_LIMIT, MAX_SOURCE_BYTES, MAX_TOTAL_SOURCE_BYTES,
            REPORT.total_files, REPORT.source_files, REPORT.test_files, REPORT.documentation_files,
            REPORT.readme_files, REPORT.workflow_files, REPORT.ownership_files, REPORT.dependency_files,
            REPORT.recovery_files, REPORT.incident_records, REPORT.source_scanned,
            REPORT.source_bytes_scanned, REPORT.source_scan_truncated ? "true" : "false", REPORT.large_source_units,
            REPORT.todo, REPORT.fixme, REPORT.hack, REPORT.xxx,
            verification, documentation, ownership, dependencies, recovery, learning);
        return;
    }
    puts("Code Mental Health / Codebase Wellbeing v1");
    printf("Project: %s | files: %lu | source: %lu | tests: %lu\n",
           project, REPORT.total_files, REPORT.source_files, REPORT.test_files);
    printf("Docs: %lu | CI workflows: %lu | source text inspected: %lu\n",
           REPORT.documentation_files, REPORT.workflow_files, REPORT.source_scanned);
    printf("- verification: %s — test and CI presence does not prove coverage or passing status\n", verification);
    printf("- documentation: %s — file presence does not prove freshness\n", documentation);
    printf("- ownership-and-support: %s — missing CODEOWNERS is not proof that no maintainers exist\n", ownership);
    printf("- dependency-care: %s — manifest presence does not prove dependency safety\n", dependencies);
    printf("- change-safety: %s — a named runbook does not prove a rehearsed rollback\n", recovery);
    printf("- incident-learning: %s — record presence does not prove lessons were applied\n", learning);
    printf("Review cues only: TODO=%lu FIXME=%lu HACK=%lu XXX=%lu large-source-units=%lu\n",
           REPORT.todo, REPORT.fixme, REPORT.hack, REPORT.xxx, REPORT.large_source_units);
    if (REPORT.truncated) puts("Inventory was capped by the 5000-file or 16-level directory limit.");
    puts("No aggregate score. No contributor ranking. No automatic source changes.");
    puts("This inventory does not prove correctness, security or maintainability.");
}

int main(int argc, char **argv) {
    const char *root = ".";
    int json = 0;
    int i;
    int path_set = 0;
    char absolute[PATH_CAP];
    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--json") == 0) json = 1;
        else if (strcmp(argv[i], "--help") == 0) {
            puts("Usage: pytyliki-code-wellbeing [--json] [PATH]");
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 2;
        } else if (root == NULL || strcmp(root, ".") != 0) {
            fputs("Only one project path may be supplied.\n", stderr);
            return 2;
        } else root = argv[i];
    }
    (void)snprintf(absolute, sizeof(absolute), "%s", root);
#ifdef _WIN32
    {
        DWORD attributes = GetFileAttributesA(absolute);
        if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0U
            || (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
            fprintf(stderr, "Not a regular project directory: %s\n", absolute);
            return 2;
        }
    }
#else
    {
        struct stat status;
        if (lstat(absolute, &status) != 0 || !S_ISDIR(status.st_mode) || S_ISLNK(status.st_mode)) {
            fprintf(stderr, "Not a regular project directory: %s\n", absolute);
            return 2;
        }
    }
#endif
    scan_directory(absolute, "", 0);
    print_report(json, absolute);
    return 0;
}
