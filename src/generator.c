#include "./generator.h"
#include <string.h>

#define EXIT(s, a)\
         printf(ANSI_COLOR_RED "[generator]: " s "\n" ANSI_COLOR_RESET, a, pos.line, pos.line_pos+1);\
         (void)system("rm out.c");\
         exit(1);

static inline const char* get_type(Token token) {
   switch (token.type) {
      case char_:
         return "char";
      case uchar_:
         return "unsigned char";
      case i8_:
         return "__INT8_TYPE__";
      case i16_:
         return "__INT16_TYPE__";
      case i32_:
         return "__INT32_TYPE__";
      case i64_:
         return "__INT64_TYPE__";
      case u8_:
         return "__UINT8_TYPE__";
      case u16_:
         return "__UINT16_TYPE__";
      case u32_:
         return "__UINT32_TYPE__";
      case u64_:
         return "__UINT64_TYPE__";
      case f32_:
         return "float";
      case f64_:
         return "double";
      case bool_:
         return "_Bool";
      case ptr:
         return "__INTPTR_TYPE__";
      case uptr:
         return "__UINTPTR_TYPE__";
      case string:
         return "char*"; // to change later, placeholder
      default:
         TokenPos pos = token.pos;
         EXIT("Unknown type %s at %zu:%zu\n", token_repr(token));
         exit(1);
   }
}

// TODO Implement string fatptr backend

static inline void gen_term(Generator* gen, NodeTerm* term) {
   switch (term->type) {
      case TERM_INT_LIT: 
         {
            fprintf(gen->out, "%s", term->value.int_lit->int_lit.value);
         }
         break;
      case TERM_STRING_LIT: 
         {
            fprintf(gen->out, "\"%s\"", term->value.string_lit->string_lit.value);
         }
         break;
      case TERM_CHAR_LIT:
         {
            fprintf(gen->out, "'%s'", term->value.char_lit->char_lit.value);
         }
         break;
      case TERM_IDENT: 
         {
            for (size_t i = 0; i < gen->table.size; i++) {
               if (!strcmp(gen->table.items[i].ident, term->value.ident->ident.value)) {
                  fprintf(gen->out, "%s", term->value.ident->ident.value);
                  return;
               }
            }
            TokenPos pos = term->value.ident->ident.pos;
            EXIT("Unknown identifier: `%s` at %zu:%zu. Invalid expression.", term->value.ident->ident.value);
         }
   }
}

static inline void gen_expr(Generator* gen, NodeExpr* expr) {
   switch (expr->type) {
      case EXPR_TERM: 
         {
            gen_term(gen, expr->value.term);
         }
         break;
      case EXPR_BIN_EXPR: 
         {
            switch (expr->value.bin_expr->type) {
               case EXPR_ADD: 
                  {
                     gen_expr(gen, expr->value.bin_expr->var.add->lhs);
                     fprintf(gen->out, "+");
                     gen_expr(gen, expr->value.bin_expr->var.add->rhs);
                  }
                  break;
               case EXPR_MULTI:
                  {
                     gen_expr(gen, expr->value.bin_expr->var.multi->lhs);
                     fprintf(gen->out, "*");
                     gen_expr(gen, expr->value.bin_expr->var.multi->rhs);
                  }
                  break;
               case EXPR_SUBTR:
                  {
                     gen_expr(gen, expr->value.bin_expr->var.multi->lhs);
                     fprintf(gen->out, "-");
                     gen_expr(gen, expr->value.bin_expr->var.multi->rhs);
                  }
                  break;
               case EXPR_DIVIDE:
                  {
                     gen_expr(gen, expr->value.bin_expr->var.multi->lhs);
                     fprintf(gen->out, "/");
                     gen_expr(gen, expr->value.bin_expr->var.multi->rhs);
                  }
                  break;
            }
         }
         break;
   }
}

static inline bool check_type_uchar(Symbol* sym, NodeStmt* stmt) {
   if (stmt->stmt_let->expr->value.term->type != TERM_CHAR_LIT) return false;
   if (uchar_ == sym->token.type) {
      return true;
   }
   return false;
}

static inline bool check_type_char(Symbol* sym, NodeStmt* stmt) {
   if (stmt->stmt_let->expr->value.term->type != TERM_CHAR_LIT) return false;
   if (char_ == sym->token.type) {
      return true;
   }
   return false;
} 

static inline bool check_type_string(Symbol* sym, NodeStmt* stmt) {
   if (stmt->stmt_let->expr->value.term->type != TERM_STRING_LIT) return false;
   if (string == sym->token.type) {
      return true;
   }
   return false;
}

static inline bool check_type_int(Symbol* sym, NodeStmt* stmt) {
   if (stmt->stmt_let->expr->value.term->type != TERM_INT_LIT) return false;
   for (TokenType i = i8_; i <= f64_; i++) { // quite fragile be careful
      if (i == sym->token.type) {
         return true;
      }
   }
   return false;
}

// TODO: integer overflow — exit instead of wrapping
static inline void gen_stmt(Generator* gen, NodeStmt* stmt) {
   TokenPos pos = stmt->stmt_let->ident.pos;
   switch (stmt->type) {
      case STMT_LET:
         {
            for (size_t i = 0; i < gen->table.size; i++) {
               if (!strcmp(gen->table.items[i].ident, stmt->stmt_let->ident.value)) {
                  EXIT("Redefinition of `%s` at %zu:%zu. Definition failed.", stmt->stmt_let->ident.value);
               }
            }

            Symbol sym;
            sym.ident = stmt->stmt_let->ident.value;
            sym.token = stmt->stmt_let->token;
            sym.type  = stmt->stmt_let->ident.type;
            sym.mut   = stmt->stmt_let->mut;
            da_append(&gen->table, sym);

            // Checking the types
            // TODO Currently I make sure that the type of the variable aligns with the 
            // assignment value, however I have the TERM tags - which means I should 
            // probably pattern match them to call the corresponding function
            if (check_type_string(&sym, stmt)) {
               if (sym.mut) {
                  fprintf(gen->out, "    char* %s = ", sym.ident);
               }
               else {
                  fprintf(gen->out, "    const char* %s = ", sym.ident);
               }
            }
            else if (check_type_char(&sym, stmt)) {
               if (sym.mut) {
                  fprintf(gen->out, "    char %s = ", sym.ident);
               }
               else {
                  fprintf(gen->out, "    const char %s = ", sym.ident);
               }
            }
            else if (check_type_uchar(&sym, stmt)){
               if (sym.mut) {
                  fprintf(gen->out, "    unsigned char %s = ", sym.ident);
               }
               else {
                  fprintf(gen->out, "    const unsigned char %s = ", sym.ident);
               }
            }
            // Perform the most expensive check last (probably negligible)
            // but also good practice I suppose
            else if (check_type_int(&sym, stmt)) {
               if (sym.mut) {
                  fprintf(gen->out, "    %s %s = ", get_type(stmt->stmt_let->token), sym.ident);
               }
               else {
                  fprintf(gen->out, "    const %s %s = ", get_type(stmt->stmt_let->token), sym.ident);
               }
            }
            else {
               EXIT("Type mismatch of `%s` at %zu:%zu.", stmt->stmt_let->ident.value);
            }

            gen_expr(gen, stmt->stmt_let->expr);
            fprintf(gen->out, ";\n");
         }
         break;
      case STMT_EXIT:
         {
            fprintf(gen->out, "    return ");
            gen_expr(gen, stmt->stmt_exit->expr);
            fprintf(gen->out, ";\n");
         }
         break;
      case STMT_PRINTLN:
         {
            fprintf(gen->out, "    puts(");
            gen_expr(gen, stmt->stmt_print->expr);
            fprintf(gen->out, ");\n");
         }
         break;
      case STMT_ASSIGN:
         {
            for (size_t i = 0; i < gen->table.size; i++) {
               if (!strcmp(gen->table.items[i].ident, stmt->stmt_assign->ident.value)) {
                  if (!gen->table.items[i].mut) {
                     EXIT("Attempting to change an immutable variable `%s` at %zu:%zu. Assignment failed.", stmt->stmt_assign->ident.value);
                  }
                  else {
                     Symbol sym = { .ident = stmt->stmt_assign->ident.value, .type = stmt->stmt_assign->ident.type, .mut = true};
                     fprintf(gen->out, "    %s = ", sym.ident);
                     gen_expr(gen, stmt->stmt_assign->expr);
                     fprintf(gen->out, ";\n");
                     return;
                  }
               }
            }
            EXIT("Undefined identifier `%s` at %zu:%zu. Assignment failed.", stmt->stmt_assign->ident.value);
         }
         break;
   }
}

Generator Generator_create(NodeProg* stmts, FILE* out) {
   Generator gen = {0};
   gen.stmts = stmts;
   gen.out = out;
   return gen;
}

void gen_prog(Generator* gen) {
   fprintf(gen->out, "// Generated automatically by the forgelang compiler\n");
   fprintf(gen->out, "#include <stdio.h>\n\n"); // TODO Only emit the libraries that are necessary
   fprintf(gen->out, "int main(void) {\n");
   for (size_t i = 0; i < gen->stmts->size; i++) {
      gen_stmt(gen, &gen->stmts->items[i]);
   }
   // for (size_t i = 0; i < gen->table.size; i++) {
      // printf("%s: ", gen->table.items[i].ident);
      // printf("%s\n", gen->table.items[i].var_type);
   // }
   fprintf(gen->out, "}\n");
}
