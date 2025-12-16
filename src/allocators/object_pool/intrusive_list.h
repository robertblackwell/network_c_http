#ifndef c_http_object_pool_intrusive_list_h
#define c_http_object_pool_intrusive_list_h
#include <rbl/check_tag.h>
struct MBlockList_s;
typedef struct MBlockList_s MBlockList, *MBlockListRef;

#define IDList_TAG "IDLIST"

typedef struct IntrusiveDoubleList_s IntrusiveDoubleList;
typedef struct IDLNode_s IDLNode;
typedef struct IDLNode_s
{
    RBL_DECLARE_TAG
    IDLNode* forward;
    IDLNode* backward;
} IDLNode;
typedef struct IntrusiveDoubleList_s IntrusiveDoubleList;
struct IntrusiveDoubleList_s
{
    RBL_DECLARE_TAG;
    IDLNode* head;
    IDLNode* tail;
    size_t count;
    size_t link_offset;
    RBL_DECLARE_END_TAG;
};

void op_intrusive_double_list_init(IntrusiveDoubleList* list, size_t link_offset);
void op_intrusive_double_list_deinit(const IntrusiveDoubleList* list);
void op_intrusive_double_list_display(IntrusiveDoubleList* list);
size_t op_intrusive_double_list_size(const IntrusiveDoubleList* list);
void op_intrusive_double_list_add(IntrusiveDoubleList* list, IDLNode* block);
void op_intrusive_double_list_remove(IntrusiveDoubleList* list, IDLNode* block);
void* op_intrusive_double_list_find(const IntrusiveDoubleList* list, const IDLNode* needle);
#endif