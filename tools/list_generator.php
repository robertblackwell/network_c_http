<?php 

function hdrs($name, $node, $item)
{
$text=<<<EOD
#ifndef c_ceg_{$name}_h
#define c_ceg_{$name}_h

// type of data to be held in the list
typedef {$item} * ListItem;
struct {$name}_s;
typedef struct {$name}_s List, *ListRef;
struct {$node}_s;

typedef struct {$node}_s {$node};
typedef {$node}* {$name}_Iter, *ListIter;
typedef void(*ListItemDeallocator)({$item}**);
ListRef {$name}_new(ListItemDeallocator dealloc);
void {$name}_init(ListRef lref, ListItemDeallocator dealloc);
void {$name}_destroy(ListRef lref);
void {$name}_free(ListRef *lref_adr);
void {$name}_display(const ListRef this);
int {$name}_size(const ListRef lref);
void {$name}_add_back(ListRef lref, {$item}* item);
void {$name}_add_front(ListRef lref, {$item}* item);
{$item}* {$name}_first(const ListRef lref);
{$item}* {$name}_remove_first(ListRef lref);
{$item}* {$name}_last(const ListRef lref);
{$item}* {$name}_remove_last(ListRef lref);
{$name}_Iter {$name}_iterator(const ListRef lref);
{$name}_Iter {$name}_itr_next(const ListRef lref, const {$name}_Iter itr);
void {$name}_itr_remove(ListRef lref, {$name}_Iter *itr_adr);
{$item}* {$name}_itr_unpack(ListRef lref, {$name}_Iter itr);

#endif
EOD;
    return $text;
}


function impl($name, $node, $item) 
{

$text=<<<EOD

//opaque type representing list
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <src/alloc.h>
#include <src/list.h>
#include <src/utils.h>
//Internal - type used to build list

struct {$node}_s {
    {$node}* forward;
    {$node}* backward;
    {$item}* item;
};

struct {$name}_s {
    int count;
    {$node}* head;
    {$node}* tail;
    ListItemDeallocator dealloc;
};

{$node}* {$node}_new({$item}* content, {$node}* prev, {$node}* next)
{
    {$node}* lnref = eg_alloc(sizeof({$node}));
    lnref->item = content;
    lnref->forward = next;
    lnref->backward = prev;
}
void {$node}_free(ListRef lref, {$node}** lnref_ptr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(lnref_ptr);
    {$node}* lnref = *lnref_ptr;
    if(lref->dealloc != NULL) {
        if(lnref->item != NULL)
            lref->dealloc(&(lnref->item));
    }
    eg_free(({$item}*)*lnref_ptr);
    *lnref_ptr = NULL;
}


// create and initialize
ListRef {$name}_new(ListItemDeallocator dealloc)
{
    ListRef lref = eg_alloc(sizeof(List));
    if(lref != NULL) {
        {$name}_init(lref, dealloc);
    }
    return lref;
}

// initialize a given block of memory as empty list
void {$name}_init(ListRef lref, ListItemDeallocator dealloc)
{
    ASSERT_NOT_NULL(lref);
    lref->count = 0;
    lref->head = NULL;
    lref->tail = NULL;
    lref->dealloc = dealloc;
}

// destroy the content including freeing any dynamic memory leaving a functioning empty list
void {$name}_destroy(ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    ListItemDeallocator dealloc = lref->dealloc;
    {$node}* t = lref->head;
    for(;;) {
        // how to free the contained item
        if(t == NULL) {
            break;
        }
//        if(lref->dealloc != NULL) {
//            lref->dealloc(&(t->item));
//        }
        {$node}* tnext = t->forward;
        {$node}_free(lref, &t);
        t = tnext;
    }
    {$name}_init(lref, dealloc);
}

//free the entire list including invalidating the lref
void {$name}_free(ListRef* lref_ptr)
{
    ASSERT_NOT_NULL(*lref_ptr);
    {$name}_destroy(*lref_ptr);
    free(({$item}*)*lref_ptr);
    *lref_ptr = NULL;
}
int {$name}_size(const ListRef lref)
{
    return lref->count;
}
void {$name}_display(const ListRef this)
{
    printf("List[%p] count: %d head %p tail %p\n", ({$item}*)this, this->count, ({$item}*)this->head, ({$item}*)this->tail);
    {$node}* iter = this->head;
    while(iter != NULL) {
        printf("Node[%p] forward:%p backwards:%p  item:%p  %ld\n", ({$item}*)iter, ({$item}*)iter->forward, ({$item}*)iter->backward, iter->item, (long)iter->item);
        {$node}* next = iter->forward;
        iter = next;
    }
}
// add to the front of the list
void {$name}_add_front(ListRef lref, {$item}* content)
{
    ASSERT_NOT_NULL(lref);
    {$node}* lnref = {$node}_new(content, NULL, NULL);
    if(lref->count == 0) {
        lref->head = lnref;
        lref->tail = lnref;
        lref->count++;
    } else {
        lnref->forward = lref->head;
        lnref->backward = NULL;
        lref->head->backward = lnref;
        lref->head = lnref;
        lref->count++;
    }
}

// add to the back of the list
void {$name}_add_back(ListRef lref, {$item}* content)
{
    ASSERT_NOT_NULL(lref);
    {$node}* lnref = {$node}_new(content, NULL, lref->head);
    if(lref->count == 0) {
        lref->tail = lnref;
        lref->head = lnref;
        lref->count++;
    } else {
        lnref->backward = lref->tail;
        lnref->forward = NULL;
        lref->tail->forward = lnref;
        lref->tail = lnref;
        lref->count++;
    }
}

// gets the item contained in the first list item without removing from list
{$item}* {$name}_first(const ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->head == NULL)
        return NULL;
    return lref->head->item;
}

// gets the item contained in the first list item AND removes that item
{$item}* {$name}_remove_first(ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->count == 0)
        return NULL;
    if(lref->count == 1) {
        lref->count--;
        {$item}* content = lref->head->item;
        lref->head->item = NULL;
        {$node}_free(lref, &(lref->head));
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    {$node}* first = lref->head;
    lref->head = first->forward;
    lref->head->backward = NULL;
    {$item}* content = first->item;
    first->forward = NULL;
    first->backward = NULL;
    first->item = NULL;
    {$node}_free(lref, &first);
    lref->count--;
    return content;
}

// gets the item contained in the last list item without removing from list
{$item}* {$name}_last(const ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->tail == NULL) return NULL;
    return lref->tail->item;
}

// gets the item contained in the last list item AND removes that item
{$item}* {$name}_remove_last(ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    if(lref->count == 0 ) {
        return NULL;
    }
    if(lref->count == 1) {
        lref->count--;
        {$item}* content = lref->head->item;
        lref->head->item = NULL;

        {$node}_free(lref, &(lref->head));
        lref->head = NULL; lref->tail = NULL;
        return content;
    }
    {$node}* last = lref->tail;
    lref->tail = last->backward;
    lref->tail->forward = NULL;
    {$item}* content = last->item;
    last->forward = NULL;
    last->backward = NULL;
    last->item = NULL;

    {$node}_free(lref, &last);
    lref->count--;
    return content;
}

//gets an iterator for the list which initially will be pointing at the first Node in the list
{$name}_Iter {$name}_iterator(const ListRef lref)
{
    ASSERT_NOT_NULL(lref);
    return lref->head;
}

// moves the iterator on to the next Node on the list, returns NULL if goes off the end of the list
{$name}_Iter {$name}_itr_next(const ListRef lref, const {$name}_Iter itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->forward;
}

// removes a list item pointed at by an iterator - invalidates the itr
// and if there is a dealloc function call it on the content of the list node
void {$name}_itr_remove(ListRef lref, {$name}_Iter* itr_ptr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr_ptr);
    {$node}* itr = *itr_ptr;
    ASSERT_NOT_NULL(itr);
    if(lref->count == 0)
        return;
    else if(lref->count == 1 ) {
        assert(*itr_ptr == lref->head);
        lref->count = 0;
        lref->head = NULL;
        lref->tail = NULL;
        {$node}_free(lref, itr_ptr);
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

    {$node}_free(lref, itr_ptr);
}

// gets the value of the item held in the Node pointed at by this iterator
{$item}* {$name}_itr_unpack(ListRef lref, {$name}_Iter itr)
{
    ASSERT_NOT_NULL(lref);
    ASSERT_NOT_NULL(itr);
    return itr->item;
}

EOD;

    return $text;
}
print(hdrs("aaname", "aanode", "aaitem"));
print (impl("aaname", "aanode", "aaitem"));
