#include "pocket_d20_storage.h"

#include <furi.h>
#include <string.h>

#define TAG "PocketD20Storage"

#define POCKET_D20_SAVE_PATH APP_DATA_PATH("character.save")
#define POCKET_D20_TEMP_PATH APP_DATA_PATH("character.tmp")
#define POCKET_D20_BACKUP_PATH APP_DATA_PATH("character.bak")

typedef struct {
    char magic[8];
    uint16_t version;
    uint16_t reserved;
    uint32_t payload_size;
    uint32_t checksum;
} PocketFileHeader;

static const char pocket_d20_magic[8] = {'P', 'D', '2', '0', 'S', 'A', 'V', 'E'};

static uint32_t pocket_d20_checksum(const void* data, size_t size) {
    const uint8_t* bytes = data;
    uint32_t hash = 2166136261UL;
    for(size_t i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619UL;
    }
    return hash;
}

static bool pocket_d20_load_path(Storage* storage, const char* path, PocketSaveData* data) {
    bool success = false;
    File* file = storage_file_alloc(storage);
    PocketFileHeader header = {0};

    bool opened = storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING);
    if(opened && storage_file_read(file, &header, sizeof(header)) == sizeof(header) &&
       memcmp(header.magic, pocket_d20_magic, sizeof(header.magic)) == 0 &&
       header.version == POCKET_D20_SAVE_VERSION && header.payload_size == sizeof(*data) &&
       storage_file_read(file, data, sizeof(*data)) == sizeof(*data) &&
       header.checksum == pocket_d20_checksum(data, sizeof(*data))) {
        pocket_d20_data_sanitize(data);
        success = true;
    }

    storage_file_close(file);
    storage_file_free(file);
    return success;
}

bool pocket_d20_storage_load(Storage* storage, PocketSaveData* data, bool* recovered_backup) {
    furi_assert(storage);
    furi_assert(data);
    if(recovered_backup) *recovered_backup = false;

    if(pocket_d20_load_path(storage, POCKET_D20_SAVE_PATH, data)) return true;
    if(pocket_d20_load_path(storage, POCKET_D20_BACKUP_PATH, data)) {
        if(recovered_backup) *recovered_backup = true;
        return true;
    }
    pocket_d20_data_set_defaults(data);
    return false;
}

bool pocket_d20_storage_save(Storage* storage, const PocketSaveData* data) {
    furi_assert(storage);
    furi_assert(data);

    PocketFileHeader header = {0};
    memcpy(header.magic, pocket_d20_magic, sizeof(header.magic));
    header.version = POCKET_D20_SAVE_VERSION;
    header.payload_size = sizeof(*data);
    header.checksum = pocket_d20_checksum(data, sizeof(*data));

    File* file = storage_file_alloc(storage);
    bool opened = storage_file_open(file, POCKET_D20_TEMP_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    bool written = false;
    if(opened) {
        written = storage_file_write(file, &header, sizeof(header)) == sizeof(header) &&
                  storage_file_write(file, data, sizeof(*data)) == sizeof(*data) &&
                  storage_file_sync(file);
    }
    storage_file_close(file);
    storage_file_free(file);

    if(!written) {
        FURI_LOG_E(TAG, "Failed to write temporary save");
        storage_common_remove(storage, POCKET_D20_TEMP_PATH);
        return false;
    }

    storage_common_remove(storage, POCKET_D20_BACKUP_PATH);
    bool had_save = storage_file_exists(storage, POCKET_D20_SAVE_PATH);
    if(had_save &&
       storage_common_rename(storage, POCKET_D20_SAVE_PATH, POCKET_D20_BACKUP_PATH) != FSE_OK) {
        FURI_LOG_E(TAG, "Failed to rotate save to backup");
        storage_common_remove(storage, POCKET_D20_TEMP_PATH);
        return false;
    }

    if(storage_common_rename(storage, POCKET_D20_TEMP_PATH, POCKET_D20_SAVE_PATH) != FSE_OK) {
        FURI_LOG_E(TAG, "Failed to promote temporary save");
        if(had_save)
            storage_common_rename(storage, POCKET_D20_BACKUP_PATH, POCKET_D20_SAVE_PATH);
        return false;
    }

    return true;
}

