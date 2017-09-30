#include "kernel.h"
#include "file_system.h"

int main() {
    set_root_dir_entry();

    // Create boot record area
    create_reserved_area();

    // Create fat area
    create_fat_area();

    // Download metadata from google drive
    download_metadata();

    // Make allocation table
    write_entries();

    run_module();

    return 0;
}
