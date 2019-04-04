#include "vslc.h"
static void generate_main(symbol_t*);
static void generate_stringtable ( void );
static void generate_global_variables( void );
static void resolve_function_r(node_t*);
static void resolve_expression(node_t *);
static void manage_function_call(node_t*, int);
static void manage_assignment_statement(node_t *);
static void resolve_print_statement(node_t*);
static void generate_functions( void );
static void generate_if_statement(node_t*);
static void generate_while_loop(node_t*);
