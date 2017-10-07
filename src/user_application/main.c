#include "kernel.h"
#include "file_system.h"

int main()
{
    // Set FAT32 file-system fixed area
    fat_init();
    
    // Download metadata from cloud storage
    download_metadata();
    
    // Synchronize with cloud storage
    sync_with_cloud();
    
    // Run kernel module to communicate with OS
    run_module();
}
