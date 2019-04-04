#include "vslc.h"
#include "generator.h"

#define MIN(a,b) (((a)<(b)) ? (a):(b))
static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

int n_if_statements = 0, n_while_loops = 0;

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
    /*size_t size_globals = tlhash_size(global_names);
    symbol_t *global_list[size_globals];
    tlhash_values(global_names, (void**)&global_list);
    for(size_t i = 0; i<size_globals; i++){
        symbol_t *global = global_list[i];
        if(global == SYM_FUNCTION){
            if(global->seq == 0){

            }
        }
    }*/
    
    
    //generate_functions();

    /* Put some dummy stuff to keep the skeleton from crashing 
    puts ( ".globl main" );
    puts ( ".section .text" );
    puts ( "main:" );
    puts ( "\tmovq $0, %rax" );
    puts ( "\tcall exit" );*/
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
        if(func->type == SYM_FUNCTION){
            printf("_%s:\n", func->name);
            puts("\tpushq %rbp\n\tmovq %rsp, %rbp"); //pushes rbp to stack and moves rsp to
            for(uint64_t j = 0; j<MIN(func->nparms, 6); j++){
                printf("\tpushq %s\n", record[j]);
            } //reserving space for local variables...:
            printf("\tsubq $%d, %%rsp\n", (int) (8 * (tlhash_size(func->locals) - func->nparms))); //Possibly check for NULL
            if(tlhash_size(func->locals)%2 == 1){ //Check for stack alignment.
                puts("\tpushq\t$0");
            }
            resolve_function_r(func->node);
        }
    }
}

void
resolve_function_r(node_t *nd){
    switch(nd->type){

        /*case EXPRESSION:
            //TODO: handle function calls
            puts("\tEXPRESSION");
            break;*/
        case ASSIGNMENT_STATEMENT:
            manage_assignment_statement(nd);
            break;
        case RETURN_STATEMENT:
            //find_variable(...)
            puts("\tleave\n\tret");
            break;
        case PRINT_STATEMENT:
            resolve_print_statement(nd);
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
                if (nd->children[i]!=NULL){ //Possibly check for NULL
                    resolve_function_r(nd->children[i]);
                }
            }
            break;
    }
}

void
resolve_expression(node_t *nd ){
    //TODO:
    //manage_function_call
    puts("\tEXPRESSION");
}

void
manage_function_call(node_t *nd, int n_children){
    int n_additional_parms = n_children;
    while(n_additional_parms >= 6){
        n_additional_parms--;
        //find_variable(nd->children[1]->[n_additional_parms], %rax);
        puts("\tpushq %rax");
    }
    for(int i = 0; i < MIN(n_children, 6); i++){
        //find_variable(nd->children[1]->children[i], (*char)record[i]);
        break;
    }
    //printf("\tcall _%s\n", (char *) node->children[0]->data);
}

void
manage_assignment_statement(node_t *nd){
    puts("\tASSIGNMENT_STATEMENT");
}

void
resolve_print_statement(node_t *nd){
    for(int i = 0; i <nd->n_children; i++){
        switch (nd->children[i]->type)
        {
            case EXPRESSION:
                break;
            case IDENTIFIER_DATA:
                break;
            case NUMBER_DATA:
                break;      
            case STRING_DATA:
                printf("\tmovq $STR%d, %%rsi\n\tmovq $strout, %%rdi\n\tcall printf\n", *((int64_t *) nd->children[i]->data)); //TODO: Double-check casting
                break;
        }
    }
    puts("\tmovq $'\\n', %rdi\n\tcall putchar"); //Creates newline
    //puts("\tcall putchar");
}

void
generate_if_statement(node_t *nd){
    int n_if_current = n_if_statements++;
    node_t *relation = nd->children[0];
    //generate_expression(relation([0]));
    puts("\tmovq\t%rax, %rbx");
    //generate_expression(relation[1]));
    puts("\tcmp %rax, %rbx");
    switch (*((char*) relation->data)){ ///TODO: Double-check casting
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
        resolve_function_r(nd->children[1]);
    }else{                                                      
        printf("\t ELSE%d\n", n_if_current); //IF relation THEN statement1 ELSE statement2 -> END_IF 
        printf("\tjmp END_IF%d\n", n_if_current);
        resolve_function_r(nd->children[1]);
        printf("ELSE%d:\n", n_if_current);
        resolve_function_r(nd->children[2]);
    }
    printf("END_IF%d:\n", n_if_current);
}

void
generate_while_loop(node_t *nd){
    int n_while_current = n_while_loops++;
    node_t *relation = nd->children[0];
    printf("WHILE%d:\n", n_while_current);
    //generate_expression(relation([0]));
    puts("\tmovq\t%rax, %rbx");
    //generate_expression(relation[1]));
    puts("\tcmp %rax, %rbx");
    switch (*((char*) relation->data)){ //Possible casting error
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
    resolve_function_r(nd->children[1]);
    printf("\tjmp WHILE%d\n", n_while_current);
    printf("DONE%d:\n", n_while_current);   
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



