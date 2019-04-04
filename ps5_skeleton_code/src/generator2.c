#include "vslc.h"

#define MIN(a,b) (((a)<(b)) ? (a):(b))
static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

// Used to calculate offset for local var address
int n_args;
// Used to get # of label for while and if statements;
int n_while = 0, n_if = 0;

static void
generate_stringtable ( void )
{
    /* These can be used to emit numbers, strings and a run-time
     * error msg. from main
     */
    puts ( ".section .rodata" );
    puts ( "intout: .string \"\%ld \"" );
    puts ( "strout: .string \"\%s \"" );
    puts ( "errout: .string \"Wrong number of arguments\"" );

    for (size_t i = 0; i < stringc; i++)
        printf ( "STR%i: .string %s\n", i, string_list[i] );


}

static void
generate_globals ( void )
{
    puts ( ".section .data" );

    size_t n_globals = tlhash_size ( global_names );
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );
    for ( size_t i = 0; i < n_globals; i++ )
        if ( global_list[i]->type == SYM_GLOBAL_VAR )
            printf ( "._%s: .zero 8\n", global_list[i]->name );
}

static void
generate_function ( symbol_t *fun )
{
    n_args = fun->nparms;
    printf ( "_%s:\n", fun->name );
    puts ( "\tpushq\t%rbp" );
    puts ( "\tmovq\t%rsp, %rbp" );
    // Push args not already on stack
    for (int i = 0; i < fun->nparms && i < 6; i++)
    {
        printf ( "\tpushq\t%s\n", record[i] );
    }
    // Reserve space for local variables
    if ((tlhash_size(fun->locals) - fun->nparms) != 0)
        printf ( "\tsubq\t$%i, %rsp\n", (tlhash_size(fun->locals) - fun->nparms)*8 );
   
    // Pad stack if No. locals is uneven
    if (tlhash_size(fun->locals)%2 == 1)
    {
        puts ( "\tpushq\t$0 /* Stack padding for 16-byte alignment */" );
    }
    traverse_subtree(fun->node);
}

static void
traverse_subtree ( node_t *node )
{
   switch(node->type)
   {
        case PRINT_STATEMENT:
          for (int i = 0; i < node->n_children; i++)
          {
              print_data(node->children[i]);
          }
            puts ( "\tmovq\t$0x0A, %rdi" );
            puts ( "\tcall\tputchar" );
          break;
        
        case ASSIGNMENT_STATEMENT:
            handle_expression(node->children[1]);
            printf ( "\tmovq\t%rax, %s\n", get_value(node->children[0]));
            break;

        case WHILE_STATEMENT:
            printf ( "WHILE%i:\n", n_while );
            handle_relation(node->children[0], "DONE", n_while);
            traverse_subtree(node->children[1]);
            printf ( "\tjmp\tWHILE%i\n", n_while );
            printf ( "DONE%i:\n", n_while );
            n_while++;
            break;

       case IF_STATEMENT:
            handle_relation(node->children[0], "ELSE", n_if);
            traverse_subtree(node->children[1]);
            if (node->n_children == 3)
            {
                printf ( "\tjmp\tENDIF%i\n", n_if );
                printf ( "ELSE%i:\n", n_if );
                traverse_subtree(node->children[2]);
            }
            printf ( "ENDIF%i:\n", n_if );
            n_if++;
            break;

        case NULL_STATEMENT:
            printf ( "\tjmp\tWHILE%i\n", n_while );
            break;

        case RETURN_STATEMENT:
            handle_expression(node->children[0]);
            puts ("\tleave\n\tret" );
            break;
          
        case EXPRESSION:
            handle_expression(node);
            break;

        default:
          for (int i = 0; i < node->n_children; i++)
              traverse_subtree(node->children[i]);
   }
}

static void
handle_relation(node_t *rel, char * label, int num)
{
	// Everything is swapped so > becomes > and not >=
    handle_expression(rel->children[0]);
    puts ( "\tmovq\t%rax, %rbx" );
    handle_expression(rel->children[1]); 
    puts ( "\tcmpq\trbx, %rax" );
    switch(*(int *)rel->data)
    {
        case '>':
            printf ( "\tjge\t%s%i\n", label, num );
            break;
        case '<':
            printf ( "\tjle\t%s%i\n", label, num );
            break;
        case '=':
            printf ( "\tjne\t%s%i\n", label, num );
            break;
    }            

}

static void
print_data ( node_t *data )
{
    switch(data->type)
    {

        case EXPRESSION:
            handle_expression(data);
            puts ( "\tmovq\t%rax, %rsi" );
            puts ( "\tmovq\t$intout, %rdi" );
            break;

        case IDENTIFIER_DATA:
            printf ( "\tmovq\t%s, %rsi\n", get_value(data) );
            puts ( "\tmovq\t$intout, %rdi" );
            break;
            
        case NUMBER_DATA:
            printf ( "\tmovq\t$%i, %rsi\n", *(int *)data->data );
            puts ( "\tmovq\t$intout, %rdi" );
            break;

        case STRING_DATA:
            printf ( "\tmovq\t$.STR%i, %rsi\n", *(int *)data->data );
            puts ( "\tmovq\t$strout, %rdi" );
            break;
    }
    puts ( "\tcall\t printf" );

}


/* evaluates expression: result left in %rax */
static void
handle_expression ( node_t *exp )
{
    if (exp->type == IDENTIFIER_DATA)
    {
        printf ( "\tmovq\t%s, %rax\n", get_value(exp) );
    }
    else if (exp->type == NUMBER_DATA)
    {
        printf ( "\tmovq\t$%s, %rax\n", get_value(exp) );
    }

    else if (exp->data == NULL)
    {
        // Iterate over all args (children of second child (expression list))
        if (exp->children[1] != NULL)
        {
            if (exp->children[1]->n_children > 6)
            {
                for (uint64_t i = exp->children[1]->n_children; i > 6;  i--)
                {
                    handle_expression(exp->children[1]->children[i-1]);
                    puts ( "\tpushq\t%rax" );
                }
                for (size_t i = 6; i > 0; i--)
                {
                    handle_expression(exp->children[1]->children[i-1]);
                    printf ( "\tmovq\t%rax, %s\n", record[i-1] );
                }
            }
            else
            {
                for (size_t i = exp->children[1]->n_children; i > 0; i--)
                {
                    handle_expression(exp->children[1]->children[i-1]);
                    printf ( "\tmovq\t%rax, %s\n", record[i-1] );
                }
            }
        }
        printf ( "\tcall _%s\n", exp->children[0]->data);
    }

    else
    {
        if (exp->n_children == 1)
        {
            handle_expression(exp->children[0]);
            puts ( "\tnotq\t%rax" );
        }
        else
        {
            // Hope you enjoy lots of arbitrary code 
            switch(*(int *)exp->data)
            {
                case '*':
                    puts ( "\tpushq\t%rdx" );
                    handle_expression(exp->children[1]);

                    puts ( "\tpushq\t%rax" );
                    handle_expression(exp->children[0]);
                    puts ( "\tmulq\t(%rsp)" );

                    // Alternate solution more in line with curriculum:
                    /* puts ( "\tcqo" ); */
                    /* puts ( "\timulq\t(%rsp)" );  */

                    puts ( "\tpopq\t%rdx" );
                    puts ( "\tpopq\t%rdx" );
                    break;

                case '/':
                    puts ( "\tpushq\t%rdx" );
                    handle_expression(exp->children[1]);

                    puts ( "\tpushq\t%rax" );
                    handle_expression(exp->children[0]);
                    puts ( "\tcqo" );
                    puts ( "\tidivq\t(%rsp)" );

                    puts ( "\tpopq\t%rdx" );
                    puts ( "\tpopq\t%rdx" );               
                    break;

                case '+':
                    handle_expression(exp->children[0]);
                    puts ( "\tpushq\t %rax" );
                    handle_expression(exp->children[1]);

                    puts ( "\taddq\t%rax, (%rsp)" );
                    puts ( "\tpopq\t%rax" );
                    break;

                case '-':
                    handle_expression(exp->children[0]);
                    puts ( "\tpushq\t%rax" );
                    handle_expression(exp->children[1]);

                    puts ( "\tsubq\t%rax, (%rsp)" );
                    puts ( "\tpopq\t%rax" );
                    break;

                case '|':
                    handle_expression(exp->children[0]);
                    puts ( "\tpushq\t%rax" );
                    handle_expression(exp->children[1]);

                    puts ( "\torq\t%rax, (%rsp)" );
                    puts ( "\tpopq\t%rax" );
                    break;

                case '^':
                    handle_expression(exp->children[0]);
                    puts ( "\tpushq\t%rax" );
                    handle_expression(exp->children[1]);

                    puts ( "\txorq\t%rax, (%rsp)" );
                    puts ( "\tpopq\t%rax" );
                    break;
 
                case '&':
                    handle_expression(exp->children[0]);
                    puts ( "\tpushq\t%rax" );
                    handle_expression(exp->children[1]);

                    puts ( "\tandq\t%rax, (%rsp)" );
                    puts ( "\tpopq\t%rax" );
                    break;

                default:
                    if (strcmp((char *)exp->data, "<<") == 0)
                    {
                        handle_expression(exp->children[0]);
                        puts ( "\tmovq\t%rax, %rcx" );
                        handle_expression(exp->children[1]);

                        puts ( "\txchgq\t%rax,%rcx" );
                        puts ( "\tsalq\t\%cl, %rax" );
                        break;
                    }
                    else
                    {
                        handle_expression(exp->children[0]);
                        puts ( "\tmovq\t%rax, %rcx" );
                        handle_expression(exp->children[1]);

                        puts ( "\txchgq\t%rax,%rcx" );
                        puts ( "\tsarq\t\%cl, %rax" );
                        break;
                    }
            }
        }
    }
}

static char*
get_value ( node_t *value )
{
    char *result = malloc(sizeof(char *));
    switch (value->type)
    {
        case IDENTIFIER_DATA:
            if (value->entry->type ==  SYM_GLOBAL_VAR)
            {
                snprintf (result, 64, "_%s", value->entry->name );
                return result;
            }
            if ( value->entry->type == SYM_LOCAL_VAR)
            {
                snprintf ( result, 64, "%i(%rbp)", -(value->entry->seq+n_args+1)*8 );
                return result;
            }
            if (value->entry->seq > 5)
            {
            snprintf ( result, 64, "%i(%rbp)", (value->entry->seq-4)*8 );
            return result;
            }
            snprintf ( result, 64, "%i(%rbp)", -(value->entry->seq+1)*8 );
            return result;


        case NUMBER_DATA:
            sprintf (result, "%i", *(int *)value->data);
            return result;
    }
}


static void
generate_main ( symbol_t *first )
{
    puts ( ".globl main" );
    puts ( ".section .text" );
    puts ( "main:" );
    puts ( "\tpushq %rbp" );
    puts ( "\tmovq %rsp, %rbp" );

    puts ( "\tsubq $1, %rdi" );
    printf ( "\tcmpq\t$%zu,%%rdi\n", first->nparms );
    puts ( "\tjne ABORT" );
    puts ( "\tcmpq $0, %rdi" );
    puts ( "\tjz SKIP_ARGS" );

    puts ( "\tmovq %rdi, %rcx" );
    printf ( "\taddq $%zu, %%rsi\n", 8*first->nparms );
    puts ( "PARSE_ARGV:" );
    puts ( "\tpushq %rcx" );
    puts ( "\tpushq %rsi" );

    puts ( "\tmovq (%rsi), %rdi" );
    puts ( "\tmovq $0, %rsi" );
    puts ( "\tmovq $10, %rdx" );
    puts ( "\tcall strtol" );

    /*  Now a new argument is an integer in rax */
    puts ( "\tpopq %rsi" );
    puts ( "\tpopq %rcx" );
    puts ( "\tpushq %rax" );
    puts ( "\tsubq $8, %rsi" );
    puts ( "\tloop PARSE_ARGV" );

    /* Now the arguments are in order on stack */
    for ( int arg=0; arg<MIN(6,first->nparms); arg++ )
        printf ( "\tpopq\t%s\n", record[arg] );

    puts ( "SKIP_ARGS:" );
    printf ( "\tcall\t_%s\n", first->name );
    puts ( "\tjmp END" );
    puts ( "ABORT:" );
    puts ( "\tmovq $errout, %rdi" );
    puts ( "\tcall puts" );

    puts ( "END:" );
    puts ( "\tmovq %rax, %rdi" );
    puts ( "\tcall exit" );
}


void
generate_program ( void )
{
    generate_stringtable();
    generate_globals();
    // Generate functions, calling main on first function in global hash.
    size_t n_globals = tlhash_size ( global_names );
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );
    bool main_called = false;
    for ( size_t i = 0; i < n_globals; i++ )
        if ( global_list[i]->type == SYM_FUNCTION )
        {
            if ( !main_called )
            {
                generate_main(global_list[i]);
                main_called = true;
            }
            generate_function(global_list[i]);
        }
}