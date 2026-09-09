#pragma once
#include <stdbool.h>
#include "t425_occam_ast.h"
/* Grammar:
   program := { chan_decl | int_decl } block
   block   := ("SEQ"|"PAR") "{" stmt* "}"
   stmt    := "OUT" "(" channel "," integer ")" ";"
           | "IN" "(" channel "," variable ")" ";"
           | "TIMER_AFTER" "(" integer ")" ";"
           | "END" ";"
   channel := "c" integer
   variable := "v" integer
*/
bool t425_occam_parse_text(const char *text,t425_ast_t *ast);
