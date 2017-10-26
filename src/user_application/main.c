#include "kernel.h"
#include "file_system.h"

int main()
{
    // Set FAT32 file-system fixed area
    fat_init();

    // Download metadata from google drive
    download_metadata();

    // Make allocation table
    sync_with_cloud();

    run_module();
}
