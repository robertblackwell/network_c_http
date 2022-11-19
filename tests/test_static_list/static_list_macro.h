


#define MYTYPE DummyObj
#define MYPREF dummy_obj

#define LIST_STRUCT MYTYPE_StaticList_s
#define LIST_TYPE   MYTYPE_StaticList
#define LIST_REF    MYTYPE_StaticListRef
typedef struct LIST_STRUCT {
    int       capacity;
    int       head;
    int       tail_plus;
    MYTYPE* list[100];
} LIST_TYPE;
typedef LIST_TYPE * LIST_REF;

LIST_REF MYPREF_static_list_new(int capacity)
{
    LIST_REF st = malloc(sizeof(LIST_REF));
    st->capacity = capacity;
    st->head = 0;
    st->tail_plus = 0;
    for(int i = 0; i < capacity; i++) {
        st->list[i] = malloc(sizeof(DummyObj));
    }
    return st;
}
void MYPREF_static_list_add(LIST_REF lstref, MYTYPE dobj)
{
    lstref->head = (lstref->head + 1) % lstref->capacity;
    *(lstref->list[lstref->head]) = dobj;
}
MYTYPE MYPREF_static_list_remove(LIST_REF  lstref)
{
    int tmpix = (lstref->tail_plus + 1) % lstref->capacity;
    lstref->tail_plus = tmpix;
    return *(lstref->list[tmpix]);
}
int MYPREF_static_list_size(LIST_REF lstref)
{
    return (lstref->head + lstref->capacity - lstref->tail_plus) % lstref->capacity;
}

