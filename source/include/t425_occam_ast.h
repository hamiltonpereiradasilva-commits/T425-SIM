#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#define T425_AST_MAX_NODES 256
#define T425_AST_MAX_NAME 32
typedef enum { AST_PROGRAM, AST_CHAN_DECL, AST_INT_DECL, AST_PAR, AST_SEQ,
 AST_IN, AST_OUT, AST_ALT, AST_TIMER_AFTER, AST_ASSIGN, AST_LITERAL, AST_VAR, AST_END } t425_ast_kind_t;
typedef struct {
 t425_ast_kind_t kind; char name[T425_AST_MAX_NAME]; int32_t value;
 uint16_t channel,var,first_child,next_sibling;
} t425_ast_node_t;
typedef struct { t425_ast_node_t node[T425_AST_MAX_NODES]; uint16_t count,root; } t425_ast_t;
void t425_ast_init(t425_ast_t*a);
int t425_ast_add(t425_ast_t*a,t425_ast_node_t n);
bool t425_ast_add_child(t425_ast_t*a,uint16_t parent,uint16_t child);
