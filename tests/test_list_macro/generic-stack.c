TYPED(Stack) *
TYPED(stack_new) (void)
{
    struct TYPED(Stack) *new_stack = (struct TYPED(Stack) *) malloc (sizeof (struct TYPED(Stack)));
    new_stack->size = 0;
    new_stack->top = NULL;
    return new_stack;
} // stack_new

void
TYPED(stack_free) (Stack *stack)
{
    // Free all the nodes in the stack
    struct TYPED(StackNode) *tmp;
    while ((tmp = stack->top) != NULL)
    {
        stack->top = tmp->next;
        free (tmp);
    } // while
    // Free the stack itself
    free (stack);
} // stack_free


int
TYPED(stack_size) (TYPED(Stack) *stack)
{
    return stack->size;
} // stack_size

int
TYPED(stack_top) (TYPED(Stack) *stack)
{
    return stack->top->val;
} // stack_top

int
TYPED(stack_pop) (TYPED(Stack) *stack)
{
    // Remember the top node and its value
    struct TYPED(StackNode) *tmp = stack->top;
    int top = tmp->val;
    // Drop that node
    stack->top = stack->top->next;
    free (tmp);
    --stack->size;
    // And return the saved value
    return top;
} // stack_pop

void
TYPED(stack_push) (TYPED(Stack) *stack, int val)
{
    struct TYPED(StackNode) *new_top =
            (struct TYPED(StackNode )*) malloc (sizeof (struct TYPED(StackNode));
    new_top->val = val;
    new_top->next = stack->top->next;
    stack->top = new_top;
} // stack_push