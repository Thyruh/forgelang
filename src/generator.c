#include "./generator.h"

void gen_expr(Generator* gen, NodeExpr* expr) {
   switch (expr->type) {
      case EXPR_INT_LIT:
         fprintf(gen->out, "%s", expr->value.int_lit.intLit.value);
         break;
      case EXPR_IDENT:
         fprintf(gen->out, "%s", expr->value.ident.ident.value);
         break;
   }
}

void gen_stmt(Generator* gen, NodeStmt* stmt) {
    switch (stmt->type) {
        case STMT_LET: // TODO check for redefinition
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
    fprintf(gen->out, "#include <stdlib.h>\n\n");
    fprintf(gen->out, "int main(void) {\n");
    for (size_t i = 0; i < gen->prog->size; i++) {
        gen_stmt(gen, &gen->prog->items[i]);
    }
    fprintf(gen->out, "    return 0;\n");
    fprintf(gen->out, "}\n");
}
