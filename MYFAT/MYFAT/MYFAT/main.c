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
    
    // Create FAT32 image file (1GB)
//    create_image();
    
    // Open image file
    open_image();
    
    // Initialise File IO Library
    fl_init();
    
    // Attach media access functions to library
    if (fl_attach_media(read_image, write_image) != FAT_INIT_OK)
    {
        printf("ERROR: Media attach failed\n");
        return 1;
    }
    
    // Download metadata from google drive
    download_metadata();
    
    // Write metadata on image file & Make allocation table
    make_metadata();
    
    // List root directory
    fl_listdirectory("/");
    
    close_image();
    
    fl_shutdown();
}
