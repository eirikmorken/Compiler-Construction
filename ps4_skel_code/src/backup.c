#include <vslc.h>
#include <stdio.h>
#include <ir.c>

void
bind_names ( symbol_t *function, node_t *root )
{
    printf("Bind-names\n");
    uint64_t stack_depth = 0, max_stack_depth = 128;
    scope_stack = malloc(max_stack_depth * sizeof(tlhash_t *)); // global_names[0]
    scope_stack[0] = global_names;
    bind_names_r (function, root, stack_depth); //TODO: root->children[2]
    string_list = malloc(sizeof(*string_list));
    stringc = 0;
}

void bind_names_r ( symbol_t *function, node_t *root, uint64_t stack_depth){
    //printf("Bnr: \n");
    //printf(root->type);
    //Iterating through
    if(root != NULL){
        switch (root->type){
            case BLOCK:
            {
                printf("    BLOCK");
                symbol_t *scope = malloc(sizeof(tlhash_t));
                printf("    *scope");
                //TODO:
                //scope_stack = realloc(scope_stack, (stack_depth) * sizeof(tlhash_t));
                tlhash_init(scope, 32);
                printf("    init");
                scope_stack[stack_depth++] = scope;
                printf("    scope_stack");
                printf("    root->n_children: %d\n", root->n_children);
                for(size_t j=0; j<root->n_children; j++){
                    printf("        BLOCK->CHILD    ");
                    //printf("        BLOCK->CHILD%d", (int) j);
                    bind_names_r(function, root->children[j], stack_depth);
                }
                //TODO: Pop scope off stack, without freeing destroying stack.
                tlhash_finalize(scope_stack[stack_depth-1]);
                free(scope);
                stack_depth--;
                break;
            }
            case DECLARATION: //Inserts all declaration into symbol tables.
                printf("    DECLARATION\n");
                for(uint64_t j; j<root->children[0]->n_children; j++){
                    symbol_t *symbol = malloc(sizeof(symbol_t));
                    //size_t local_sequence = tlhash_size(function->locals) - function->nparms;
                    symbol->name = root->children[0]->children[j]->data;
                    symbol->type = SYM_LOCAL_VAR;
                    symbol->node = NULL;
                    symbol->seq = tlhash_size(function->locals) - function->nparms; //TODO: Explain this
                    symbol->nparms = 0;
                    symbol->locals = NULL;
                    tlhash_insert(function->locals, &(symbol->seq), sizeof(size_t), symbol);
                    tlhash_insert(scope_stack[stack_depth-1], symbol->name, strlen(symbol->name), symbol);
                }
                
                break;
            case IDENTIFIER_DATA:
            { //Iteratively checks if INDENTIFIER exists in scope_stack. First locally, then globally.  
                printf("    IDENTIFIER\n");
                size_t sd = stack_depth;
                symbol_t *lookup_target = NULL;
                while(lookup_target == NULL && sd>0){
                    sd--;
                    tlhash_lookup(scope_stack[sd], root->data, strlen(root->data), (void**)&lookup_target);
                }
                if(lookup_target == NULL){
                    tlhash_lookup(function->locals, root->data, strlen(root->data), (void**)&lookup_target);
                    if(lookup_target==NULL){
                        printf("Target symbol not found\n");
                        exit(EXIT_FAILURE);
                    }
                }
                /*

                }*/
                root->entry = lookup_target;
                break;
            }
            case STRING_DATA:
            {
                printf("    STRING_DATA");
                string_list = realloc(string_list, (stringc+1) * sizeof(*string_list));
                string_list[stringc] = root->data;
                root->data = malloc(sizeof(size_t));
                *((size_t *) root->data) = stringc++;
                printf("     Stringc: %d\n", stringc);
                break;
            }
            default:
                //printf("    DEFAULT\n");
                for(size_t k =0; k<root->n_children; k++){
                    bind_names_r(function, root->children[k], stack_depth);
                }
                break;
                
        }
    }    

}

void bind_names_r ( symbol_t *function, node_t *root, uint64_t stack_depth){
    //printf("Bnr: \n");
    //printf(root->type);
    for ( uint64_t i=0; i<root->n_children; i++){ //Iterating through 
        if(root->children[i] != NULL){
            node_t *child = root->children[i];
            switch (child->type){
                case BLOCK:
                {
                    printf("    BLOCK\n");
                    symbol_t *scope = malloc(sizeof(tlhash_t));
                    //TODO:
                    //scope_stack = realloc(scope_stack, (stack_depth) * sizeof(tlhash_t));
                    tlhash_init(scope, 32);
                    scope_stack[stack_depth++] = scope;
                    for(uint64_t j=0; j<child->n_children; j++){
                        bind_names_r(function, child->children[j], stack_depth);
                    }
                    //TODO: Pop scope off stack, without freeing destroying stack.
                    tlhash_finalize(scope_stack[stack_depth-1]);
                    free(scope);
                    stack_depth--;
                    break;
                }
                case DECLARATION: //Inserts all declaration into symbol tables.
                    printf("    DECLARATION\n");
                    for(uint64_t j=0; j<child->children[0]->n_children; j++){
                        symbol_t *symbol = malloc(sizeof(symbol_t));
                        symbol->name = child->children[0]->children[j]->data;
                        symbol->type = SYM_LOCAL_VAR;
                        symbol->node = NULL;
                        symbol->seq = tlhash_size(function->locals) - function->nparms; //TODO: Explain this
                        symbol->nparms = 0;
                        symbol->locals = NULL;
                        tlhash_insert(function->locals, &(symbol->seq), sizeof(size_t), symbol);
                        tlhash_insert(scope_stack[stack_depth-1], symbol->name, strlen(symbol->name), symbol);
                    }
                    break;
                case IDENTIFIER_DATA:
                { //Iteratively checks if INDENTIFIER exists in scope_stack. First locally, then globally.  
                    printf("    IDENTIFIER\n");
                    size_t sd = stack_depth;
                    symbol_t *lookup_target = NULL;
                    while(lookup_target == NULL && sd>0){
                        sd--;
                        tlhash_lookup(scope_stack[sd], child->data, strlen(child->data), (void**)&lookup_target);
                    }
                    if(lookup_target == NULL){
                        tlhash_lookup(function->locals, child->data, strlen(child->data), (void**)&lookup_target);
                        if(lookup_target==NULL){
                            printf("Target symbol not found\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    
                    /*if(lookup_target == NULL){
                        tlhash_lookup(global_names, child->data, strlen(child->data), (void**)&lookup_target);

                    }*/
                    child->entry = lookup_target;
                    break;
                }
                case STRING_DATA:
                    printf("    STRING_DATA\n");
                    string_list = realloc(string_list, (stringc+1) * sizeof(*string_list));
                    string_list[stringc] = child->data;
                    child->data = malloc(sizeof(size_t));
                    *((size_t *) child->data) = stringc++;
                    break;
                default:
                    //printf("    DEFAULT\n");
                    bind_names_r(function, child, stack_depth);
                    break;
            }
        }    
    }
}

/*
void bind_names_r ( symbol_t *function, node_t *root, uint64_t stack_depth){
    //printf("Bnr: \n");
    //printf(root->type);
    //Iterating through
    if(root != NULL){
        switch (root->type){
            case BLOCK:
            {
                printf("    BLOCK");
                symbol_t *scope = malloc(sizeof(tlhash_t));
                printf("    *scope");
                //TODO:
                //scope_stack = realloc(scope_stack, (stack_depth) * sizeof(tlhash_t));
                tlhash_init(scope, 32);
                printf("    init");
                scope_stack[stack_depth++] = scope;
                printf("    scope_stack");
                printf("    root->n_children: %d\n", root->n_children);
                for(size_t j=0; j<root->n_children; j++){
                    printf("        BLOCK->CHILD    ");
                    //printf("        BLOCK->CHILD%d", (int) j);
                    bind_names_r(function, root->children[j], stack_depth);
                }
                //TODO: Pop scope off stack, without freeing destroying stack.
                tlhash_finalize(scope_stack[stack_depth-1]);
                free(scope);
                stack_depth--;
                break;
            }
            case DECLARATION: //Inserts all declaration into symbol tables.
                printf("    DECLARATION\n");
                for(uint64_t j; j<root->children[0]->n_children; j++){
                    symbol_t *symbol = malloc(sizeof(symbol_t));
                    //size_t local_sequence = tlhash_size(function->locals) - function->nparms;
                    symbol->name = root->children[0]->children[j]->data;
                    symbol->type = SYM_LOCAL_VAR;
                    symbol->node = NULL;
                    symbol->seq = tlhash_size(function->locals) - function->nparms; //TODO: Explain this
                    symbol->nparms = 0;
                    symbol->locals = NULL;
                    tlhash_insert(function->locals, &(symbol->seq), sizeof(size_t), symbol);
                    tlhash_insert(scope_stack[stack_depth-1], symbol->name, strlen(symbol->name), symbol);
                }
                
                break;
            case IDENTIFIER_DATA:
            { //Iteratively checks if INDENTIFIER exists in scope_stack. First locally, then globally.  
                printf("    IDENTIFIER\n");
                size_t sd = stack_depth;
                symbol_t *lookup_target = NULL;
                while(lookup_target == NULL && sd>0){
                    sd--;
                    tlhash_lookup(scope_stack[sd], root->data, strlen(root->data), (void**)&lookup_target);
                }
                if(lookup_target == NULL){
                    tlhash_lookup(function->locals, root->data, strlen(root->data), (void**)&lookup_target);
                    if(lookup_target==NULL){
                        printf("Target symbol not found\n");
                        exit(EXIT_FAILURE);
                    }
                }
                root->entry = lookup_target;
                break;
            }
            case STRING_DATA:
            {
                printf("    STRING_DATA");
                string_list = realloc(string_list, (stringc+1) * sizeof(*string_list));
                string_list[stringc] = root->data;
                root->data = malloc(sizeof(size_t));
                *((size_t *) root->data) = stringc++;
                printf("     Stringc: %d\n", stringc);
                break;
            }
            default:
                //printf("    DEFAULT\n");
                for(size_t k =0; k<root->n_children; k++){
                    bind_names_r(function, root->children[k], stack_depth);
                }
                break;
                
        }
    }    

}*/