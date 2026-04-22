#include "./generator.h"
#include "./common/tokenhandlers.h"
#include <string.h>

Generator Generator_create(NodeProg* stmts, FILE* out) {
  Generator gen = {0};
  gen.stmts = stmts;
  gen.out = out;
  return gen;
}

// TODO Implement string da_append/pascal string backend

static inline void gen_term(Generator* gen, NodeTerm* term) {
  switch (term->type) {
  case TERM_INT_LIT: 
    {
      fprintf(gen->out, "%s", term->value.int_lit->int_lit.value);
    } break;
  case TERM_STRING_LIT: 
    {
      fprintf(gen->out, "\"%s\"", term->value.string_lit->string_lit.value);
    } break;
  case TERM_CHAR_LIT: 
    {
      fprintf(gen->out, "'%s'", term->value.char_lit->char_lit.value);
    } break;
  case TERM_IDENT: 
    {
      fprintf(gen->out, "%s", term->value.ident->ident.value);
    } break;
  }
}

static inline void gen_expr(Generator* gen, NodeExpr* expr) {
  switch (expr->type) {
  case EXPR_TERM: 
    {
      gen_term(gen, expr->value.term);
    } break;
  case EXPR_BIN_EXPR: 
    {
      switch (expr->value.bin_expr->type) {
      case EXPR_ADD: 
        {
          gen_expr(gen, expr->value.bin_expr->lhs);
          fprintf(gen->out, "+");
          gen_expr(gen, expr->value.bin_expr->rhs);
        } break;
      case EXPR_MULTI: 
        {
          gen_expr(gen, expr->value.bin_expr->lhs);
          fprintf(gen->out, "*");
          gen_expr(gen, expr->value.bin_expr->rhs);
        } break;
      case EXPR_SUBTR: 
        {
          gen_expr(gen, expr->value.bin_expr->lhs);
          fprintf(gen->out, "-");
          gen_expr(gen, expr->value.bin_expr->rhs);
        } break;
      case EXPR_DIVIDE: 
        {
          gen_expr(gen, expr->value.bin_expr->lhs);
          fprintf(gen->out, "/");
          gen_expr(gen, expr->value.bin_expr->rhs);
        } break;
      }
    } break;
  }
}

// TODO: integer overflow — exit instead of wrapping
static inline void gen_stmt(Generator* gen, NodeStmt* stmt) {
  switch (stmt->type) {
  case STMT_LET: 
    {
      // TODO make sure const actually emits const
      fprintf(gen->out, "    %s %s = ", get_type(stmt->stmt_let->type), stmt->stmt_let->ident.value);
      gen_expr(gen, stmt->stmt_let->expr);
      fprintf(gen->out, ";\n");
    } break;
  case STMT_EXIT: 
    {
      fprintf(gen->out, "    return ");
      gen_expr(gen, stmt->stmt_exit->expr);
      fprintf(gen->out, ";\n");
    } break;
  case STMT_PRINTLN: 
    {
      fprintf(gen->out, "    puts(");
      gen_expr(gen, stmt->stmt_print->expr);
      fprintf(gen->out, ");\n");
    } break;
  case STMT_ASSIGN: 
    {
      fprintf(gen->out, "    %s = ", stmt->stmt_assign->ident.value);
      gen_expr(gen, stmt->stmt_assign->expr);
      fprintf(gen->out, ";\n");
    } break;
  }
}

void gen_prog(Generator* gen) {
  fprintf(gen->out, "// Generated automatically by the forgelang compiler\n");
  fprintf(gen->out, "#include <stdio.h>\n\n"); // TODO Only emit the libraries that are necessary
  fprintf(gen->out, "int main(void) {\n");
  for (size_t i = 0; i < gen->stmts->size; i++) {
    gen_stmt(gen, &gen->stmts->items[i]);
  }
  fprintf(gen->out, "}\n");
}
