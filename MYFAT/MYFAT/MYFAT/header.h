#ifndef header_h
#define header_h

#include <string.h>
#include "fat_defs.h"
#include "fat_opts.h"
#include "fat_types.h"

//
//
// FL_FILE

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

struct fat_node
{
    struct fat_node    *previous;
    struct fat_node    *next;
};

struct fat_list
{
    struct fat_node    *head;
    struct fat_node    *tail;
};

struct cluster_lookup
{
    uint32 ClusterIdx;
    uint32 CurrentCluster;
};

typedef struct sFL_FILE
{
    uint32                  parentcluster;
    uint32                  startcluster;
    uint32                  bytenum;
    uint32                  filelength;
    int                     filelength_changed;
    char                    path[FATFS_MAX_LONG_FILENAME];
    char                    filename[FATFS_MAX_LONG_FILENAME];
    uint8                   shortfilename[11];
    
    // Cluster Lookup
    struct cluster_lookup   last_fat_lookup;
    
    // Read/Write sector buffer
    uint8                   file_data_sector[FAT_SECTOR_SIZE];
    uint32                  file_data_address;
    int                     file_data_dirty;
    
    // File fopen flags
    uint8                   flags;
#define FILE_READ           (1 << 0)
#define FILE_WRITE          (1 << 1)
#define FILE_APPEND         (1 << 2)
#define FILE_BINARY         (1 << 3)
#define FILE_ERASE          (1 << 4)
#define FILE_CREATE         (1 << 5)
    
    struct fat_node         list_node;
} FL_FILE;



//
//
// fl_init

#ifndef FAT_ASSERT
#define FAT_ASSERT(x)
#endif

#define fat_list_first(l)           (l)->head

static struct fat_list    _open_file_list;
static struct fat_list    _free_file_list;
static FL_FILE            _files[FATFS_MAX_OPEN_FILES];
static int                _filelib_init = 0;

static void fat_list_init(struct fat_list *list)
{
    FAT_ASSERT(list);
    
    list->head = list->tail = 0;
}

static void fat_list_remove(struct fat_list *list, struct fat_node *node)
{
    FAT_ASSERT(list);
    FAT_ASSERT(node);
    
    if(!node->previous)
        list->head = node->next;
    else
        node->previous->next = node->next;
    
    if(!node->next)
        list->tail = node->previous;
    else
        node->next->previous = node->previous;
}

static void fat_list_insert_after(struct fat_list *list, struct fat_node *node, struct fat_node *new_node)
{
    FAT_ASSERT(list);
    FAT_ASSERT(node);
    FAT_ASSERT(new_node);
    
    new_node->previous = node;
    new_node->next = node->next;
    if (!node->next)
        list->tail = new_node;
    else
        node->next->previous = new_node;
    node->next = new_node;
}

static void fat_list_insert_before(struct fat_list *list, struct fat_node *node, struct fat_node *new_node)
{
    FAT_ASSERT(list);
    FAT_ASSERT(node);
    FAT_ASSERT(new_node);
    
    new_node->previous = node->previous;
    new_node->next = node;
    if (!node->previous)
        list->head = new_node;
    else
        node->previous->next = new_node;
    node->previous = new_node;
}

static void fat_list_insert_first(struct fat_list *list, struct fat_node *node)
{
    FAT_ASSERT(list);
    FAT_ASSERT(node);
    
    if (!list->head)
    {
        list->head = node;
        list->tail = node;
        node->previous = 0;
        node->next = 0;
    }
    else
        fat_list_insert_before(list, list->head, node);
}

static void fat_list_insert_last(struct fat_list *list, struct fat_node *node)
{
    FAT_ASSERT(list);
    FAT_ASSERT(node);
    
    if (!list->tail)
        fat_list_insert_first(list, node);
    else
        fat_list_insert_after(list, list->tail, node);
}

static int fat_list_is_empty(struct fat_list *list)
{
    FAT_ASSERT(list);
    
    return !list->head;
}

static struct fat_node * fat_list_pop_head(struct fat_list *list)
{
    struct fat_node * node;
    
    FAT_ASSERT(list);
    
    node = fat_list_first(list);
    if (node)
        fat_list_remove(list, node);
    
    return node;
}

void fl_init(void)
{
    int i;
    
    fat_list_init(&_free_file_list);
    fat_list_init(&_open_file_list);
    
    // Add all file objects to free list
    for (i=0;i<FATFS_MAX_OPEN_FILES;i++)
        fat_list_insert_last(&_free_file_list, &_files[i].list_node);
    
    _filelib_init = 1;
}



//
//
// fl_attach_media

#define FAT_INIT_OK                         0
#define FAT_INIT_MEDIA_ACCESS_ERROR         (-1)
#define FAT_INIT_INVALID_SECTOR_SIZE        (-2)
#define FAT_INIT_INVALID_SIGNATURE          (-3)
#define FAT_INIT_ENDIAN_ERROR               (-4)
#define FAT_INIT_WRONG_FILESYS_TYPE         (-5)
#define FAT_INIT_WRONG_PARTITION_TYPE       (-6)
#define FAT_INIT_STRUCT_PACKING             (-7)

#define GET_32BIT_WORD(buffer, location)    \
( ((uint32)buffer[location+3]<<24) + ((uint32)buffer[location+2]<<16) + ((uint32)buffer[location+1]<<8) + (uint32)buffer[location+0] )
#define GET_16BIT_WORD(buffer, location)    \
( ((uint16)buffer[location+1]<<8) + (uint16)buffer[location+0] )

#define CHECK_FL_INIT()     { if (_filelib_init==0) fl_init(); }

typedef int (*fn_diskio_read) (uint32 sector, uint8 *buffer, uint32 sector_count);
typedef int (*fn_diskio_write)(uint32 sector, uint8 *buffer, uint32 sector_count);

struct disk_if
{
    // User supplied function pointers for disk IO
    fn_diskio_read          read_media;
    fn_diskio_write         write_media;
};

typedef enum eFatType
{
    FAT_TYPE_16,
    FAT_TYPE_32
} tFatType;

struct fat_buffer
{
    uint8                   sector[FAT_SECTOR_SIZE * FAT_BUFFER_SECTORS];
    uint32                  address;
    int                     dirty;
    uint8 *                 ptr;
    
    // Next in chain of sector buffers
    struct fat_buffer       *next;
};

struct fatfs
{
    // Filesystem globals
    uint8                   sectors_per_cluster;
    uint32                  cluster_begin_lba;
    uint32                  rootdir_first_cluster;
    uint32                  rootdir_first_sector;
    uint32                  rootdir_sectors;
    uint32                  fat_begin_lba;
    uint16                  fs_info_sector;
    uint32                  lba_begin;
    uint32                  fat_sectors;
    uint32                  next_free_cluster;
    uint16                  root_entry_count;
    uint16                  reserved_sectors;
    uint8                   num_of_fats;
    tFatType                fat_type;
    
    // Disk/Media API
    struct disk_if          disk_io;
    
    // [Optional] Thread Safety
    void                    (*fl_lock)(void);
    void                    (*fl_unlock)(void);
    
    // Working buffer
    struct fat_buffer        currentsector;
    
    // FAT Buffer
    struct fat_buffer        *fat_buffer_head;
    struct fat_buffer        fat_buffers[FAT_BUFFERS];
};

static int                _filelib_valid = 0;
static struct fatfs       _fs;

void fatfs_fat_init(struct fatfs *fs)
{
    int i;
    
    // FAT buffer chain head
    fs->fat_buffer_head = NULL;
    
    for (i=0;i<FAT_BUFFERS;i++)
    {
        // Initialise buffers to invalid
        fs->fat_buffers[i].address = FAT32_INVALID_CLUSTER;
        fs->fat_buffers[i].dirty = 0;
        memset(fs->fat_buffers[i].sector, 0x00, sizeof(fs->fat_buffers[i].sector));
        fs->fat_buffers[i].ptr = NULL;
        
        // Add to head of queue
        fs->fat_buffers[i].next = fs->fat_buffer_head;
        fs->fat_buffer_head = &fs->fat_buffers[i];
    }
}

int fatfs_init(struct fatfs *fs)
{
    uint8 num_of_fats;
    uint16 reserved_sectors;
    uint32 FATSz;
    uint32 root_dir_sectors;
    uint32 total_sectors;
    uint32 data_sectors;
    uint32 count_of_clusters;
    uint8 valid_partition = 0;
    
    fs->currentsector.address = FAT32_INVALID_CLUSTER;
    fs->currentsector.dirty = 0;
    
    fs->next_free_cluster = 0; // Invalid
    
    fatfs_fat_init(fs);
    
    // Make sure we have a read function (write function is optional)
    if (!fs->disk_io.read_media)
        return FAT_INIT_MEDIA_ACCESS_ERROR;
    
    // MBR: Sector 0 on the disk
    // NOTE: Some removeable media does not have this.
    
    // Load MBR (LBA 0) into the 512 byte buffer
    if (!fs->disk_io.read_media(0, fs->currentsector.sector, 1))
        return FAT_INIT_MEDIA_ACCESS_ERROR;
    
    // Make Sure 0x55 and 0xAA are at end of sector
    // (this should be the case regardless of the MBR or boot sector)
    if (fs->currentsector.sector[SIGNATURE_POSITION] != 0x55 || fs->currentsector.sector[SIGNATURE_POSITION+1] != 0xAA)
        return FAT_INIT_INVALID_SIGNATURE;
    
    // Now check again using the access function to prove endian conversion function
    if (GET_16BIT_WORD(fs->currentsector.sector, SIGNATURE_POSITION) != SIGNATURE_VALUE)
        return FAT_INIT_ENDIAN_ERROR;
    
    // Verify packed structures
    if (sizeof(struct fat_dir_entry) != FAT_DIR_ENTRY_SIZE)
        return FAT_INIT_STRUCT_PACKING;
    
    // Check the partition type code
    switch(fs->currentsector.sector[PARTITION1_TYPECODE_LOCATION])
    {
        case 0x0B:
        case 0x06:
        case 0x0C:
        case 0x0E:
        case 0x0F:
        case 0x05:
            valid_partition = 1;
            break;
        case 0x00:
            valid_partition = 0;
            break;
        default:
            if (fs->currentsector.sector[PARTITION1_TYPECODE_LOCATION] <= 0x06)
                valid_partition = 1;
            break;
    }
    
    // Read LBA Begin for the file system
    if (valid_partition)
        fs->lba_begin = GET_32BIT_WORD(fs->currentsector.sector, PARTITION1_LBA_BEGIN_LOCATION);
    // Else possibly MBR less disk
    else
        fs->lba_begin = 0;
    
    // Load Volume 1 table into sector buffer
    // (We may already have this in the buffer if MBR less drive!)
    if (!fs->disk_io.read_media(fs->lba_begin, fs->currentsector.sector, 1))
        return FAT_INIT_MEDIA_ACCESS_ERROR;
    
    // Make sure there are 512 bytes per cluster
    if (GET_16BIT_WORD(fs->currentsector.sector, 0x0B) != FAT_SECTOR_SIZE)
        return FAT_INIT_INVALID_SECTOR_SIZE;
    
    // Load Parameters of FAT partition
    fs->sectors_per_cluster = fs->currentsector.sector[BPB_SECPERCLUS];
    reserved_sectors = GET_16BIT_WORD(fs->currentsector.sector, BPB_RSVDSECCNT);
    num_of_fats = fs->currentsector.sector[BPB_NUMFATS];
    fs->root_entry_count = GET_16BIT_WORD(fs->currentsector.sector, BPB_ROOTENTCNT);
    
    if(GET_16BIT_WORD(fs->currentsector.sector, BPB_FATSZ16) != 0)
        fs->fat_sectors = GET_16BIT_WORD(fs->currentsector.sector, BPB_FATSZ16);
    else
        fs->fat_sectors = GET_32BIT_WORD(fs->currentsector.sector, BPB_FAT32_FATSZ32);
    
    // For FAT32 (which this may be)
    fs->rootdir_first_cluster = GET_32BIT_WORD(fs->currentsector.sector, BPB_FAT32_ROOTCLUS);
    fs->fs_info_sector = GET_16BIT_WORD(fs->currentsector.sector, BPB_FAT32_FSINFO);
    
    // For FAT16 (which this may be), rootdir_first_cluster is actuall rootdir_first_sector
    fs->rootdir_first_sector = reserved_sectors + (num_of_fats * fs->fat_sectors);
    fs->rootdir_sectors = ((fs->root_entry_count * 32) + (FAT_SECTOR_SIZE - 1)) / FAT_SECTOR_SIZE;
    
    // First FAT LBA address
    fs->fat_begin_lba = fs->lba_begin + reserved_sectors;
    
    // The address of the first data cluster on this volume
    fs->cluster_begin_lba = fs->fat_begin_lba + (num_of_fats * fs->fat_sectors);
    
    if (GET_16BIT_WORD(fs->currentsector.sector, 0x1FE) != 0xAA55) // This signature should be AA55
        return FAT_INIT_INVALID_SIGNATURE;
    
    // Calculate the root dir sectors
    root_dir_sectors = ((GET_16BIT_WORD(fs->currentsector.sector, BPB_ROOTENTCNT) * 32) + (GET_16BIT_WORD(fs->currentsector.sector, BPB_BYTSPERSEC) - 1)) / GET_16BIT_WORD(fs->currentsector.sector, BPB_BYTSPERSEC);
    
    if(GET_16BIT_WORD(fs->currentsector.sector, BPB_FATSZ16) != 0)
        FATSz = GET_16BIT_WORD(fs->currentsector.sector, BPB_FATSZ16);
    else
        FATSz = GET_32BIT_WORD(fs->currentsector.sector, BPB_FAT32_FATSZ32);
    
    if(GET_16BIT_WORD(fs->currentsector.sector, BPB_TOTSEC16) != 0)
        total_sectors = GET_16BIT_WORD(fs->currentsector.sector, BPB_TOTSEC16);
    else
        total_sectors = GET_32BIT_WORD(fs->currentsector.sector, BPB_TOTSEC32);
    
    data_sectors = total_sectors - (GET_16BIT_WORD(fs->currentsector.sector, BPB_RSVDSECCNT) + (fs->currentsector.sector[BPB_NUMFATS] * FATSz) + root_dir_sectors);
    
    // Find out which version of FAT this is...
    if (fs->sectors_per_cluster != 0)
    {
        count_of_clusters = data_sectors / fs->sectors_per_cluster;
        
        if(count_of_clusters < 4085)
            // Volume is FAT12
            return FAT_INIT_WRONG_FILESYS_TYPE;
        else if(count_of_clusters < 65525)
        {
            // Clear this FAT32 specific param
            fs->rootdir_first_cluster = 0;
            
            // Volume is FAT16
            fs->fat_type = FAT_TYPE_16;
            return FAT_INIT_OK;
        }
        else
        {
            // Volume is FAT32
            fs->fat_type = FAT_TYPE_32;
            return FAT_INIT_OK;
        }
    }
    else
        return FAT_INIT_WRONG_FILESYS_TYPE;
}

int fl_attach_media(fn_diskio_read rd, fn_diskio_write wr)
{
    int res;
    
    // If first call to library, initialise
    CHECK_FL_INIT();
    
    _fs.disk_io.read_media = rd;
    _fs.disk_io.write_media = wr;
    
    // Initialise FAT parameters
    if ((res = fatfs_init(&_fs)) != FAT_INIT_OK)
    {
        FAT_PRINTF(("FAT_FS: Error could not load FAT details (%d)!\r\n", res));
        return res;
    }
    
    _filelib_valid = 1;
    return FAT_INIT_OK;
}

#endif /* header_h */
