#include "http_header_line.h"

#include <ctype.h>
#include <string.h>
#include <src/common/utils.h>

#define HeaderLine_TAG "HDR_TAG"

HeaderLine* header_line_new(Cbuffer* key, Cbuffer* value)
{
    HeaderLine* tmp = malloc(sizeof(HeaderLine));
    header_line_init(tmp, key, value);
    return tmp;
}
HeaderLine* header_line_from_buffer(char* key, int keylen, char* value, int valuelen)
{
    CbufferRef k = Cbuffer_new();
    Cbuffer_append(k, key, keylen);
    CbufferRef v = Cbuffer_new();
    Cbuffer_append(v, value, valuelen);
    HeaderLine* tmp = header_line_new(k, v);
    return tmp;
}
HeaderLine* header_line_from_cstr(char* keycstr, char* valuecstr)
{
    CbufferRef k = Cbuffer_new();
    Cbuffer_append_cstr(k, keycstr);
    CbufferRef v = Cbuffer_new();
    Cbuffer_append_cstr(v, valuecstr);
    HeaderLine* tmp = header_line_new(k, v);
    return tmp;
}
void header_line_free(HeaderLine* hdr)
{
    header_line_deinit(hdr);
    free(hdr);
}
void inplace_toupper(char* cstr)
{
    for (char* p = cstr; *p != '\0'; ++p) {
        *p = toupper(*p);
    }
}
void header_line_init(HeaderLine* hdrline, Cbuffer* key, Cbuffer* value)
{
    RBL_SET_TAG(HeaderLine_TAG, hdrline)
    RBL_SET_END_TAG(HeaderLine_TAG, hdrline)
    assert(sizeof(void*) == sizeof(size_t));
    inplace_toupper((char*)Cbuffer_cstr(key));
    hdrline->key = key;
    hdrline->value = value;
    hdrline->backward = NULL;
    hdrline->forward = NULL;
}
void header_line_deinit(HeaderLine* hdr)
{
    RBL_SET_TAG(HeaderLine_TAG, hdr)
    RBL_SET_END_TAG(HeaderLine_TAG, hdr)
    Cbuffer_free(hdr->key);
    Cbuffer_free(hdr->value);
}
void header_line_append_key(HeaderLinePtr hline, char* buf, int len)
{
    for (int i = 0; i < len; ++i) {
        char ch = toupper(buf[i]);
        Cbuffer_append(hline->key, &ch, 1);
    }
}
void header_line_append_value(HeaderLinePtr hline, char* buf, int len)
{
    Cbuffer_append(hline->value, buf, len);
}

void header_line_set_value(HeaderLinePtr hline, Cbuffer* value)
{
    CbufferRef tmp = hline->value;
    hline->value = value;
    Cbuffer_free(tmp);
}

#if 0
HeaderListPtr header_list_from_array(const char* ar[][2])
{
    HeaderListPtr tmp = header_list_new();
    const char* k;
    const char* v;
    for(int row = 0; ar[row][0] != NULL ; row++) {
        k = ar[row][0];
        v = ar[row][1];
        HeaderLinePtr hl = header_line_from_cstr(k, v);
        header_list_add_back(hl);
    }
    return tmp;
}
void header_list_add_arr(HeaderListPtr this, const char* ar[][2])
{
    const char* k;
    const char* v;
    for(int row = 0; ar[row][0] != NULL ; row++) {
        k = ar[row][0];
        v = ar[row][1];
        header_list_add_cstr(this, k, v);
    }

}

HeaderListIter header_list_find_iter(const HeaderListPtr hlref, const char *key)
{
    HeaderListIter result = NULL;
    char *fixed_key = make_upper(key);
    HeaderListIter iter = header_list_iterator(hlref);
    while(iter) {
        HeaderLinePtr hlr = header_list_itr_unpack(hlref, iter);
        char *k = KVPair_label(hlr);
        if(strcmp(k, fixed_key) == 0) {
            result = iter;
            break;
        }
        iter = header_list_itr_next(hlref, iter);
    }
    if(fixed_key != NULL) { free(fixed_key); }
    return result;
}

HeaderLinePtr header_list_find(const HeaderListPtr hlref, const char *key)
{
    HeaderListIter iter = header_list_find_iter(hlref, key);
    if(iter == NULL) {
        return NULL;
    } else {
        HeaderLinePtr hlr = header_list_itr_unpack(hlref, iter);
        return hlr;
    }
}

void header_list_remove(HeaderListPtr hlref, const char *key)
{
    HeaderListIter iter = header_list_find_iter(hlref, key);
    if(iter == NULL) {
        return;
    } else {
        header_list_itr_remove(hlref, &iter);
    }

}

void header_list_add_cbuf(HeaderListPtr this, const CbufferRef key, const CbufferRef value)
{
    char *labptr = Cbuffer_data(key);
    int lablen = Cbuffer_size(key);
    char *valptr = Cbuffer_data(value);
    int vallen = Cbuffer_size(value);
    HeaderLinePtr hl = KVPair_new(labptr, lablen, valptr, vallen);
    header_list_add_back(this, hl);
}

void header_list_add_line(HeaderListPtr this, const char *label, int lablen, const char *value, int vallen)
{
    CbufferRef key = Cbuffer_from_buffer(label, lablen);
    CbufferRef v   = Cbuffer_from_buffer(value, vallen);
    HeaderLinePtr hl_content_type = KVPair_new(label, lablen, value, vallen);
    header_list_add_front(this, hl_content_type);
}

void header_list_add_cstr(HeaderListPtr this, const char *label, const char *value)
{
    int lablen = strlen(label);
    int vallen = strlen(value);
    HeaderLinePtr hl_content_type = KVPair_new(label, lablen, value, vallen);
    header_list_add_back(this, hl_content_type);
}

void header_list_add_many(HeaderListPtr this, CStrPair *pairs[])
{
    for(int i = 0; pairs[i] != NULL; i++) {

    }
}

// just to see it update
CbufferRef header_list_serialize(HeaderListPtr this)
{
    CbufferRef cb = Cbuffer_new();
    ListIterator iter = header_list_iterator(this);
    while(iter != NULL) {
        HeaderLinePtr line = header_list_itr_unpack(this, iter);
        Cbuffer_append_cstr(cb, KVPair_label(line));
        Cbuffer_append_cstr(cb, ": ");
        Cbuffer_append_cstr(cb, KVPair_value(line));
        Cbuffer_append_cstr(cb, "\r\n");
        iter = header_list_itr_next(this, iter);
    }
    return cb;
}
// create and initialize
HeaderListPtr header_list_new()
{
    HeaderListPtr lref = malloc(sizeof(HeaderList));
    if(lref != NULL) {
        header_list_init(lref);
    }
    return lref;
}

// initialize a given block of memory as empty list
void header_list_init(HeaderListPtr lref)
{
    ASSERT_NOT_NULL(lref);
    lref->count = 0;
    lref->head = NULL;
    lref->tail = NULL;
}

void header_list_free(HeaderListPtr lref)
{
    ASSERT_NOT_NULL(lref);
    assert(lref->count == 0);
    free(lref);
}
int header_list_size(const HeaderListPtr lref)
{
    return lref->count;
}
void header_list_display(const HeaderListPtr this)
{
    printf("List[%p] count: %d head %p tail %p\n", (void*)this, this->count, (void*)this->head, (void*)this->tail);
    HeaderLine* iter = this->head;
    while(iter != NULL) {
        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", (void*)iter, (void*)iter->forward, (void*)iter->backward, iter->item, (long)iter->item);
        HeaderLine* next = iter->forward;
        iter = next;
    }
}
HeaderLinePtr header_list_find_cbuffer(HeaderListPtr hlp, Cbuffer* needlekey)
{
//    printf("List[%p] count: %d head %p tail %p\n", (void*)this, this->count, (void*)this->head, (void*)this->tail);
    HeaderLine* iter = hlp->head;
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
void header_list_add_front(HeaderListPtr lref, HeaderLinePtr line)
{
    ASSERT_NOT_NULL(line);
    assert(line->backward == NULL);
    assert(line->forward == NULL);
    if(lref->count == 0) {
        lref->head = line;
        lref->tail = line;
        lref->count++;
    } else {
        line->forward = lref->head;
        line->backward = NULL;
        lref->head->backward = line;
        lref->head = line;
        lref->count++;
    }
}

// add to the back of the list
void header_list_add_back(HeaderListPtr lref, HeaderLinePtr line)
{
    ASSERT_NOT_NULL(line);
    assert(line->backward == NULL);
    assert(line->forward == NULL);
    if(lref->count == 0) {
        lref->tail = line;
        lref->head = line;
        lref->count++;
    } else {
        line->backward = lref->tail;
        line->forward = NULL;
        lref->tail->forward = line;
        lref->tail = line;
        lref->count++;
    }
}

// gets the item contained in the first list item without removing from list
HeaderLinePtr header_list_first(const HeaderListPtr lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->head == NULL)
        return NULL;
    return lref->head;
}

// gets the item contained in the first list item AND removes that item
HeaderLinePtr header_list_remove_first(HeaderListPtr lref)
{
    assert(lref != NULL);
    if(lref->count == 0)
        return NULL;
    if(lref->count == 1) {
        lref->count--;
        HeaderLinePtr content = lref->head;
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    HeaderLine* first = lref->head;
    lref->head = first->forward;
    lref->head->backward = NULL;
    first->forward = NULL;
    first->backward = NULL;
    lref->count--;
    return first;
}

// gets the item contained in the last list item without removing from list
HeaderListPtr header_list_last(const HeaderListPtr lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->tail == NULL) return NULL;
    return lref->tail;
}

// gets the item contained in the last list item AND removes that item
HeaderLinePtr header_list_remove_last(HeaderListPtr lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->count == 0 ) {
        return NULL;
    }
    if(lref->count == 1) {
        lref->count--;
        void* content = lref->head;
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    HeaderLine* last = lref->tail;
    lref->tail = last->backward;
    lref->tail->forward = NULL;
    last->forward = NULL;
    last->backward = NULL;
    lref->count--;
    return last;
}

//gets an iterator for the list which initially will be pointing at the first Node in the list
HeaderListIter header_list_iterator(const HeaderListPtr lref)
{
    ASSERT_NOT_NULL(lref);
    return lref->head;
}

// moves the iterator on to the next Node on the list, returns NULL if goes off the end of the list
HeaderListIter header_list_itr_next(const HeaderListPtr lref, const HeaderListIter itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->forward;
}
// removes a list item pointed at by an iterator - invalidates the itr
// and if there is a dealloc function call it on the content of the list node
void header_list_itr_remove(HeaderListPtr lref, HeaderListIter* itr_ptr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr_ptr);
    HeaderLine* itr = *itr_ptr;
    ASSERT_NOT_NULL(itr);
    if(lref->count == 0)
        return;
    else if(lref->count == 1 ) {
        assert(*itr_ptr == lref->head);
        lref->count = 0;
        lref->head = NULL;
        lref->tail = NULL;
        return;
    }
    if(lref->head == *itr_ptr) {
        (*itr_ptr)->forward->backward = (*itr_ptr)->backward;
        lref->head = (*itr_ptr)->forward;
    } else if (lref->tail == *itr_ptr) {
        (*itr_ptr)->backward->forward = (*itr_ptr)->forward;
        lref->tail = (*itr_ptr)->backward;
    } else {
        (*itr_ptr)->forward->backward = (*itr_ptr)->backward;
        (*itr_ptr)->backward->forward = (*itr_ptr)->forward;
    }
    lref->count--;

    header_line_free(*itr_ptr);
}

// gets the value of the item held in the Node pointed at by this iterator
HeaderLinePtr header_list_itr_unpack(HeaderListPtr lref, HeaderListIter itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr;
}
#endif