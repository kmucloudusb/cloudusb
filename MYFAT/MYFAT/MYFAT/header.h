#ifndef header_h
#define header_h

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

#endif /* header_h */
