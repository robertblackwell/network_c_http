/**
 * The nodes in our stack.
 */
struct TYPED(StackNode)
{
    TYPE val;                    // The value at the top of the stack
    struct TYPED(StackNode) *next;     // The rest of the values in the stack.
};
/**
 * The stack itself.
 */
typedef struct TYPED(Stack)
{
    int size;                   // How many elements are in the stack?
    struct TYPED(StackNode) *top;      // What's at the top of the stack?
} Stack;


TYPED(Stack) *
TYPED(stack_new) (void)
{
    struct TYPED(Stack) *new_stack = (struct TYPED(Stack) *) malloc (sizeof (struct TYPED(Stack)));
    new_stack->size = 0;
    new_stack->top = NULL;
    return new_stack;
} // stack_new

void
TYPED(stack_free) (TYPED(Stack) *stack)
{
    // Free all the nodes in the stack
    struct TYPED(StackNode) *tmp;
    while ((tmp = stack->top) != NULL)
    {
        stack->top = tmp->next;
        TYPED(free) (tmp);
    } // while
    // Free the stack itself
    TYPED(free) (stack);
} // stack_free


int
TYPED(stack_size) (TYPED(Stack) *stack)
{
    return stack->size;
} // stack_size
#if 0
int
stack_top (Stack *stack)
{
    return stack->top->val;
} // stack_top

int
stack_pop (Stack *stack)
{
    // Remember the top node and its value
    struct StackNode *tmp = stack->top;
    int top = tmp->val;
    // Drop that node
    stack->top = stack->top->next;
    free (tmp);
    --stack->size;
    // And return the saved value
    return top;
} // stack_pop

void
stack_push (Stack *stack, int val) {
    struct StackNode *new_top =
            (struct StackNode *) malloc(sizeof(struct StackNode));
    new_top->val = val;
    new_top->next = stack->top->next;
    stack->top = new_top;
}
#endif