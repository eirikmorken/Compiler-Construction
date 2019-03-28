#include <vslc.h>
#include <stdio.h>


// Externally visible, for the generator
extern tlhash_t *global_names;
extern char **string_list;
extern size_t n_string_list,stringc;

/* External interface */

void create_symbol_table(void);
void print_symbol_table(void);
void print_symbols(void);
void print_bindings(node_t *root);
void destroy_symbol_table(void);
void find_globals(void);
void bind_names(symbol_t *function, node_t *root);
void destroy_symtab(void);

//Scope_stack
tlhash_t **scope_stack = NULL;

void
create_symbol_table ( void )
{
  find_globals();
  size_t n_globals = tlhash_size ( global_names );
  symbol_t *global_list[n_globals];
  tlhash_values ( global_names, (void **)&global_list );
  for ( size_t i=0; i<n_globals; i++ )
      if ( global_list[i]->type == SYM_FUNCTION )
          bind_names ( global_list[i], global_list[i]->node );
}



//Output stored in the "*.tree" files.
void
print_symbol_table ( void )
{
	print_symbols();
	print_bindings(root);
}



void
print_symbols ( void )
{
    printf ( "String table:\n" );
    for ( size_t s=0; s<stringc; s++ ){
        printf  ( "%zu: %s\n", s, string_list[s] );
    }
    printf ( "-- \n" );

    printf ( "Globals:\n" );
    size_t n_globals = tlhash_size(global_names);
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );
    for ( size_t g=0; g<n_globals; g++ )
    {
        switch ( global_list[g]->type )
        {
            case SYM_FUNCTION:
                printf (
                    "%s: function %zu:\n",
                    global_list[g]->name, global_list[g]->seq
                );
                if ( global_list[g]->locals != NULL )
                {
                    size_t localsize = tlhash_size( global_list[g]->locals );
                    printf (
                        "\t%zu local variables, %zu are parameters:\n",
                        localsize, global_list[g]->nparms
                    );
                    symbol_t *locals[localsize];
                    tlhash_values(global_list[g]->locals, (void **)locals );
                    for ( size_t i=0; i<localsize; i++ )
                    {
                        printf ( "\t%s: ", locals[i]->name );
                        switch ( locals[i]->type )
                        {
                            case SYM_PARAMETER:
                                printf ( "parameter %zu\n", locals[i]->seq );
                                break;
                            case SYM_LOCAL_VAR:
                                printf ( "local var %zu\n", locals[i]->seq );
                                break;
                        }
                    }
                }
                break;
            case SYM_GLOBAL_VAR:
                printf ( "%s: global variable\n", global_list[g]->name );
                break;
        }
    }
    printf ( "-- \n" );
}


void
print_bindings ( node_t *root )
{
    if ( root == NULL )
        return;
    else if ( root->entry != NULL )
    {
        switch ( root->entry->type )
        {
            case SYM_GLOBAL_VAR: 
                printf ( "Linked global var '%s'\n", root->entry->name );
                break;
            case SYM_FUNCTION:
                printf ( "Linked function %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break; 
            case SYM_PARAMETER:
                printf ( "Linked parameter %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break;
            case SYM_LOCAL_VAR:
                printf ( "Linked local var %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break;
        }
    } else if ( root->type == STRING_DATA ) {
        size_t string_index = *((size_t *)root->data);
        if ( string_index < stringc )
            printf ( "Linked string %zu\n", *((size_t *)root->data) );
        else
            printf ( "(Not an indexed string)\n" );
    }
    for ( size_t c=0; c<root->n_children; c++ )
        print_bindings ( root->children[c] );
}


void
destroy_symbol_table ( void )
{
      destroy_symtab();
}



//Finds all globals by searching the children of the GLOBAL_LIST-node.
//Creates symbols for globals and hash-inserts them to global_names.
void
find_globals ( void )
{
    global_names = malloc(sizeof(tlhash_t));
    tlhash_init(global_names, 32);
    node_t *global_parent = root->children[0];
    int sequence_counter = 0;
    for(uint64_t i = 0; i < global_parent->n_children; i++){ //Searches all children of GLOBAL_LIST.
        node_t *global_child = global_parent->children[i];
        switch(global_child->type){
            case FUNCTION: //Child0: Indentifier, Child1: parameter_list, Child2: statement
            { 
                symbol_t *symbol = malloc(sizeof(symbol_t));
                symbol->name = global_child->children[0]->data;
                symbol->type = SYM_FUNCTION;
                symbol->node = global_child->children[2];
                symbol->seq = (size_t) sequence_counter;
                symbol->nparms = 0;
                symbol->locals = malloc(sizeof(tlhash_t));
                sequence_counter++;
                tlhash_init (symbol->locals, 32);
                if (global_child->children[1] != NULL){
                    symbol->nparms = global_child->children[1]->n_children;
                    for (uint64_t j = 0; j < symbol->nparms; j++){
                        symbol_t *sym_p = malloc(sizeof(symbol_t)); 
                        sym_p->name = global_child->children[1]->children[j]->data;
                        sym_p->type = SYM_PARAMETER;
                        sym_p->node = NULL;
                        sym_p->seq = j;
                        sym_p->nparms = 0;
                        sym_p->locals = NULL;
                        tlhash_insert(symbol->locals, sym_p->name, strlen(sym_p->name), sym_p);
                    }
                }
                tlhash_insert(global_names, symbol->name, strlen(symbol->name), symbol);
                break;
            }
            case DECLARATION:
            {   
                for (uint64_t j=0; j < global_child->children[0]->n_children; j++){ //Searches through VARIABLE_LIST
                    symbol_t *symbol = malloc(sizeof(symbol_t));
                    symbol->name = global_child->children[0]->children[j]->data;
                    symbol->type = SYM_GLOBAL_VAR;
                    symbol->node = NULL;
                    symbol->seq = 0;
                    symbol->nparms = 0;
                    symbol->locals = NULL;
                    tlhash_insert(global_names, symbol->name, strlen(symbol->name), symbol);
                }
                break;
            }
                    
        }
    }
}
//Applied on all function subtrees by loop in create_symbol_table.
//Iterates by for loop through all children of root, with root starting as global FUNCTION-nodes, 
//and recursivley moves down each sub-tree with bind_names_r().
void
bind_names ( symbol_t *function, node_t *root )
{
    uint64_t stack_depth = 1, max_stack_depth = 64; //Stack_depth initiated to 1 as global_names are pushed to the stack at scope_stack[0].
    if(scope_stack == NULL){ //Bind names called for each function, so no need to create multiple scope_stacks and string_lists.
        scope_stack = malloc(max_stack_depth * sizeof(tlhash_t *));
        scope_stack[0] = global_names;
        string_list = malloc(sizeof(*string_list));
        stringc = 0;
    }
    bind_names_r (function, root, stack_depth);
}


void bind_names_r ( symbol_t *function, node_t *root, uint64_t stack_depth){
    for ( uint64_t i=0; i<root->n_children; i++){
        if(root->children[i] != NULL){
            node_t *child = root->children[i];
            switch (child->type){
                case BLOCK: //Allocates memory for scope, and pushes to stack. 
                {
                    tlhash_t *scope = malloc(sizeof(tlhash_t));
                    tlhash_init(scope, 32);
                    scope_stack[stack_depth++] = scope;
                    for(uint64_t j=0; j<child->n_children; j++){
                        bind_names_r(function, child->children[j], stack_depth);
                    }
                    tlhash_finalize(scope_stack[stack_depth-1]);
                    free(scope);
                    scope_stack[stack_depth-1] = NULL;
                    stack_depth--;
                    break;
                }
                case DECLARATION: //Creates symbol_t structs for each declaration and inserts them into hash_tables. 
                    for(uint64_t j=0; j<child->children[0]->n_children; j++){
                        symbol_t *symbol = malloc(sizeof(symbol_t));
                        symbol->name = child->children[0]->children[j]->data;
                        symbol->type = SYM_LOCAL_VAR;
                        symbol->node = NULL;
                        symbol->seq = tlhash_size(function->locals) - function->nparms;
                        symbol->nparms = 0;
                        symbol->locals = NULL;
                        tlhash_insert(function->locals, &(symbol->seq), sizeof(size_t), symbol);
                        tlhash_insert(scope_stack[stack_depth-1], symbol->name, strlen(symbol->name), symbol);
                    }
                    break;
                case IDENTIFIER_DATA: //Iteratively checks if INDENTIFIER exists in scope_stack. First locally, then globally.
                {   
                    size_t sd = stack_depth;
                    symbol_t *lookup_target = NULL;
                    if(lookup_target == NULL){
                        tlhash_lookup(function->locals, child->data, strlen(child->data), (void**)&lookup_target);
                        
                    }
                    while(lookup_target == NULL && sd>0){
                        sd--;
                        tlhash_lookup(scope_stack[sd], child->data, strlen(child->data), (void**)&lookup_target);
                    }
                    if(lookup_target==NULL){
                            printf("Target symbol not found\n");
                            exit(EXIT_FAILURE);
                        }
                    child->entry = lookup_target;
                    break;
                }
                case STRING_DATA: //Allocs memory for strings and stores pointer in string_list. Replaces data in nodes with the pointer in string_list.
                    string_list = realloc(string_list, (stringc+1) * sizeof(*string_list));
                    string_list[stringc] = child->data;
                    child->data = malloc(sizeof(size_t));
                    *((size_t *) child->data) = stringc++;
                    break;
                default:
                    bind_names_r(function, child, stack_depth);
                    break;
            }
        }    
    }
}


//Extracts the values (symbol_t structs) from the hashed table.
//Frees both memory allocated in the hash_tables and the string_list.
void
destroy_symtab ( void )
{   
    size_t global_names_size = tlhash_size(global_names);
    symbol_t *values[global_names_size];
    tlhash_values(global_names, (void **)&values); //Looks up values
    for(size_t i = 0; i<global_names_size; i++){
        if(values[i]->type == SYM_FUNCTION){
            size_t locals_size = tlhash_size(values[i]->locals);
            symbol_t *local_values[locals_size];
            tlhash_values(values[i]->locals, (void**)&local_values); 
            for(size_t j = 0; j<locals_size; j++){
                free(local_values[j]);
            }
            tlhash_finalize(values[i]->locals);
            free(values[i]->locals);
        }
        free(values[i]);
    }
    tlhash_finalize(global_names);
    free(global_names);
    for(size_t i = 0; i<stringc; i++){
        free(string_list[i]);
    }
    free(string_list);
    free(scope_stack);
}

//TODO: no-pie in makefile

//cd ..; make purge && make; cd vsl_programs; make clean; valgrind --leak-check=yes ../src/vslc <euclid.vsl> euclid.tree
