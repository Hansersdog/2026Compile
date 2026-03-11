#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

int has_error = 0;
int last_lex_error_line = -1;
int last_syn_error_line = -1;
Node* syntax_tree_root = NULL;

extern FILE* yyin;
int yyparse(void);

int main(int argc, char** argv) {
  if (argc <= 1) {
    return 1;
  }

  yyin = fopen(argv[1], "r");
  if (!yyin) {
    perror(argv[1]);
    return 1;
  }

  yyparse();
  fclose(yyin);

  if (!has_error && syntax_tree_root) {
    node_print(syntax_tree_root);
  }

  return 0;
}
