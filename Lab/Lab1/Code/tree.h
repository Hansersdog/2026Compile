#ifndef TREE_H
#define TREE_H

typedef enum NodeValueKind {
  NODE_VAL_NONE = 0,
  NODE_VAL_STR = 1,
  NODE_VAL_INT = 2,
  NODE_VAL_FLOAT = 3
} NodeValueKind;

typedef struct Node {
  char* name;
  int line;
  int is_token;
  NodeValueKind value_kind;
  char* sval;
  int ival;
  float fval;
  struct Node* first_child;
  struct Node* next_sibling;
} Node;

Node* node_new(const char* name, int line, int is_token, int child_count, ...);
Node* node_token_none(const char* name, int line);
Node* node_token_str(const char* name, int line, const char* value);
Node* node_token_int(const char* name, int line, int value);
Node* node_token_float(const char* name, int line, float value);
void node_print(Node* root);

#endif
