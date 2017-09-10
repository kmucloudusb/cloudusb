#include <stdio.h>
#include "fat_custom.h"
#include "fat_filelib.h"

int main(int argc, char *argv[])
{
    // Read paths to script, pipe
    read_path(argv[0]);
    
    // Create boot record area
    create_reserved_area();
    
    // Create fat area
    create_fat_area();
    
    // Initialise File IO Library
    fl_init();
    
    // Attach media access functions to library
    if (fl_attach_media(read_virtual, write_virtual) != FAT_INIT_OK)
    {
        printf("ERROR: Media attach failed\n");
        return 1;
    }
    
    // Create root directory entry
    create_rootdir_entry();
    
    // Download metadata from google drive
    download_metadata();
    
    // Make allocation table
    write_entries();
    
    run_module();
    
    fl_shutdown();
}
