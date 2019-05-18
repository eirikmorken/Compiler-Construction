#include "vslc.h"
#include "generator.h"

#define MIN(a,b) (((a)<(b)) ? (a):(b))
static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

int n_if_statements = 0, n_while_loops = 0;
size_t size_parameters = 0;

/*Implement the following constructs in generator.c:
1. Local variables 20%
2. Function calls 20%
3. Conditionals (IF and relations) 15%
4. While loops 10%
5. Continue (null statement) 5%*/

void
generate_program ( void )
{
    generate_stringtable();
    generate_global_variables();
    generate_functions();
}

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

    //Generates list of strings 
    for (int i = 0; i<stringc; i++){
        printf("STR%d:\t.string %s\n", i, string_list[i]);
    }
}

void
generate_global_variables( void ){
    size_t size_globals = tlhash_size(global_names);
    symbol_t *global_list[size_globals];
    tlhash_values(global_names, (void**)&global_list);
    puts(".section .data");
    for (size_t i=0; i<size_globals; i++){
        if(global_list[i]->type == SYM_GLOBAL_VAR){
            printf("_%s\t.zero\t8\n", global_list[i]->name);
        }
    }
} 


static void
generate_functions( void ){
    size_t size_globals = tlhash_size(global_names);
    symbol_t *global_list[size_globals];
    tlhash_values(global_names, (void**)&global_list);
    size_t i;
    symbol_t *func;
    for (i = 0; i<size_globals; i++){  //Generates main
        func = global_list[i];
        if(func->type == SYM_FUNCTION && func->seq == 0){
            generate_main(func);
            break;
        }
    }
    for(i = 0; i < size_globals; i++){
        func = global_list[i];
        size_parameters = func->nparms;
        if(func->type == SYM_FUNCTION){
            printf("_%s:\n", func->name);
            puts("\tpushq %rbp\n\tmovq %rsp, %rbp"); //pushes rbp to stack and moves rsp to
            for(uint64_t j = 0; j<MIN(size_parameters, 6); j++){
                printf("\tpushq %s\n", record[j]);
            } //reserving space for local variables...:
            printf("\tsubq $%ld, %%rsp\n", 8 * (tlhash_size(func->locals) - size_parameters));
            if(tlhash_size(func->locals)%2 == 1){ //Check for stack alignment.
                puts("\tpushq\t$0");
            }
            traverse_function_tree_r(func->node);
        }
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

static void
traverse_function_tree_r(node_t *nd){
    switch(nd->type)
    {
        case ASSIGNMENT_STATEMENT:
            generate_assignment_statement(nd);
            break;
        case RETURN_STATEMENT:
            generate_return(nd->children[0]);
            break;
        case PRINT_STATEMENT:
            generate_print_statement(nd);
            break;
        case IF_STATEMENT:
            generate_if_statement(nd);
            break;
        case WHILE_STATEMENT:
            generate_while_loop(nd);
            break;
        case NULL_STATEMENT:
            printf("\tjmp WHILE%d\n", n_while_loops - 1);
            break;
        default:    
            for(size_t i = 0; i<nd->n_children; i++){
                if (nd->children[i] != NULL){ 
                    traverse_function_tree_r(nd->children[i]);
                }
            }
            break;
    }
}

static void
generate_assignment_statement ( node_t *statement )
{
    resolve_variable(statement->children[1], "%rax");
    assign_variable(statement->children[0], "%rax");
}

static void generate_return(node_t* nd) {
    switch (nd->type)
    {
        case EXPRESSION:
        case IDENTIFIER_DATA:
        case NUMBER_DATA:
            resolve_variable(nd, "%rax");
            break;
    }
    puts("\tleave\n\tret");
}

static void
generate_print_statement(node_t *nd){
    
    for(int i = 0; i <nd->n_children; i++){
        switch (nd->children[i]->type)
        {
            case EXPRESSION:
            case IDENTIFIER_DATA:
            case NUMBER_DATA:                
                resolve_variable(nd->children[i], "%rsi");
                puts("\tmovq $intout, %rdi\n\tcall printf");
                break;      
            case STRING_DATA:
                printf("\tmovq $STR%zu, %%rsi\n\tmovq $strout, %%rdi\n\tcall printf\n", *((size_t *) nd->children[i]->data));
                break;
        }
    }
    puts("\tmovq $0x0A, %rdi\n\tcall putchar"); //Creates newline
}

static void
generate_if_statement(node_t *nd){  
    node_t *relation = nd->children[0];
    int n_if_current = n_if_statements++; //Keeps track of jumping labels when having multpile ifs.
    resolve_variable(relation->children[0], "%rax");
    resolve_variable(relation->children[1], "%rbx");
    puts("\tcmpq %rax, %rbx");
    switch (*((char*) relation->data))
    {
        case '=':
            printf("jne");
            break;
        case '<':
            printf("jle");
            break;
        case '>':
            printf("jge");
            break;
    }
    if (nd->n_children == 2){
        printf("\t END_IF%d\n", n_if_current); //IF relation THEN statement -> END_IF
        traverse_function_tree_r(nd->children[1]);
    }
    else{                                                      
        printf("\t ELSE%d\n", n_if_current); //IF relation THEN statement1 ELSE statement2 -> END_IF 
        traverse_function_tree_r(nd->children[1]);
        printf("\tjmp END_IF%d\n", n_if_current);
        printf("ELSE%d:\n", n_if_current);
        traverse_function_tree_r(nd->children[2]);
    }
    printf("END_IF%d:\n", n_if_current);
}

static void
generate_while_loop(node_t *nd){
    node_t *relation = nd->children[0];
    int n_while_current = n_while_loops++; //Keeps track of jumping labels when having multpile while loops
    printf("WHILE%d:\n", n_while_current);
    resolve_variable(relation->children[0], "%rax");
    resolve_variable(relation->children[1], "%rbx");
    puts("\tcmpq %rax, %rbx");
    switch (*((char*) relation->data))
    {
        case '=':
            printf("jne");
            break;
        case '<':
            printf("jle");
            break;
        case '>':
            printf("jge");
            break;
    }
    printf("\t DONE%d\n", n_while_current);
    traverse_function_tree_r(nd->children[1]);
    printf("\tjmp WHILE%d\nDONE%d:\n", n_while_current, n_while_current);
}

static void
resolve_variable(node_t *nd, char *reg){
    switch (nd->type)
    {
        case EXPRESSION:
            generate_expression(nd);
            if (strcmp(reg, "%rax")){
                printf("\tmovq %%rax, %s\n", reg);
            }
            break;
        case IDENTIFIER_DATA:
            {
            symbol_t *symbol = nd->entry;
            switch ( symbol->type )
                {
                    case SYM_GLOBAL_VAR:
                        printf ("\tmovq _%s, %s\n", symbol->name, reg);
                        break;
                    case SYM_PARAMETER:
                        if ( symbol->seq > 5 )
                            printf ("\tmovq %ld(%%rbp), %s\n", 8+8*(symbol->seq-5), reg);
                        else
                            printf ("\tmovq %ld(%%rbp), %s\n", -8*(symbol->seq+1), reg);
                        break;
                    case SYM_LOCAL_VAR:
                        printf("\tmovq -%d(%%rbp), %s\n", (int) (8 * (nd->entry->seq + MIN(size_parameters, 6) + 1)), reg);
                        break;
                }
            }
            break;
        case NUMBER_DATA:
            printf("\tmovq $%d, %s\n", (int) *((int64_t*)nd->data), reg);
            break;
    }
}

static void
generate_expression(node_t* expr){
    node_t *c0 = expr->children[0];
    if(expr->n_children == 1){
        resolve_variable ( c0, "%rax" );
        switch (*((char*)(expr->data)) )
        {
            case '-':
                puts("\tnegq\t%%rax");
                break;
          }
    }
    else if(expr->n_children == 2){
        node_t *c1 = expr->children[1];
        if(c1 == NULL){ //Generates function call
            manage_function_call(expr, 0);
            printf("\tcall _%s\n", (char *) c0->data);
        }        
        else if(c1->type == EXPRESSION_LIST){ //Generates function call
            manage_function_call(expr, c1->n_children);
            printf("\tcall _%s\n", (char *) c0->data);
        }
        else if(expr->data != NULL){
            switch (*((char *)expr->data))
            {
                case '+':
                    resolve_variable(c0, "%rax");
                    puts("\tpushq %rax");
                    resolve_variable(c1, "%rax");
                    puts("\taddq %rax, (%rsp)\n\tpopq %rax");
                    break;
                case '-':
                    resolve_variable(c0, "%rax");
                    puts("\tpushq %rax");
                    resolve_variable(c1, "%rax");
                    puts("\tsubq %rax, (%rsp)\n\tpopq %rax"); 
                    break;
                case '*':
                    puts("\tpushq %rdx");
                    resolve_variable(c1, "%rax");
                    puts("\tpushq %rax");
                    resolve_variable(c0, "%rax");
                    puts("\tmulq (%rsp)\n\tpopq %rdx\n\tpopq %rdx");
                    break;
                case '/':
                    puts("\tpushq %rdx");
                    resolve_variable(c1, "%rax");
                    puts("\tpushq %rax");
                    resolve_variable(c0, "%rax");
                    puts("\tcqo\n\tidivq (%rsp)\n\tpopq %rdx\n\tpopq rdx");
                    break;
            }
        }
    }
}

static void
manage_function_call(node_t *nd, int n_children){
    int n_additional_parms = n_children;
    while(n_additional_parms > 6){
        n_additional_parms--;
        resolve_variable(nd->children[1]->children[n_additional_parms], "%rax");
        puts("\tpushq %rax");
    }
    for(int i = 0; i < MIN(n_children, 6); i++){
        resolve_variable(nd->children[1]->children[i], (char *) record[i]);
    }
}


static void assign_variable(node_t* nd, char* reg) {
    symbol_t *symbol = nd->entry;
    switch (symbol->type)
    {
        case SYM_GLOBAL_VAR:
            printf("\tmovq %s, _%s\n", reg, symbol->name);
            break;
        case SYM_PARAMETER:
            if(symbol->seq >5)
                printf("\tmovq %s, %ld(%%rbp)\n", reg, 8+8*(symbol->seq-5));
            else
                printf("\tmovq %s, %ld(%%rbp)\n", reg, -8*(symbol->seq-5));    
            break;
        case SYM_LOCAL_VAR:
            printf("\tmovq %s, %ld(%%rbp)\n", reg, (-8 * ((int) symbol->seq + MIN(size_parameters, 6) + 1)));
            break;
    }
}









