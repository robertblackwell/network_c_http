#define STACK_STRUCT(tag, item_type) \
struct tag { \
    size_t nitems, limit; \
    item_type *items; \
};

#define STACK_PUSH(tag, item_type) \
bool tag ## _push(struct tag *const stack, item_type item) \
{ \
if (stack->nitems >= stack->limit) { \
return false; \
} \
stack->items[stack->nitems++] = item; \
return true; \
}

#define STACK_POP(tag, item_type) \
item_type tag ## _pop(struct tag *const stack) \
{ \
assert(stack->nitems > 0); \
return stack->items[--stack->nitems]; \
}

// etc.

#define STACK_DECLARE(tag, item_type) \
STACK_STRUCT(tag, item_type) \
STACK_INIT(tag, item_type) \
STACK_TERM(tag, item_type) \
STACK_PUSH(tag, item_type) \
STACK_POP(tag, item_type) \
STACK_IS_EMPTY(tag, item_type)