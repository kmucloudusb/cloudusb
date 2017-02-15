#ifndef header_h
#define header_h

#include <string.h>
#include "fat_defs.h"
#include "fat_opts.h"
#include "fat_types.h"
#include "fat_string.h"

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

#define fat_list_first(l)           (l)->head

static struct fat_list    _open_file_list;
static struct fat_list    _free_file_list;
static FL_FILE            _files[FATFS_MAX_OPEN_FILES];
static int                _filelib_init = 0;

static void fat_list_init(struct fat_list *list)
{
    list->head = list->tail = 0;
}

static void fat_list_remove(struct fat_list *list, struct fat_node *node)
{
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
    if (!list->tail)
        fat_list_insert_first(list, node);
    else
        fat_list_insert_after(list, list->tail, node);
}

static int fat_list_is_empty(struct fat_list *list)
{
    return !list->head;
}

static struct fat_node * fat_list_pop_head(struct fat_list *list)
{
    struct fat_node * node;
    
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



//
//
// fl_listdirectory

#define FL_LOCK(a)          do { if ((a)->fl_lock) (a)->fl_lock(); } while (0)
#define FL_UNLOCK(a)        do { if ((a)->fl_unlock) (a)->fl_unlock(); } while (0)

#define FAT_DIR_ENTRIES_PER_SECTOR          (FAT_SECTOR_SIZE / FAT_DIR_ENTRY_SIZE)

#define FAT32_GET_32BIT_WORD(pbuf, location)        ( GET_32BIT_WORD(pbuf->ptr, location) )
#define FAT32_SET_32BIT_WORD(pbuf, location, value) { SET_32BIT_WORD(pbuf->ptr, location, value); pbuf->dirty = 1; }
#define FAT16_GET_16BIT_WORD(pbuf, location)        ( GET_16BIT_WORD(pbuf->ptr, location) )
#define FAT16_SET_16BIT_WORD(pbuf, location, value) { SET_16BIT_WORD(pbuf->ptr, location, value); pbuf->dirty = 1; }

#define MAX_LONGFILENAME_ENTRIES    20
#define MAX_LFN_ENTRY_LENGTH        13

typedef struct fs_dir_list_status
{
    uint32                  sector;
    uint32                  cluster;
    uint8                   offset;
}FL_DIR;

typedef struct fs_dir_ent
{
    char                    filename[FATFS_MAX_LONG_FILENAME];
    uint8                   is_dir;
    uint32                  cluster;
    uint32                  size;
    
#if FATFS_INC_TIME_DATE_SUPPORT
    uint16                  access_date;
    uint16                  write_time;
    uint16                  write_date;
    uint16                  create_date;
    uint16                  create_time;
#endif
}fl_dirent;

static int fatfs_fat_writeback(struct fatfs *fs, struct fat_buffer *pcur)
{
    if (pcur)
    {
        // Writeback sector if changed
        if (pcur->dirty)
        {
            if (fs->disk_io.write_media)
            {
                uint32 sectors = FAT_BUFFER_SECTORS;
                uint32 offset = pcur->address - fs->fat_begin_lba;
                
                // Limit to sectors used for the FAT
                if ((offset + FAT_BUFFER_SECTORS) <= fs->fat_sectors)
                    sectors = FAT_BUFFER_SECTORS;
                else
                    sectors = fs->fat_sectors - offset;
                
                if (!fs->disk_io.write_media(pcur->address, pcur->sector, sectors))
                    return 0;
            }
            
            pcur->dirty = 0;
        }
        
        return 1;
    }
    else
        return 0;
}

static struct fat_buffer *fatfs_fat_read_sector(struct fatfs *fs, uint32 sector)
{
    struct fat_buffer *last = NULL;
    struct fat_buffer *pcur = fs->fat_buffer_head;
    
    // Itterate through sector buffer list
    while (pcur)
    {
        // Sector within this buffer?
        if ((sector >= pcur->address) && (sector < (pcur->address + FAT_BUFFER_SECTORS)))
            break;
        
        // End of list?
        if (pcur->next == NULL)
        {
            // Remove buffer from list
            if (last)
                last->next = NULL;
            // We the first and last buffer in the chain?
            else
                fs->fat_buffer_head = NULL;
        }
        
        last = pcur;
        pcur = pcur->next;
    }
    
    // We found the sector already in FAT buffer chain
    if (pcur)
    {
        pcur->ptr = (uint8 *)(pcur->sector + ((sector - pcur->address) * FAT_SECTOR_SIZE));
        return pcur;
    }
    
    // Else, we removed the last item from the list
    pcur = last;
    
    // Add to start of sector buffer list (now newest sector)
    pcur->next = fs->fat_buffer_head;
    fs->fat_buffer_head = pcur;
    
    // Writeback sector if changed
    if (pcur->dirty)
        if (!fatfs_fat_writeback(fs, pcur))
            return 0;
    
    // Address is now new sector
    pcur->address = sector;
    
    // Read next sector
    if (!fs->disk_io.read_media(pcur->address, pcur->sector, FAT_BUFFER_SECTORS))
    {
        // Read failed, invalidate buffer address
        pcur->address = FAT32_INVALID_CLUSTER;
        return NULL;
    }
    
    pcur->ptr = pcur->sector;
    return pcur;
}

uint32 fatfs_get_root_cluster(struct fatfs *fs)
{
    // NOTE: On FAT16 this will be 0 which has a special meaning...
    return fs->rootdir_first_cluster;
}

uint32 fatfs_find_next_cluster(struct fatfs *fs, uint32 current_cluster)
{
    uint32 fat_sector_offset, position;
    uint32 nextcluster;
    struct fat_buffer *pbuf;
    
    // Why is '..' labelled with cluster 0 when it should be 2 ??
    if (current_cluster == 0)
        current_cluster = 2;
    
    // Find which sector of FAT table to read
    if (fs->fat_type == FAT_TYPE_16)
        fat_sector_offset = current_cluster / 256;
    else
        fat_sector_offset = current_cluster / 128;
    
    // Read FAT sector into buffer
    pbuf = fatfs_fat_read_sector(fs, fs->fat_begin_lba+fat_sector_offset);
    if (!pbuf)
        return (FAT32_LAST_CLUSTER);
    
    if (fs->fat_type == FAT_TYPE_16)
    {
        // Find 32 bit entry of current sector relating to cluster number
        position = (current_cluster - (fat_sector_offset * 256)) * 2;
        
        // Read Next Clusters value from Sector Buffer
        nextcluster = FAT16_GET_16BIT_WORD(pbuf, (uint16)position);
        
        // If end of chain found
        if (nextcluster >= 0xFFF8 && nextcluster <= 0xFFFF)
            return (FAT32_LAST_CLUSTER);
    }
    else
    {
        // Find 32 bit entry of current sector relating to cluster number
        position = (current_cluster - (fat_sector_offset * 128)) * 4;
        
        // Read Next Clusters value from Sector Buffer
        nextcluster = FAT32_GET_32BIT_WORD(pbuf, (uint16)position);
        
        // Mask out MS 4 bits (its 28bit addressing)
        nextcluster = nextcluster & 0x0FFFFFFF;
        
        // If end of chain found
        if (nextcluster >= 0x0FFFFFF8 && nextcluster <= 0x0FFFFFFF)
            return (FAT32_LAST_CLUSTER);
    }
    
    // Else return next cluster
    return (nextcluster);
}

uint32 fatfs_lba_of_cluster(struct fatfs *fs, uint32 Cluster_Number)
{
    if (fs->fat_type == FAT_TYPE_16)
        return (fs->cluster_begin_lba + (fs->root_entry_count * 32 / FAT_SECTOR_SIZE) + ((Cluster_Number-2) * fs->sectors_per_cluster));
    else
        return ((fs->cluster_begin_lba + ((Cluster_Number-2)*fs->sectors_per_cluster)));
}

int fatfs_sector_reader(struct fatfs *fs, uint32 start_cluster, uint32 offset, uint8 *target)
{
    uint32 sector_to_read = 0;
    uint32 cluster_to_read = 0;
    uint32 cluster_chain = 0;
    uint32 i;
    uint32 lba;
    
    // FAT16 Root directory
    if (fs->fat_type == FAT_TYPE_16 && start_cluster == 0)
    {
        if (offset < fs->rootdir_sectors)
            lba = fs->lba_begin + fs->rootdir_first_sector + offset;
        else
            return 0;
    }
    // FAT16/32 Other
    else
    {
        // Set start of cluster chain to initial value
        cluster_chain = start_cluster;
        
        // Find parameters
        cluster_to_read = offset / fs->sectors_per_cluster;
        sector_to_read = offset - (cluster_to_read*fs->sectors_per_cluster);
        
        // Follow chain to find cluster to read
        for (i=0; i<cluster_to_read; i++)
            cluster_chain = fatfs_find_next_cluster(fs, cluster_chain);
        
        // If end of cluster chain then return false
        if (cluster_chain == FAT32_LAST_CLUSTER)
            return 0;
        
        // Calculate sector address
        lba = fatfs_lba_of_cluster(fs, cluster_chain)+sector_to_read;
    }
    
    // User provided target array
    if (target)
        return fs->disk_io.read_media(lba, target, 1);
    // Else read sector if not already loaded
    else if (lba != fs->currentsector.address)
    {
        fs->currentsector.address = lba;
        return fs->disk_io.read_media(fs->currentsector.address, fs->currentsector.sector, 1);
    }
    else
        return 1;
}

int fatfs_entry_sfn_only(struct fat_dir_entry *entry)
{
    if ( (entry->Attr!=FILE_ATTR_LFN_TEXT) &&
        (entry->Name[0]!=FILE_HEADER_BLANK) &&
        (entry->Name[0]!=FILE_HEADER_DELETED) &&
        (entry->Attr!=FILE_ATTR_VOLUME_ID) &&
        (!(entry->Attr&FILE_ATTR_SYSHID)) )
        return 1;
    else
        return 0;
}

int fatfs_entry_is_dir(struct fat_dir_entry *entry)
{
    if (entry->Attr & FILE_TYPE_DIR)
        return 1;
    else
        return 0;
}

struct lfn_cache
{
#if FATFS_INC_LFN_SUPPORT
    // Long File Name Structure (max 260 LFN length)
    uint8 String[MAX_LONGFILENAME_ENTRIES][MAX_LFN_ENTRY_LENGTH];
    uint8 Null;
#endif
    uint8 no_of_strings;
};

void fatfs_lfn_cache_init(struct lfn_cache *lfn, int wipeTable)
{
    int i = 0;
    
    lfn->no_of_strings = 0;
    
#if FATFS_INC_LFN_SUPPORT
    
    // Zero out buffer also
    if (wipeTable)
        for (i=0;i<MAX_LONGFILENAME_ENTRIES;i++)
            memset(lfn->String[i], 0x00, MAX_LFN_ENTRY_LENGTH);
#endif
}

int fatfs_entry_lfn_text(struct fat_dir_entry *entry)
{
    if ((entry->Attr & FILE_ATTR_LFN_TEXT) == FILE_ATTR_LFN_TEXT)
        return 1;
    else
        return 0;
}

void fatfs_lfn_cache_entry(struct lfn_cache *lfn, uint8 *entryBuffer)
{
    uint8 LFNIndex, i;
    LFNIndex = entryBuffer[0] & 0x1F;
    
    // Limit file name to cache size!
    if (LFNIndex > MAX_LONGFILENAME_ENTRIES)
        return ;
    
    // This is an error condition
    if (LFNIndex == 0)
        return ;
    
    if (lfn->no_of_strings == 0)
        lfn->no_of_strings = LFNIndex;
    
    lfn->String[LFNIndex-1][0] = entryBuffer[1];
    lfn->String[LFNIndex-1][1] = entryBuffer[3];
    lfn->String[LFNIndex-1][2] = entryBuffer[5];
    lfn->String[LFNIndex-1][3] = entryBuffer[7];
    lfn->String[LFNIndex-1][4] = entryBuffer[9];
    lfn->String[LFNIndex-1][5] = entryBuffer[0x0E];
    lfn->String[LFNIndex-1][6] = entryBuffer[0x10];
    lfn->String[LFNIndex-1][7] = entryBuffer[0x12];
    lfn->String[LFNIndex-1][8] = entryBuffer[0x14];
    lfn->String[LFNIndex-1][9] = entryBuffer[0x16];
    lfn->String[LFNIndex-1][10] = entryBuffer[0x18];
    lfn->String[LFNIndex-1][11] = entryBuffer[0x1C];
    lfn->String[LFNIndex-1][12] = entryBuffer[0x1E];
    
    for (i=0; i<MAX_LFN_ENTRY_LENGTH; i++)
        if (lfn->String[LFNIndex-1][i]==0xFF)
            lfn->String[LFNIndex-1][i] = 0x20; // Replace with spaces
}

int fatfs_entry_lfn_invalid(struct fat_dir_entry *entry)
{
    if ( (entry->Name[0]==FILE_HEADER_BLANK)  ||
        (entry->Name[0]==FILE_HEADER_DELETED)||
        (entry->Attr==FILE_ATTR_VOLUME_ID) ||
        (entry->Attr & FILE_ATTR_SYSHID) )
        return 1;
    else
        return 0;
}

int fatfs_entry_lfn_exists(struct lfn_cache *lfn, struct fat_dir_entry *entry)
{
    if ( (entry->Attr!=FILE_ATTR_LFN_TEXT) &&
        (entry->Name[0]!=FILE_HEADER_BLANK) &&
        (entry->Name[0]!=FILE_HEADER_DELETED) &&
        (entry->Attr!=FILE_ATTR_VOLUME_ID) &&
        (!(entry->Attr&FILE_ATTR_SYSHID)) &&
        (lfn->no_of_strings) )
        return 1;
    else
        return 0;
}

char* fatfs_lfn_cache_get(struct lfn_cache *lfn)
{
    // Null terminate long filename
    if (lfn->no_of_strings == MAX_LONGFILENAME_ENTRIES)
        lfn->Null = '\0';
    else if (lfn->no_of_strings)
        lfn->String[lfn->no_of_strings][0] = '\0';
    else
        lfn->String[0][0] = '\0';
    
    return (char*)&lfn->String[0][0];
}

uint32 fatfs_get_file_entry(struct fatfs *fs, uint32 Cluster, char *name_to_find, struct fat_dir_entry *sfEntry)
{
    uint8 item=0;
    uint16 recordoffset = 0;
    uint8 i=0;
    int x=0;
    char *long_filename = NULL;
    char short_filename[13];
    struct lfn_cache lfn;
    int dotRequired = 0;
    struct fat_dir_entry *directoryEntry;
    
    fatfs_lfn_cache_init(&lfn, 1);
    
    // Main cluster following loop
    while (1)
    {
        // Read sector
        if (fatfs_sector_reader(fs, Cluster, x++, 0)) // If sector read was successfull
        {
            // Analyse Sector
            for (item = 0; item < FAT_DIR_ENTRIES_PER_SECTOR; item++)
            {
                // Create the multiplier for sector access
                recordoffset = FAT_DIR_ENTRY_SIZE * item;
                
                // Overlay directory entry over buffer
                directoryEntry = (struct fat_dir_entry*)(fs->currentsector.sector+recordoffset);
                
#if FATFS_INC_LFN_SUPPORT
                // Long File Name Text Found
                if (fatfs_entry_lfn_text(directoryEntry) )
                    fatfs_lfn_cache_entry(&lfn, fs->currentsector.sector+recordoffset);
                
                // If Invalid record found delete any long file name information collated
                else if (fatfs_entry_lfn_invalid(directoryEntry) )
                    fatfs_lfn_cache_init(&lfn, 0);
                
                // Normal SFN Entry and Long text exists
                else if (fatfs_entry_lfn_exists(&lfn, directoryEntry) )
                {
                    long_filename = fatfs_lfn_cache_get(&lfn);
                    
                    // Compare names to see if they match
                    if (fatfs_compare_names(long_filename, name_to_find))
                    {
                        memcpy(sfEntry,directoryEntry,sizeof(struct fat_dir_entry));
                        return 1;
                    }
                    
                    fatfs_lfn_cache_init(&lfn, 0);
                }
                else
#endif
                    // Normal Entry, only 8.3 Text
                    if (fatfs_entry_sfn_only(directoryEntry) )
                    {
                        memset(short_filename, 0, sizeof(short_filename));
                        
                        // Copy name to string
                        for (i=0; i<8; i++)
                            short_filename[i] = directoryEntry->Name[i];
                        
                        // Extension
                        dotRequired = 0;
                        for (i=8; i<11; i++)
                        {
                            short_filename[i+1] = directoryEntry->Name[i];
                            if (directoryEntry->Name[i] != ' ')
                                dotRequired = 1;
                        }
                        
                        // Dot only required if extension present
                        if (dotRequired)
                        {
                            // If not . or .. entry
                            if (short_filename[0]!='.')
                                short_filename[8] = '.';
                            else
                                short_filename[8] = ' ';
                        }
                        else
                            short_filename[8] = ' ';
                        
                        // Compare names to see if they match
                        if (fatfs_compare_names(short_filename, name_to_find))
                        {
                            memcpy(sfEntry,directoryEntry,sizeof(struct fat_dir_entry));
                            return 1;
                        }
                        
                        fatfs_lfn_cache_init(&lfn, 0);
                    }
            } // End of if
        }
        else
            break;
    } // End of while loop
    
    return 0;
}


static int _open_directory(char *path, uint32 *pathCluster)
{
    int levels;
    int sublevel;
    char currentfolder[FATFS_MAX_LONG_FILENAME];
    struct fat_dir_entry sfEntry;
    uint32 startcluster;
    
    // Set starting cluster to root cluster
    startcluster = fatfs_get_root_cluster(&_fs);
    
    // Find number of levels
    levels = fatfs_total_path_levels(path);
    
    // Cycle through each level and get the start sector
    for (sublevel=0;sublevel<(levels+1);sublevel++)
    {
        if (fatfs_get_substring(path, sublevel, currentfolder, sizeof(currentfolder)) == -1)
            return 0;
        
        // Find clusteraddress for folder (currentfolder)
        if (fatfs_get_file_entry(&_fs, startcluster, currentfolder,&sfEntry))
        {
            // Check entry is folder
            if (fatfs_entry_is_dir(&sfEntry))
                startcluster = ((FAT_HTONS((uint32)sfEntry.FstClusHI))<<16) + FAT_HTONS(sfEntry.FstClusLO);
            else
                return 0;
        }
        else
            return 0;
    }
    
    *pathCluster = startcluster;
    return 1;
}

void fatfs_list_directory_start(struct fatfs *fs, struct fs_dir_list_status *dirls, uint32 StartCluster)
{
    dirls->cluster = StartCluster;
    dirls->sector = 0;
    dirls->offset = 0;
}

FL_DIR* fl_opendir(const char* path, FL_DIR *dir)
{
    int levels;
    int res = 1;
    uint32 cluster = FAT32_INVALID_CLUSTER;
    
    FL_LOCK(&_fs);
    
    levels = fatfs_total_path_levels((char*)path) + 1;
    
    // If path is in the root dir
    if (levels == 0)
        cluster = fatfs_get_root_cluster(&_fs);
    // Find parent directory start cluster
    else
        res = _open_directory((char*)path, &cluster);
    
    if (res)
        fatfs_list_directory_start(&_fs, dir, cluster);
    
    FL_UNLOCK(&_fs);
    
    return cluster != FAT32_INVALID_CLUSTER ? dir : 0;
}

int fatfs_list_directory_next(struct fatfs *fs, struct fs_dir_list_status *dirls, struct fs_dir_ent *entry)
{
    uint8 i,item;
    uint16 recordoffset;
    struct fat_dir_entry *directoryEntry;
    char *long_filename = NULL;
    char short_filename[13];
    struct lfn_cache lfn;
    int dotRequired = 0;
    int result = 0;
    
    // Initialise LFN cache first
    fatfs_lfn_cache_init(&lfn, 0);
    
    while (1)
    {
        // If data read OK
        if (fatfs_sector_reader(fs, dirls->cluster, dirls->sector, 0))
        {
            // Maximum of 16 directory entries
            for (item = dirls->offset; item < FAT_DIR_ENTRIES_PER_SECTOR; item++)
            {
                // Increase directory offset
                recordoffset = FAT_DIR_ENTRY_SIZE * item;
                
                // Overlay directory entry over buffer
                directoryEntry = (struct fat_dir_entry*)(fs->currentsector.sector+recordoffset);
                
#if FATFS_INC_LFN_SUPPORT
                // Long File Name Text Found
                if ( fatfs_entry_lfn_text(directoryEntry) )
                    fatfs_lfn_cache_entry(&lfn, fs->currentsector.sector+recordoffset);
                
                // If Invalid record found delete any long file name information collated
                else if ( fatfs_entry_lfn_invalid(directoryEntry) )
                    fatfs_lfn_cache_init(&lfn, 0);
                
                // Normal SFN Entry and Long text exists
                else if (fatfs_entry_lfn_exists(&lfn, directoryEntry) )
                {
                    // Get text
                    long_filename = fatfs_lfn_cache_get(&lfn);
                    strncpy(entry->filename, long_filename, FATFS_MAX_LONG_FILENAME-1);
                    
                    if (fatfs_entry_is_dir(directoryEntry))
                        entry->is_dir = 1;
                    else
                        entry->is_dir = 0;
                    
#if FATFS_INC_TIME_DATE_SUPPORT
                    // Get time / dates
                    entry->create_time = ((uint16)directoryEntry->CrtTime[1] << 8) | directoryEntry->CrtTime[0];
                    entry->create_date = ((uint16)directoryEntry->CrtDate[1] << 8) | directoryEntry->CrtDate[0];
                    entry->access_date = ((uint16)directoryEntry->LstAccDate[1] << 8) | directoryEntry->LstAccDate[0];
                    entry->write_time  = ((uint16)directoryEntry->WrtTime[1] << 8) | directoryEntry->WrtTime[0];
                    entry->write_date  = ((uint16)directoryEntry->WrtDate[1] << 8) | directoryEntry->WrtDate[0];
#endif
                    
                    entry->size = FAT_HTONL(directoryEntry->FileSize);
                    entry->cluster = (FAT_HTONS(directoryEntry->FstClusHI)<<16) | FAT_HTONS(directoryEntry->FstClusLO);
                    
                    // Next starting position
                    dirls->offset = item + 1;
                    result = 1;
                    return 1;
                }
                // Normal Entry, only 8.3 Text
                else
#endif
                    if ( fatfs_entry_sfn_only(directoryEntry) )
                    {
                        fatfs_lfn_cache_init(&lfn, 0);
                        
                        memset(short_filename, 0, sizeof(short_filename));
                        
                        // Copy name to string
                        for (i=0; i<8; i++)
                            short_filename[i] = directoryEntry->Name[i];
                        
                        // Extension
                        dotRequired = 0;
                        for (i=8; i<11; i++)
                        {
                            short_filename[i+1] = directoryEntry->Name[i];
                            if (directoryEntry->Name[i] != ' ')
                                dotRequired = 1;
                        }
                        
                        // Dot only required if extension present
                        if (dotRequired)
                        {
                            // If not . or .. entry
                            if (short_filename[0]!='.')
                                short_filename[8] = '.';
                            else
                                short_filename[8] = ' ';
                        }
                        else
                            short_filename[8] = ' ';
                        
                        fatfs_get_sfn_display_name(entry->filename, short_filename);
                        
                        if (fatfs_entry_is_dir(directoryEntry))
                            entry->is_dir = 1;
                        else
                            entry->is_dir = 0;
                        
#if FATFS_INC_TIME_DATE_SUPPORT
                        // Get time / dates
                        entry->create_time = ((uint16)directoryEntry->CrtTime[1] << 8) | directoryEntry->CrtTime[0];
                        entry->create_date = ((uint16)directoryEntry->CrtDate[1] << 8) | directoryEntry->CrtDate[0];
                        entry->access_date = ((uint16)directoryEntry->LstAccDate[1] << 8) | directoryEntry->LstAccDate[0];
                        entry->write_time  = ((uint16)directoryEntry->WrtTime[1] << 8) | directoryEntry->WrtTime[0];
                        entry->write_date  = ((uint16)directoryEntry->WrtDate[1] << 8) | directoryEntry->WrtDate[0];
#endif
                        
                        entry->size = FAT_HTONL(directoryEntry->FileSize);
                        entry->cluster = (FAT_HTONS(directoryEntry->FstClusHI)<<16) | FAT_HTONS(directoryEntry->FstClusLO);
                        
                        // Next starting position
                        dirls->offset = item + 1;
                        result = 1;
                        return 1;
                    }
            }// end of for
            
            // If reached end of the dir move onto next sector
            dirls->sector++;
            dirls->offset = 0;
        }
        else
            break;
    }
    
    return result;
}

int fl_readdir(FL_DIR *dirls, fl_dirent *entry)
{
    int res = 0;
    
    FL_LOCK(&_fs);
    
    res = fatfs_list_directory_next(&_fs, dirls, entry);
    
    FL_UNLOCK(&_fs);
    
    return res ? 0 : -1;
}

void fl_listdirectory(const char *path)
{
    FL_DIR dirstat;
    
    FL_LOCK(&_fs);
    
    FAT_PRINTF(("\r\nDirectory %s\r\n", path));
    
    if (fl_opendir(path, &dirstat))
    {
        struct fs_dir_ent dirent;
        
        while (fl_readdir(&dirstat, &dirent) == 0)
        {
#if FATFS_INC_TIME_DATE_SUPPORT
            int d,m,y,h,mn,s;
            fatfs_convert_from_fat_time(dirent.write_time, &h,&m,&s);
            fatfs_convert_from_fat_date(dirent.write_date, &d,&mn,&y);
            FAT_PRINTF(("%02d/%02d/%04d  %02d:%02d      ", d,mn,y,h,m));
#endif
            
            if (dirent.is_dir)
            {
                FAT_PRINTF(("%s <DIR>\r\n", dirent.filename));
            }
            else
            {
                FAT_PRINTF(("%s [%d bytes]\r\n", dirent.filename, dirent.size));
            }
        }
    }
    
    FL_UNLOCK(&_fs);
}

#endif /* header_h */
