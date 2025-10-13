#define OBJECT_ALLOCATOR_HEADER(type_tag, func_tag, type, N)                  \
#define EVT_MAX N                                               \
                                                                \
typedef struct FreeList_s FreeList, *FreeListRef;               \
typedef struct MemorySlab_s MemorySlab, *MemorySlabRef;         \
                                                                \
struct FreeList_s {                                             \
    uint16_t    count;                                          \
    uint16_t    max_entries;                                    \
    uint16_t    modulo_max;                                     \
    uint16_t    rdix;                                           \
    uint16_t    wrix;                                           \
    uint16_t    buffer[EVT_MAX+1];                              \
};                                                              \
                                                                \
void freelist_init(FreeListRef fl);                             \
bool freelist_is_full(FreeListRef fl);                          \
bool freelist_is_empty(FreeListRef fl);                         \
void freelist_add(FreeListRef fl, uint16_t element);            \
uint16_t freelist_get(FreeListRef fl);                          \
size_t freelist_size(FreeListRef fl);                           \
                                                                \
                                                                \
struct MemorySlab_s {                                           \
    struct {                                                    \
        type  t;
        uint16_t    my_index;                                   \
    } m;                                                        \
};                                                              \
                                                                \
struct type_tag ## Table_s {                                           \
    FreeList    free_list;                                      \
    MemorySlab  memory[EVT_MAX+1];                              \
};                                                              \
type_tag ## TableRef func_tag ## _table_new();                                \
void func_tag ## _table_init(EventTableRef et);                        \
void* func_tag ## _table_get_entry(EventTableRef et);                  \
void func_tag ## _table_release_entry(EventTableRef et, void* p);      \
bool func_tag ## _table_has_outstanding_events(EventTableRef et);      \
size_t func_tag ## _table_number_in_use(EventTableRef et);             \
