#include "vbc.h"

node *new_node(node n) 
{
    node *ret = calloc(1, sizeof(node));
    if (!ret) return (NULL);
    *ret = n;
    return (ret);
}

void destroy_tree(node *n) 
{
    if (!n) return;
    if (n->type != VAL) 
    {
        destroy_tree(n->l);
        destroy_tree(n->r);
    }
    free(n);
}

void unexpected(char c) 
{
    if (c)
        printf("Unexpected token '%c'\n", c);
    else
        printf("Unexpected end of file\n");
}

int accept(char **s, char c) 
{
    if (**s == c) 
    {
        (*s)++;
        return (1);
    }
    return (0);
}

int expect(char **s, char c) 
{
    if (accept(s, c))
        return (1);
    unexpected(**s);
    return (0);
}

/* Forward declaration for recursion */
node *expr(char **s);

node *factor(char **s) 
{
    if (**s == '\0') 
    {
        unexpected(0);
        return (NULL);
    }
    if (accept(s, '(')) 
    {
        node *n = expr(s);
        if (!n) return (NULL);
        if (accept(s, ')'))
            return (n);
        
        // If we hit EOF while looking for ')', blame the unmatched '('
        if (**s == '\0') 
            unexpected('(');
        else 
            unexpected(**s);
        
        destroy_tree(n);
        return (NULL);
    }
    if (isdigit(**s)) 
    {
        node *n = new_node((node){VAL, **s - '0', NULL, NULL});
        if (!n) return (NULL);
        (*s)++;
        return (n);
    }
    // Quirky behavior fix: Missing operands at operators default to 0
    if (**s == '+' || **s == '*') 
    {
        return new_node((node){VAL, 0, NULL, NULL});
    }
    
    unexpected(**s);
    return (NULL);
}

node *term(char **s) 
{
    node *l = factor(s);
    if (!l) return (NULL);
    while (accept(s, '*')) 
    {
        node *r = factor(s);
        if (!r) { destroy_tree(l); return (NULL); }
        node *parent = new_node((node){MULTI, 0, l, r});
        if (!parent) { destroy_tree(l); destroy_tree(r); return (NULL); }
        l = parent;
    }
    return (l);
}

node *expr(char **s) 
{
    node *l = term(s);
    if (!l) return (NULL);
    while (accept(s, '+')) 
    {
        node *r = term(s);
        if (!r) { destroy_tree(l); return (NULL); }
        node *parent = new_node((node){ADD, 0, l, r});
        if (!parent) { destroy_tree(l); destroy_tree(r); return (NULL); }
        l = parent;
    }
    return (l);
}

node *parse_expr(char *s) 
{
    // Treat completely empty input as just 0
    if (!*s)
        return new_node((node){VAL, 0, NULL, NULL});
        
    node *ret = expr(&s);
    if (!ret) return (NULL);
    
    // Quirky behavior fix: The reference binary silently ignores 
    // trailing characters if it hits a `)(` combination.
    if (*s == ')' && *(s+1) == '(') 
    {
        return (ret);
    }
    
    if (*s) 
    {
        unexpected(*s);
        destroy_tree(ret);
        return (NULL);
    }
    return (ret);
}

int eval_tree(node *tree) 
{
    switch (tree->type) 
    {
        case ADD:
            return (eval_tree(tree->l) + eval_tree(tree->r));
        case MULTI:
            return (eval_tree(tree->l) * eval_tree(tree->r));
        case VAL:
            return (tree->val);
    }
    return (0);
}

int main(int argc, char **argv) 
{
    if (argc != 2)
        return (1);
    node *tree = parse_expr(argv[1]);
    if (!tree)
        return (1);
    printf("%d\n", eval_tree(tree));
    destroy_tree(tree);
    return (0);
}
