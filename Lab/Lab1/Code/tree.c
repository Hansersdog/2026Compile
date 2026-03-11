#include "tree.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* xstrdup(const char* s) {
  if (!s) return NULL;
  size_t n = strlen(s);
  char* p = (char*)malloc(n + 1);
  if (!p) exit(1);
  memcpy(p, s, n + 1);
  return p;
}

Node* node_new(const char* name, int line, int is_token, int child_count, ...) {
  Node* n = (Node*)calloc(1, sizeof(Node));
  if (!n) exit(1);
  n->name = xstrdup(name);
  n->line = line;
  n->is_token = is_token;

  va_list ap;
  va_start(ap, child_count);
  Node* last = NULL;
  for (int i = 0; i < child_count; i++) {
    Node* c = va_arg(ap, Node*);
    if (!c) continue;
    if (!n->first_child) {
      n->first_child = c;
      last = c;
    } else {
      last->next_sibling = c;
      last = c;
    }
  }
  va_end(ap);
  return n;
}

Node* node_token_none(const char* name, int line) {
  return node_new(name, line, 1, 0);
}

Node* node_token_str(const char* name, int line, const char* value) {
  Node* n = node_new(name, line, 1, 0);
  n->value_kind = NODE_VAL_STR;
  n->sval = xstrdup(value);
  return n;
}

Node* node_token_int(const char* name, int line, int value) {
  Node* n = node_new(name, line, 1, 0);
  n->value_kind = NODE_VAL_INT;
  n->ival = value;
  return n;
}

Node* node_token_float(const char* name, int line, float value) {
  Node* n = node_new(name, line, 1, 0);
  n->value_kind = NODE_VAL_FLOAT;
  n->fval = value;
  return n;
}

static void node_print_impl(Node* n, int indent) {
  if (!n) return;

  if (!n->is_token) {
    if (!n->first_child) return;
    printf("%*s%s (%d)\n", indent, "", n->name, n->line);
  } else {
    printf("%*s%s", indent, "", n->name);
    if (n->value_kind == NODE_VAL_STR) {
      printf(": %s", n->sval ? n->sval : "");
    } else if (n->value_kind == NODE_VAL_INT) {
      printf(": %d", n->ival);
    } else if (n->value_kind == NODE_VAL_FLOAT) {
      printf(": %f", n->fval);
    }
    printf("\n");
  }

  for (Node* c = n->first_child; c; c = c->next_sibling) {
    node_print_impl(c, indent + 2);
  }
}

void node_print(Node* root) {
  node_print_impl(root, 0);
}

