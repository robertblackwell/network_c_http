#include <stdlib.h>
/**
 * The nodes in our stack.
 */
struct TYPED(StackNode_s)
{
    TYPE val;                    // The value at the top of the stack
    struct TYPED(StackNode_s) *next;     // The rest of the values in the stack.
};
typedef struct TYPED(StackNode_s) TYPED(StackNode);
/**
 * The stack itself.
 */
struct TYPED(Stack_s)
{
    int size;                   // How many elements are in the stack?
    TYPED(StackNode) *top;      // What's at the top of the stack?
};
typedef struct TYPED(Stack_s) TYPED(Stack);
typedef TYPED(Stack) * TYPED(StackRef);

TYPED(StackRef)
TYPED(Stack_new) (void)
{
    TYPED(Stack) *new_stack = (struct TYPED(Stack_s) *) malloc (sizeof (TYPED(Stack)));
    new_stack->size = 0;
    new_stack->top = NULL;
    return new_stack;
} // Stack_new

void
TYPED(Stack_free) (TYPED(Stack) *stack)
{
    // Free all the nodes in the stack
    TYPED(StackNode) *tmp;
    while ((tmp = stack->top) != NULL)
    {
        stack->top = tmp->next;
//        TYPED(StackNode_free) (tmp);
        free(tmp);
    } // while
    // Free the stack itself
    TYPED(Stack_free) (stack);
} // Stack_free


int
TYPED(Stack_size) (TYPED(Stack) *stack)
{
    return stack->size;
}
TYPE
TYPED(Stack_top) (TYPED(Stack) *stack)
{
    return stack->top->val;
}

TYPE
TYPED(Stack_pop) (TYPED(Stack) *stack)
{
    // Remember the top node and its value
    TYPED(StackNode) *tmp = stack->top;
    int top = tmp->val;
    // Drop that node
    stack->top = stack->top->next;
//    TYPED(free) (tmp);
    free(tmp);
    --stack->size;
    // And return the saved value
    return top;
} // Stack_pop

void
TYPED(Stack_push) (TYPED(Stack) *stack, TYPE val) {
    TYPED(StackNode) *new_top =
            (TYPED(StackNode) *) malloc(sizeof(TYPED(StackNode)));
    new_top->val = val;
    new_top->next = (stack->top) ? stack->top: NULL;
    stack->top = new_top;
    ++stack->size;
}
