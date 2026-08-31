#include "dnd_handoff.h"

#include <furi.h>
#include <loader/loader.h>
#include <storage/storage.h>

bool dnd_handoff_launch(const char* fap_path, const char* args) {
    if(!fap_path || !fap_path[0]) return false;

    Loader* loader = furi_record_open(RECORD_LOADER);
    if(!loader) return false;

    loader_enqueue_launch(loader, fap_path, args, LoaderDeferredLaunchFlagGui);
    furi_record_close(RECORD_LOADER);
    return true;
}


bool dnd_handoff_launch_if_present(const char* fap_path, const char* args) {
    if(!fap_path || !fap_path[0]) return false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage) return false;
    bool present = storage_file_exists(storage, fap_path);
    furi_record_close(RECORD_STORAGE);
    if(!present) return false;

    return dnd_handoff_launch(fap_path, args);
}
