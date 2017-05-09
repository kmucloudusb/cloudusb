#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "fat_filelib.h"
#include "fat_custom.h"

int main(int argc, char *argv[])
{
    // Read paths to image, script, pipe
    read_path(argv[0]);
    
    create_boot_record();
    create_fat();
    
    // Initialise File IO Library
    fl_init();
    
    // Attach media access functions to library
    if (fl_attach_media(read_virtual, write_virtual) != FAT_INIT_OK)
    {
        printf("ERROR: Media attach failed\n");
        return 1;
    }
    
    create_rootdir_entry();
    
    // Download metadata from google drive
    download_metadata();
    
    // Make table
    write_entries();
    
    fl_listdirectory("/");
    
    fl_shutdown();
}
