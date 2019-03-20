#include <vslc.h>

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
    for ( size_t s=0; s<stringc; s++ )
        printf  ( "%zu: %s\n", s, string_list[s] );
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


void
find_globals ( void )
{

    global_names = malloc(sizeof(tlhash_t));
    tlhash_init(global_names, 32);
    node_t *global_parent = root->children[0];
    size_t sequence_counter = 0;
    

    for(int i = 0; i < global_parent->n_children; i++){ //Searches all children of GLOBAL_LIST. DEC or FUNC
        
        symbol_t *symbol = malloc(sizeof(symbol_t));
        node_t *global_child = global_parent->children[i];
        switch(global_parent->type){
            //node_t *grandchild= global_child->children[0]

            case FUNCTION: //Child1: Indentifier, Child2: parameter_list, Child3: statement

                symbol->name = global_child->children[0]->data;
                symbol->type = SYM_FUNCTION;
                symbol->node = global_child->children[2];
                symbol->seq = sequence_counter;
                //symbol->nparms = 0;
                symbol->locals = malloc(sizeof(tlhash_t));
                sequence_counter++;

                tlhash_init (symbol->locals, 32);
                if (global_child->children[1]){
                    symbol->nparms = global_child->children[1]->n_children;
                    for (int j = 0; j < symbol->nparms; j++){
                        symbol_t *sym_p = malloc(sizeof(symbol_t)); 
                        sym_p->name = global_child->children[1]->children[j]->data;
                        sym_p->type = SYM_PARAMETER;
                        sym_p->node = NULL;
                        sym_p->seq = j;
                        sym_p->nparms = 0;
                        sym_p->locals = NULL;

                        tlhash_insert(sym_p->locals, sym_p->name, strlen(sym_p->name), sym_p);


                    }
                }break;

                



            case DECLARATION: 
                for (int j=0; j < global_child->children[0]->n_children; j++){ //Searches through VARIABLE_LIST
                    symbol->name = global_child->children[0]->children[j]->data,
                    symbol->type = SYM_GLOBAL_VAR,
                    symbol->node = NULL,
                    symbol->seq = 0,
                    symbol->nparms = 0,
                    symbol->locals = NULL;
                    //tl_hash insert for global

                }break;

            
        }
        

    }
    //TODO
    //Find all nodes on depth one.

    
}

void
bind_names ( symbol_t *function, node_t *root )
{
    //TODO
}

void
destroy_symtab ( void )
{
}
