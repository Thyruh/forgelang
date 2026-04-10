#include "./generator.h"
#include <string.h>

#define EXIT(s, a)\
         printf(s, a);\
         (void)system("rm out.c");\
         exit(1);

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
      case TERM_IDENT: 
         {
            for (size_t i = 0; i < gen->table.size; i++) {
               if (!strcmp(gen->table.items[i].ident, term->value.ident->ident.value)) {
                  fprintf(gen->out, "%s", term->value.ident->ident.value);
                  return;
               }
            }
            EXIT("[generator]: Unknown identifier: `%s`. Invalid expression.\n", term->value.ident->ident.value);
         }
         break;
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
                     gen_expr(gen, expr->value.bin_expr->var.add->lhs);
                     fprintf(gen->out, "*");
                     gen_expr(gen, expr->value.bin_expr->var.add->rhs);
                  }
                  break;
            }
         }
         break;
   }
}

static inline void gen_stmt(Generator* gen, NodeStmt* stmt) {
   switch (stmt->type) {
      case STMT_LET:
         {
            for (size_t i = 0; i < gen->table.size; i++) {
               if (!strcmp(gen->table.items[i].ident, stmt->stmt_let->ident.value)) {
                  EXIT("[generator]: Redefinition of `%s`. Definition failed.\n", stmt->stmt_let->ident.value);
               }
            }
            Symbol sym = { stmt->stmt_let->ident.value };
            da_append(&gen->table, sym);
            fprintf(gen->out, "    int %s = ", sym.ident);
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
      case STMT_PRINT:
         {
            fprintf(gen->out, "    printf(\"%%s\\n\", ");
            gen_expr(gen, stmt->stmt_exit->expr);
            fprintf(gen->out, ");\n");
         }
         break;
      case STMT_ASSIGN:
         {
            for (size_t i = 0; i < gen->table.size; i++) {
               if (!strcmp(gen->table.items[i].ident, stmt->stmt_assign->ident.value)) {
                  Symbol sym = { stmt->stmt_assign->ident.value };
                  fprintf(gen->out, "    %s = ", sym.ident);
                  gen_expr(gen, stmt->stmt_assign->expr);
                  fprintf(gen->out, ";\n");
                  return;
               }
            }
            EXIT("[generator]: Undefined identifier `%s`. Assignment failed.\n", stmt->stmt_assign->ident.value);
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
   fprintf(gen->out, "#include <stdio.h>\n\n");
   fprintf(gen->out, "int main(void) {\n");
   for (size_t i = 0; i < gen->stmts->size; i++) {
      gen_stmt(gen, &gen->stmts->items[i]);
   }
   fprintf(gen->out, "}\n");
}
