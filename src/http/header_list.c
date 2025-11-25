#include "header_list.h"
#include <ctype.h>
#include <string.h>
#include <src/common/utils.h>
#define HeaderList_TAG "HDRLST"

HeaderListPtr header_list_from_array(const char* ar[][2])
{
    HeaderListPtr tmp = header_list_new();
    const char* k;
    const char* v;
    for(int row = 0; ar[row][0] != NULL ; row++) {
        k = ar[row][0];
        v = ar[row][1];
        HeaderLinePtr hl = header_line_from_cstr((char*)k, (char*)v);
        header_list_add_back(tmp, hl);
    }
    return tmp;
}
void header_list_add_arr(HeaderListPtr hlist, const char* ar[][2])
{
    const char* k;
    const char* v;
    for(int row = 0; ar[row][0] != NULL ; row++) {
        k = ar[row][0];
        v = ar[row][1];
        header_list_add_cstr(hlist, k, v);
    }
}
HeaderListIter header_list_find_iter(const HeaderListPtr hlist, const char* key)
{
    HeaderListIter result = NULL;
    char *fixed_key = make_upper(key);
    HeaderListIter iter = header_list_iterator(hlist);
    while(iter) {
        HeaderLinePtr hlr = header_list_itr_unpack(hlist, iter);
        const char* k = Cbuffer_cstr(iter->key);
        if(strcmp(k, fixed_key) == 0) {
            result = iter;
            break;
        }
        iter = header_list_itr_next(hlist, iter);
    }
    if(fixed_key != NULL) { free(fixed_key); }
    return result;
}
HeaderLinePtr header_list_find(const HeaderListPtr hlist, const char *key)
{
    HeaderListIter iter = header_list_find_iter(hlist, key);
    if(iter == NULL) {
        return NULL;
    }
    HeaderLinePtr hlr = header_list_itr_unpack(hlist, iter);
    return hlr;
}

void header_list_remove(HeaderListPtr hlist, const char *key)
{
    HeaderListIter iter = header_list_find_iter(hlist, key);
    if(iter == NULL) {
        return;
    }
    header_list_itr_remove(hlist, &iter);
}

void header_list_add_cbuf(HeaderListPtr hlist, const CbufferRef key, const CbufferRef value)
{
    HeaderLinePtr hl = header_line_new(key, value);
    header_list_add_back(hlist, hl);
}
void header_list_add_line(HeaderListPtr hlist, const char *label, int lablen, const char *value, int vallen)
{
    HeaderLinePtr hl = header_line_from_buffer((char*)label, lablen, (char*)value, vallen);
    header_list_add_front(hlist, hl);
}
void header_list_add_cstr(HeaderListPtr hlist, const char *label, const char *value)
{
    char* lab = (char*)label;
    char* v = (char*)value;
    int lablen = strlen(label);
    int vallen = strlen(value);
    HeaderLinePtr hl_content_type = header_line_from_cstr(lab, v);
    header_list_add_back(hlist, hl_content_type);
}
CbufferRef header_list_serialize(HeaderListPtr hlist)
{
    CbufferRef cb = Cbuffer_new();
    HeaderLinePtr iter = header_list_iterator(hlist);
    while(iter != NULL) {
        Cbuffer_append_cstr(cb, Cbuffer_cstr(iter->key));
        Cbuffer_append_cstr(cb, ": ");
        Cbuffer_append_cstr(cb, Cbuffer_cstr(iter->value));
        Cbuffer_append_cstr(cb, "\r\n");
        iter = header_list_itr_next(hlist, iter);
    }
    return cb;
}
HeaderListPtr header_list_new()
{
    HeaderListPtr hlist = malloc(sizeof(HeaderList));
    assert(hlist != NULL);
    header_list_init(hlist);
    return hlist;
}
void header_list_init(HeaderListPtr hlist)
{
    RBL_SET_TAG(HeaderList_TAG, hlist);
    RBL_SET_END_TAG(HeaderList_TAG, hlist)
    ASSERT_NOT_NULL(hlist);
    hlist->count = 0;
    hlist->head = NULL;
    hlist->tail = NULL;
}
void header_list_free(HeaderListPtr hlist)
{
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    ASSERT_NOT_NULL(hlist);
    while(hlist->count > 0) {
        HeaderLinePtr p = header_list_remove_first(hlist);
        header_line_free(p);
    }
    assert(hlist->count == 0);
    free(hlist);
}
int header_list_size(const HeaderListPtr hlist)
{
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    return hlist->count;
}
void header_list_display(const HeaderListPtr hlist)
{
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    printf("List[%p] count: %d head %p tail %p\n", (void*)hlist, hlist->count, (void*)hlist->head, (void*)hlist->tail);
    HeaderLine* iter = hlist->head;
    while(iter != NULL) {
        printf("Node[%p] forward:%p backwards:%p \n", (void*)iter, (void*)iter->forward, (void*)iter->backward);
        HeaderLine* next = iter->forward;
        iter = next;
    }
}
HeaderLinePtr header_list_find_cbuffer(HeaderListPtr hlist, Cbuffer* needlekey)
{
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    //    printf("List[%p] count: %d head %p tail %p\n", (void*)hlist, hlist->count, (void*)hlist->head, (void*)hlist->tail);
    HeaderLine* iter = hlist->head;
    while(iter != NULL) {
        if(Cbuffer_equal(iter->key, needlekey)) {
            return iter;
        }
//        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", (void*)iter, (void*)iter->forward, (void*)iter->backward, iter->item, (long)iter->item);
        HeaderLine* next = iter->forward;
        iter = next;
    }
    return NULL;
}

// add to the front of the list
void header_list_add_front(HeaderListPtr hlist, HeaderLinePtr line)
{
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    ASSERT_NOT_NULL(line);
    assert(line->backward == NULL);
    assert(line->forward == NULL);
    if(hlist->count == 0) {
        hlist->head = line;
        hlist->tail = line;
        hlist->count++;
    } else {
        line->forward = hlist->head;
        line->backward = NULL;
        hlist->head->backward = line;
        hlist->head = line;
        hlist->count++;
    }
}
void header_list_add_back(HeaderListPtr hlist, HeaderLinePtr line)
{
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    ASSERT_NOT_NULL(line);
    assert(line->backward == NULL);
    assert(line->forward == NULL);
    if(hlist->count == 0) {
        hlist->tail = line;
        hlist->head = line;
        hlist->count++;
    } else {
        line->backward = hlist->tail;
        line->forward = NULL;
        hlist->tail->forward = line;
        hlist->tail = line;
        hlist->count++;
    }
}
HeaderLinePtr header_list_first(const HeaderListPtr hlist)
{
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    ASSERT_NOT_NULL(hlist);
    if(hlist->head == NULL)
        return NULL;
    return hlist->head;
}
HeaderLinePtr header_list_remove_first(HeaderListPtr hlist)
{
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    assert(hlist != NULL);
    if(hlist->count == 0)
        return NULL;
    if(hlist->count == 1) {
        hlist->count--;
        HeaderLinePtr content = hlist->head;
        hlist->head = NULL; hlist->tail = NULL;
        return content;
    }
    HeaderLine* first = hlist->head;
    hlist->head = first->forward;
    hlist->head->backward = NULL;
    first->forward = NULL;
    first->backward = NULL;
    hlist->count--;
    return first;
}
HeaderLinePtr header_list_last(const HeaderListPtr hlist)
{
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    ASSERT_NOT_NULL(hlist);
    if(hlist->tail == NULL) return NULL;
    return hlist->tail;
}
HeaderLinePtr header_list_remove_last(HeaderListPtr hlist)
{
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    ASSERT_NOT_NULL(hlist);
    if(hlist->count == 0 ) {
        return NULL;
    }
    if(hlist->count == 1) {
        hlist->count--;
        void* content = hlist->head;
        hlist->head = NULL; hlist->tail = NULL;
        return content;
    }
    HeaderLine* last = hlist->tail;
    hlist->tail = last->backward;
    hlist->tail->forward = NULL;
    last->forward = NULL;
    last->backward = NULL;
    hlist->count--;
    return last;
}
HeaderListIter header_list_iterator(const HeaderListPtr hlist)
{
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    ASSERT_NOT_NULL(hlist);
    return hlist->head;
}
HeaderListIter header_list_itr_next(const HeaderListPtr hlist, const HeaderListIter itr)
{
    ASSERT_NOT_NULL(hlist);
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    ASSERT_NOT_NULL(itr);
    return itr->forward;
}
void header_list_itr_remove(HeaderListPtr hlist, HeaderListIter* itr_ptr)
{
    ASSERT_NOT_NULL(hlist);
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    ASSERT_NOT_NULL(itr_ptr);
    HeaderLine* itr = *itr_ptr;
    ASSERT_NOT_NULL(itr);
    if(hlist->count == 0) {
        return;
    }
    if(hlist->count == 1 ) {
        assert(*itr_ptr == hlist->head);
        hlist->count = 0;
        hlist->head = NULL;
        hlist->tail = NULL;
        return;
    }
    if(hlist->head == *itr_ptr) {
        (*itr_ptr)->forward->backward = (*itr_ptr)->backward;
        hlist->head = (*itr_ptr)->forward;
    } else if (hlist->tail == *itr_ptr) {
        (*itr_ptr)->backward->forward = (*itr_ptr)->forward;
        hlist->tail = (*itr_ptr)->backward;
    } else {
        (*itr_ptr)->forward->backward = (*itr_ptr)->backward;
        (*itr_ptr)->backward->forward = (*itr_ptr)->forward;
    }
    hlist->count--;

    header_line_free(*itr_ptr);
}
HeaderLinePtr header_list_itr_unpack(HeaderListPtr hlist, HeaderListIter itr)
{
    ASSERT_NOT_NULL(hlist);
    RBL_CHECK_TAG(HeaderList_TAG, hlist);
    RBL_CHECK_END_TAG(HeaderList_TAG, hlist);
    ASSERT_NOT_NULL(itr);
    return itr;
}
