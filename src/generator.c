#include "./generator.h"
#include <string.h>

static inline void gen_expr(Generator* gen, NodeExpr* expr) {
   switch (expr->type) {
      case EXPR_INT_LIT:
         fprintf(gen->out, "%s", expr->value.int_lit->int_lit.value);
         break;
      case EXPR_IDENT:
         for (size_t i = 0; i < gen->table.size; i++) {
            if (strcmp(gen->table.items[i].ident, expr->value.ident->ident.value) == 0) {
               fprintf(gen->out, "%s", expr->value.ident->ident.value);
               return;
            }
         }
         printf("[generator]: Unknown identifier: `%s`\n", expr->value.ident->ident.value);
         exit(1);
         break;
   }
}

static inline void gen_stmt(Generator* gen, NodeStmt* stmt) {
   switch (stmt->type) {
      case STMT_LET:
         {
            for (size_t i = 0; i < gen->table.size; i++) {
               if (strcmp(gen->table.items[i].ident, stmt->stmt_let->ident.value) == 0) {
                  printf("[generator]: Redefinition of `%s`\n", stmt->stmt_let->ident.value);
                  (void)system("rm out.c"); // TODO abstract the clean up and exit into a separate function/macro
                  exit(1);
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
      case STMT_ASSIGN:
         {
            for (size_t i = 0; i < gen->table.size; i++) {
               if (strcmp(gen->table.items[i].ident, stmt->stmt_assign->ident.value) == 0) {
                  Symbol sym = { stmt->stmt_assign->ident.value };
                  fprintf(gen->out, "    %s = ", sym.ident);
                  gen_expr(gen, stmt->stmt_assign->expr);
                  fprintf(gen->out, ";\n");
                  return;
               }
            }
            printf("[generator]: Undefined identifier `%s`\n", stmt->stmt_let->ident.value);
            (void)system("rm out.c");
            exit(1);
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
   fprintf(gen->out, "#include <stdlib.h>\n\n");
   fprintf(gen->out, "int main(void) {\n");
   for (size_t i = 0; i < gen->stmts->size; i++) {
      gen_stmt(gen, &gen->stmts->items[i]);
   }
   fprintf(gen->out, "}\n");
}
