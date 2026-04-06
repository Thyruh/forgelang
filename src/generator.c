#include "./generator.h"
#include <string.h>

static inline void gen_expr(Generator* gen, NodeExpr* expr) {
   switch (expr->type) {
      case EXPR_INT_LIT:
         fprintf(gen->out, "%s", expr->value.int_lit.intLit.value);
         break;
      case EXPR_IDENT:
         fprintf(gen->out, "%s", expr->value.ident.ident.value);
         break;
   }
}

static inline void gen_stmt(Generator* gen, NodeStmt* stmt) {
    switch (stmt->type) {
        case STMT_LET: // TODO check for redefinition
            for (size_t i = 0; i < gen->table.size; i++) {
               if (strcmp(gen->table.items[i].ident, stmt->stmt_let.ident.value) == 0) {
                  printf("Redefinition of %s\n", stmt->stmt_let.ident.value);
                  (void)system("rm out.c");
                  exit(1);
               }
            }
            Symbol sym = { stmt->stmt_let.ident.value };
            da_append(&gen->table, sym);
            fprintf(gen->out, "    int %s = ", stmt->stmt_let.ident.value);
            gen_expr(gen, &stmt->stmt_let.expr);
            fprintf(gen->out, ";\n");
            break;
        case STMT_EXIT:
            fprintf(gen->out, "    return ");
            gen_expr(gen, &stmt->stmt_exit.expr);
            fprintf(gen->out, ";\n");
            break;
    }
}

Generator Generator_create(NodeProg* prog, FILE* out) {
    Generator gen = {0};
    gen.prog = prog;
    gen.out = out;
    return gen;
}

void gen_prog(Generator* gen) {
    fprintf(gen->out, "// Generated automatically by the forgelang compiler\n");
    fprintf(gen->out, "#include <stdlib.h>\n\n");
    fprintf(gen->out, "int main(void) {\n");
    for (size_t i = 0; i < gen->prog->size; i++) {
        gen_stmt(gen, &gen->prog->items[i]);
    }
    fprintf(gen->out, "    return 0;\n");
    fprintf(gen->out, "}\n");
}
