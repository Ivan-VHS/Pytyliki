#define _POSIX_C_SOURCE 200809L
#include "snapshot_store.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

typedef struct {
    pl_snapshot_info info;
    unsigned char *payload;
} snapshot_candidate;

static uint32_t snapshot_crc32(const unsigned char *data, size_t size) {
    uint32_t crc = UINT32_C(0xFFFFFFFF);
    size_t index;
    if (data == NULL && size != 0u) return 0u;
    for (index = 0u; index < size; ++index) {
        unsigned bit;
        crc ^= (uint32_t)data[index];
        for (bit = 0u; bit < 8u; ++bit) {
            const uint32_t mask = (uint32_t)-(int32_t)(crc & 1u);
            crc = (crc >> 1u) ^ (UINT32_C(0xEDB88320) & mask);
        }
    }
    return ~crc;
}

static int write_u32(FILE *file, uint32_t value) {
    unsigned char bytes[4];
    unsigned index;
    for (index = 0u; index < 4u; ++index) bytes[index] = (unsigned char)(value >> (index * 8u));
    return fwrite(bytes, 1u, sizeof bytes, file) == sizeof bytes;
}

static int write_u64(FILE *file, uint64_t value) {
    unsigned char bytes[8];
    unsigned index;
    for (index = 0u; index < 8u; ++index) bytes[index] = (unsigned char)(value >> (index * 8u));
    return fwrite(bytes, 1u, sizeof bytes, file) == sizeof bytes;
}

static int read_u32(FILE *file, uint32_t *value) {
    unsigned char bytes[4];
    unsigned index;
    uint32_t result = 0u;
    if (fread(bytes, 1u, sizeof bytes, file) != sizeof bytes) return 0;
    for (index = 0u; index < 4u; ++index) result |= (uint32_t)bytes[index] << (index * 8u);
    *value = result;
    return 1;
}

static int read_u64(FILE *file, uint64_t *value) {
    unsigned char bytes[8];
    unsigned index;
    uint64_t result = 0u;
    if (fread(bytes, 1u, sizeof bytes, file) != sizeof bytes) return 0;
    for (index = 0u; index < 8u; ++index) result |= (uint64_t)bytes[index] << (index * 8u);
    *value = result;
    return 1;
}

static int valid_path(const char *path) {
    size_t length = 0u;
    if (path == NULL) return 0;
    while (length <= PL_SNAPSHOT_PATH_MAX && path[length] != '\0') ++length;
    return length > 0u && length <= PL_SNAPSHOT_PATH_MAX;
}

static int durable_flush(FILE *file) {
    if (fflush(file) != 0) return 0;
#if defined(_WIN32)
    return _commit(_fileno(file)) == 0;
#else
    return fsync(fileno(file)) == 0;
#endif
}

static int replace_file(const char *source, const char *destination) {
#if defined(_WIN32)
    return MoveFileExA(source, destination,
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return rename(source, destination) == 0;
#endif
}

static void candidate_destroy(snapshot_candidate *candidate) {
    if (candidate == NULL) return;
    free(candidate->payload);
    candidate->payload = NULL;
    memset(&candidate->info, 0, sizeof candidate->info);
}

static pl_snapshot_status read_candidate(const char *path, snapshot_candidate *out) {
    FILE *file;
    unsigned char magic[4];
    uint32_t version = 0u, schema = 0u, crc = 0u;
    uint64_t generation = 0u, payload_size = 0u;
    unsigned char *payload;
    int trailing;
    if (out == NULL || !valid_path(path)) return PL_SNAPSHOT_INVALID;
    memset(out, 0, sizeof *out);
    file = fopen(path, "rb");
    if (file == NULL) return PL_SNAPSHOT_NOT_FOUND;
    if (fread(magic, 1u, sizeof magic, file) != sizeof magic ||
        memcmp(magic, "PLSJ", 4u) != 0 ||
        !read_u32(file, &version) || !read_u32(file, &schema) ||
        !read_u64(file, &generation) || !read_u64(file, &payload_size) ||
        !read_u32(file, &crc)) {
        (void)fclose(file);
        return PL_SNAPSHOT_INTEGRITY;
    }
    if (version != 1u || schema == 0u || generation == 0u) {
        (void)fclose(file);
        return PL_SNAPSHOT_INTEGRITY;
    }
    if (payload_size > (uint64_t)PL_SNAPSHOT_MAX_BYTES ||
        payload_size > (uint64_t)SIZE_MAX) {
        (void)fclose(file);
        return PL_SNAPSHOT_LIMIT;
    }
    payload = (unsigned char *)malloc(payload_size == 0u ? 1u : (size_t)payload_size);
    if (payload == NULL) {
        (void)fclose(file);
        return PL_SNAPSHOT_LIMIT;
    }
    if (payload_size != 0u &&
        fread(payload, 1u, (size_t)payload_size, file) != (size_t)payload_size) {
        free(payload);
        (void)fclose(file);
        return PL_SNAPSHOT_INTEGRITY;
    }
    trailing = fgetc(file);
    if (ferror(file) || trailing != EOF || fclose(file) != 0) {
        free(payload);
        return PL_SNAPSHOT_INTEGRITY;
    }
    if (snapshot_crc32(payload, (size_t)payload_size) != crc) {
        free(payload);
        return PL_SNAPSHOT_INTEGRITY;
    }
    out->payload = payload;
    out->info.schema_version = schema;
    out->info.generation = generation;
    out->info.payload_size = (size_t)payload_size;
    out->info.crc32 = crc;
    return PL_SNAPSHOT_OK;
}

static pl_snapshot_status pending_path(const char *path, char *output, size_t capacity) {
    const int written = snprintf(output, capacity, "%s.pending", path);
    if (written < 0 || (size_t)written >= capacity) return PL_SNAPSHOT_LIMIT;
    return PL_SNAPSHOT_OK;
}

pl_snapshot_status pl_snapshot_save_atomic(const char *path, uint32_t schema_version,
                                           uint64_t generation, const void *payload,
                                           size_t payload_size, pl_snapshot_info *info) {
    char pending[PL_SNAPSHOT_PATH_MAX + 16u];
    FILE *file;
    snapshot_candidate current;
    pl_snapshot_status current_status;
    uint32_t crc;
    if (!valid_path(path) || schema_version == 0u || generation == 0u ||
        (payload == NULL && payload_size != 0u))
        return PL_SNAPSHOT_INVALID;
    if (payload_size > PL_SNAPSHOT_MAX_BYTES) return PL_SNAPSHOT_LIMIT;
    if (pending_path(path, pending, sizeof pending) != PL_SNAPSHOT_OK)
        return PL_SNAPSHOT_LIMIT;

    current_status = read_candidate(path, &current);
    if (current_status == PL_SNAPSHOT_OK) {
        if (generation <= current.info.generation) {
            candidate_destroy(&current);
            return PL_SNAPSHOT_CONFLICT;
        }
        candidate_destroy(&current);
    } else if (current_status != PL_SNAPSHOT_NOT_FOUND &&
               current_status != PL_SNAPSHOT_INTEGRITY) {
        return current_status;
    }

    crc = snapshot_crc32((const unsigned char *)payload, payload_size);
    file = fopen(pending, "wb");
    if (file == NULL) return PL_SNAPSHOT_IO;
    if (fwrite("PLSJ", 1u, 4u, file) != 4u ||
        !write_u32(file, 1u) || !write_u32(file, schema_version) ||
        !write_u64(file, generation) || !write_u64(file, (uint64_t)payload_size) ||
        !write_u32(file, crc) ||
        (payload_size != 0u && fwrite(payload, 1u, payload_size, file) != payload_size) ||
        !durable_flush(file)) {
        (void)fclose(file);
        (void)remove(pending);
        return PL_SNAPSHOT_IO;
    }
    if (fclose(file) != 0) {
        (void)remove(pending);
        return PL_SNAPSHOT_IO;
    }
    if (!replace_file(pending, path)) return PL_SNAPSHOT_IO;
    if (info != NULL) {
        info->schema_version = schema_version;
        info->generation = generation;
        info->payload_size = payload_size;
        info->crc32 = crc;
        info->recovered_pending_write = 0;
    }
    return PL_SNAPSHOT_OK;
}

pl_snapshot_status pl_snapshot_recover(const char *path, void *output,
                                       size_t output_capacity, pl_snapshot_info *info) {
    char pending[PL_SNAPSHOT_PATH_MAX + 16u];
    snapshot_candidate primary, staged;
    pl_snapshot_status primary_status, staged_status, result;
    int use_staged = 0;
    if (!valid_path(path) || (output == NULL && output_capacity != 0u))
        return PL_SNAPSHOT_INVALID;
    if (pending_path(path, pending, sizeof pending) != PL_SNAPSHOT_OK)
        return PL_SNAPSHOT_LIMIT;
    primary_status = read_candidate(path, &primary);
    staged_status = read_candidate(pending, &staged);
    if (staged_status == PL_SNAPSHOT_OK &&
        (primary_status != PL_SNAPSHOT_OK ||
         staged.info.generation > primary.info.generation))
        use_staged = 1;

    if (use_staged) {
        if (!replace_file(pending, path)) {
            candidate_destroy(&primary);
            candidate_destroy(&staged);
            return PL_SNAPSHOT_IO;
        }
        candidate_destroy(&primary);
        result = PL_SNAPSHOT_OK;
        if (info != NULL) {
            *info = staged.info;
            info->recovered_pending_write = 1;
        }
        if (staged.info.payload_size > output_capacity) {
            candidate_destroy(&staged);
            return PL_SNAPSHOT_LIMIT;
        }
        if (staged.info.payload_size != 0u)
            memcpy(output, staged.payload, staged.info.payload_size);
        candidate_destroy(&staged);
        return result;
    }

    candidate_destroy(&staged);
    if (primary_status != PL_SNAPSHOT_OK) {
        candidate_destroy(&primary);
        return primary_status;
    }
    if (primary.info.payload_size > output_capacity) {
        if (info != NULL) *info = primary.info;
        candidate_destroy(&primary);
        return PL_SNAPSHOT_LIMIT;
    }
    if (primary.info.payload_size != 0u)
        memcpy(output, primary.payload, primary.info.payload_size);
    if (info != NULL) {
        *info = primary.info;
        info->recovered_pending_write = 0;
    }
    candidate_destroy(&primary);
    return PL_SNAPSHOT_OK;
}

const char *pl_snapshot_status_name(pl_snapshot_status status) {
    switch (status) {
    case PL_SNAPSHOT_OK: return "ok";
    case PL_SNAPSHOT_INVALID: return "invalid";
    case PL_SNAPSHOT_IO: return "io";
    case PL_SNAPSHOT_INTEGRITY: return "integrity";
    case PL_SNAPSHOT_LIMIT: return "limit";
    case PL_SNAPSHOT_NOT_FOUND: return "not-found";
    case PL_SNAPSHOT_CONFLICT: return "conflict";
    default: return "unknown";
    }
}
