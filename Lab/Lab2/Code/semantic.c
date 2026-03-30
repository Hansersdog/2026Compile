#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int has_error;

typedef enum TypeKind {
  TYPE_INVALID = 0,
  TYPE_BASIC = 1,
  TYPE_ARRAY = 2,
  TYPE_STRUCT = 3
} TypeKind;

typedef enum BasicKind {
  BASIC_INT = 1,
  BASIC_FLOAT = 2
} BasicKind;

typedef struct Type Type;
typedef struct Field Field;
typedef struct VarSymbol VarSymbol;
typedef struct StructSymbol StructSymbol;
typedef struct FunctionSymbol FunctionSymbol;
typedef struct ErrorLine ErrorLine;

struct Type {
  TypeKind kind;
  union {
    BasicKind basic;
    struct {
      Type* elem;
      int size;
    } array;
    struct {
      char* name;
      Field* fields;
    } structure;
  } info;
};

struct Field {
  char* name;
  Type* type;
  int line;
  Field* next;
};

struct VarSymbol {
  char* name;
  Type* type;
  int line;
  VarSymbol* next;
};

struct StructSymbol {
  char* name;
  Type* type;
  int line;
  StructSymbol* next;
};

struct FunctionSymbol {
  char* name;
  Type* return_type;
  Field* params;
  int declared_line;
  int defined_line;
  int has_declaration;
  int has_definition;
  FunctionSymbol* next;
};

struct ErrorLine {
  int line;
  ErrorLine* next;
};

typedef struct ExprResult {
  Type* type;
  int is_lvalue;
} ExprResult;

static Type builtin_int_type = {TYPE_BASIC, {.basic = BASIC_INT}};
static Type builtin_float_type = {TYPE_BASIC, {.basic = BASIC_FLOAT}};
static VarSymbol* var_symbols = NULL;
static StructSymbol* struct_symbols = NULL;
static FunctionSymbol* function_symbols = NULL;
static ErrorLine* error_lines = NULL;
static int anonymous_struct_id = 0;

static char* xstrdup(const char* s) {
  size_t n = strlen(s);
  char* p = (char*)malloc(n + 1);
  if (!p) exit(1);
  memcpy(p, s, n + 1);
  return p;
}

static int child_count(Node* node) {
  int count = 0;
  for (Node* cur = node ? node->first_child : NULL; cur; cur = cur->next_sibling) {
    count++;
  }
  return count;
}

static Node* child_at(Node* node, int index) {
  int i = 0;
  for (Node* cur = node ? node->first_child : NULL; cur; cur = cur->next_sibling) {
    if (i == index) return cur;
    i++;
  }
  return NULL;
}

static int node_is(Node* node, const char* name) {
  return node && node->name && strcmp(node->name, name) == 0;
}

static Node* find_named_child(Node* node, const char* name) {
  for (Node* cur = node ? node->first_child : NULL; cur; cur = cur->next_sibling) {
    if (node_is(cur, name)) return cur;
  }
  return NULL;
}

static Type* invalid_type(void) {
  return NULL;
}

static int is_invalid_type(Type* type) {
  return type == NULL || type->kind == TYPE_INVALID;
}

static Type* type_basic(BasicKind kind) {
  return kind == BASIC_INT ? &builtin_int_type : &builtin_float_type;
}

static Type* type_array(Type* elem, int size) {
  Type* type = (Type*)calloc(1, sizeof(Type));
  if (!type) exit(1);
  type->kind = TYPE_ARRAY;
  type->info.array.elem = elem;
  type->info.array.size = size;
  return type;
}

static Type* type_struct(const char* name, Field* fields) {
  Type* type = (Type*)calloc(1, sizeof(Type));
  if (!type) exit(1);
  type->kind = TYPE_STRUCT;
  type->info.structure.name = xstrdup(name);
  type->info.structure.fields = fields;
  return type;
}

static Field* field_new(const char* name, Type* type, int line) {
  Field* field = (Field*)calloc(1, sizeof(Field));
  if (!field) exit(1);
  field->name = xstrdup(name);
  field->type = type;
  field->line = line;
  return field;
}

static Field* field_append(Field* head, Field* item) {
  if (!head) return item;
  Field* tail = head;
  while (tail->next) tail = tail->next;
  tail->next = item;
  return head;
}

static Field* field_find(Field* head, const char* name) {
  for (Field* cur = head; cur; cur = cur->next) {
    if (strcmp(cur->name, name) == 0) return cur;
  }
  return NULL;
}

static VarSymbol* find_var(const char* name) {
  for (VarSymbol* cur = var_symbols; cur; cur = cur->next) {
    if (strcmp(cur->name, name) == 0) return cur;
  }
  return NULL;
}

static StructSymbol* find_struct_symbol(const char* name) {
  for (StructSymbol* cur = struct_symbols; cur; cur = cur->next) {
    if (strcmp(cur->name, name) == 0) return cur;
  }
  return NULL;
}

static FunctionSymbol* find_function(const char* name) {
  for (FunctionSymbol* cur = function_symbols; cur; cur = cur->next) {
    if (strcmp(cur->name, name) == 0) return cur;
  }
  return NULL;
}

static void add_var_symbol(const char* name, Type* type, int line) {
  VarSymbol* symbol = (VarSymbol*)calloc(1, sizeof(VarSymbol));
  if (!symbol) exit(1);
  symbol->name = xstrdup(name);
  symbol->type = type;
  symbol->line = line;
  symbol->next = var_symbols;
  var_symbols = symbol;
}

static void add_struct_symbol(const char* name, Type* type, int line) {
  StructSymbol* symbol = (StructSymbol*)calloc(1, sizeof(StructSymbol));
  if (!symbol) exit(1);
  symbol->name = xstrdup(name);
  symbol->type = type;
  symbol->line = line;
  symbol->next = struct_symbols;
  struct_symbols = symbol;
}

static FunctionSymbol* add_function_symbol(const char* name, Type* return_type, Field* params, int line, int defined) {
  FunctionSymbol* symbol = (FunctionSymbol*)calloc(1, sizeof(FunctionSymbol));
  if (!symbol) exit(1);
  symbol->name = xstrdup(name);
  symbol->return_type = return_type;
  symbol->params = params;
  symbol->declared_line = line;
  symbol->defined_line = defined ? line : 0;
  symbol->has_declaration = 1;
  symbol->has_definition = defined;
  symbol->next = function_symbols;
  function_symbols = symbol;
  return symbol;
}

static int line_already_reported(int line) {
  for (ErrorLine* cur = error_lines; cur; cur = cur->next) {
    if (cur->line == line) return 1;
  }
  return 0;
}

static void remember_error_line(int line) {
  ErrorLine* item = (ErrorLine*)calloc(1, sizeof(ErrorLine));
  if (!item) exit(1);
  item->line = line;
  item->next = error_lines;
  error_lines = item;
}

static void semantic_error(int type, int line, const char* msg, const char* name) {
  if (line <= 0 || line_already_reported(line)) return;
  remember_error_line(line);
  has_error = 1;
  if (name) {
    printf("Error type %d at Line %d: %s \"%s\".\n", type, line, msg, name);
  } else {
    printf("Error type %d at Line %d: %s.\n", type, line, msg);
  }
}

static int type_equal(Type* lhs, Type* rhs) {
  if (lhs == rhs) return 1;
  if (is_invalid_type(lhs) || is_invalid_type(rhs)) return 0;
  if (lhs->kind != rhs->kind) return 0;
  if (lhs->kind == TYPE_BASIC) {
    return lhs->info.basic == rhs->info.basic;
  }
  if (lhs->kind == TYPE_ARRAY) {
    return type_equal(lhs->info.array.elem, rhs->info.array.elem);
  }
  if (lhs->kind == TYPE_STRUCT) {
    return strcmp(lhs->info.structure.name, rhs->info.structure.name) == 0;
  }
  return 0;
}

static int assignable(Type* lhs, Type* rhs) {
  if (is_invalid_type(lhs) || is_invalid_type(rhs)) return 0;
  if (!type_equal(lhs, rhs)) return 0;
  return lhs->kind == TYPE_BASIC || lhs->kind == TYPE_STRUCT;
}

static int is_int_type(Type* type) {
  return !is_invalid_type(type) && type->kind == TYPE_BASIC && type->info.basic == BASIC_INT;
}

static int is_numeric_type(Type* type) {
  return !is_invalid_type(type) && type->kind == TYPE_BASIC;
}

static int same_numeric_type(Type* lhs, Type* rhs) {
  return is_numeric_type(lhs) && is_numeric_type(rhs) && lhs->info.basic == rhs->info.basic;
}

static int function_signature_equal(Type* return_type, Field* lhs, Type* other_return_type, Field* rhs) {
  if (!type_equal(return_type, other_return_type)) return 0;
  while (lhs && rhs) {
    if (!type_equal(lhs->type, rhs->type)) return 0;
    lhs = lhs->next;
    rhs = rhs->next;
  }
  return lhs == NULL && rhs == NULL;
}

static const char* hidden_struct_name(void) {
  static char buf[64];
  anonymous_struct_id++;
  snprintf(buf, sizeof(buf), "__anonymous_%d", anonymous_struct_id);
  return buf;
}

static Type* analyze_specifier(Node* specifier);
static Field* analyze_struct_deflist(Node* def_list);
static void analyze_extdef_list(Node* extdef_list);
static void analyze_extdef(Node* extdef);
static void analyze_extdeclist(Node* extdeclist, Type* base_type);
static Type* build_var_type(Node* vardec, Type* base_type, char** name_out, int* line_out);
static Field* build_param_list(Node* fun_dec);
static void define_params(Field* params);
static void analyze_compst(Node* compst, Type* return_type);
static void analyze_deflist(Node* def_list, int in_struct, Field** struct_fields);
static void analyze_stmtlist(Node* stmt_list, Type* return_type);
static void analyze_stmt(Node* stmt, Type* return_type);
static ExprResult analyze_exp(Node* exp);
static int match_args(Field* params, Node* args);

static Type* analyze_specifier(Node* specifier) {
  Node* child = child_at(specifier, 0);
  if (node_is(child, "TYPE")) {
    if (child->sval && strcmp(child->sval, "int") == 0) return type_basic(BASIC_INT);
    return type_basic(BASIC_FLOAT);
  }

  Node* struct_specifier = child;
  if (!struct_specifier) return invalid_type();

  if (child_count(struct_specifier) == 2) {
    Node* tag = child_at(struct_specifier, 1);
    Node* id = child_at(tag, 0);
    StructSymbol* symbol = id && id->sval ? find_struct_symbol(id->sval) : NULL;
    if (!symbol) {
      semantic_error(17, id ? id->line : struct_specifier->line, "Undefined structure", id ? id->sval : NULL);
      return invalid_type();
    }
    return symbol->type;
  }

  Node* opt_tag = find_named_child(struct_specifier, "OptTag");
  Node* id = opt_tag ? child_at(opt_tag, 0) : NULL;
  Field* fields = analyze_struct_deflist(find_named_child(struct_specifier, "DefList"));
  const char* name = id && id->sval ? id->sval : hidden_struct_name();
  Type* type = type_struct(name, fields);

  if (id && id->sval) {
    if (find_struct_symbol(id->sval) || find_var(id->sval)) {
      semantic_error(16, id->line, "Duplicated name", id->sval);
    } else {
      add_struct_symbol(id->sval, type, id->line);
    }
  }

  return type;
}

static void handle_variable_definition(Type* type, const char* name, int line) {
  if (is_invalid_type(type)) return;
  if (find_var(name) || find_struct_symbol(name)) {
    semantic_error(3, line, "Redefined variable", name);
    return;
  }
  add_var_symbol(name, type, line);
}

static void analyze_extdeclist(Node* extdeclist, Type* base_type) {
  if (!extdeclist) return;
  Node* vardec = child_at(extdeclist, 0);
  char* name = NULL;
  int line = 0;
  Type* type = build_var_type(vardec, base_type, &name, &line);
  if (name) handle_variable_definition(type, name, line);
  if (child_count(extdeclist) == 3) {
    analyze_extdeclist(child_at(extdeclist, 2), base_type);
  }
}

static Type* build_var_type(Node* vardec, Type* base_type, char** name_out, int* line_out) {
  if (!vardec) return invalid_type();
  if (child_count(vardec) == 1) {
    Node* id = child_at(vardec, 0);
    if (name_out) *name_out = id ? id->sval : NULL;
    if (line_out) *line_out = id ? id->line : vardec->line;
    return base_type;
  }
  Type* inner = build_var_type(child_at(vardec, 0), base_type, name_out, line_out);
  Node* size_node = child_at(vardec, 2);
  return type_array(inner, size_node ? size_node->ival : 0);
}

static void analyze_dec(Node* dec, Type* base_type, int in_struct, Field** struct_fields) {
  Node* vardec = child_at(dec, 0);
  char* name = NULL;
  int line = 0;
  Type* type = build_var_type(vardec, base_type, &name, &line);
  if (!name || is_invalid_type(type)) return;

  if (in_struct) {
    if (field_find(*struct_fields, name)) {
      semantic_error(15, line, "Redefined field", name);
    } else {
      *struct_fields = field_append(*struct_fields, field_new(name, type, line));
    }
    if (child_count(dec) == 3) {
      semantic_error(15, line, "Initialized field", name);
    }
    return;
  }

  if (find_var(name) || find_struct_symbol(name)) {
    semantic_error(3, line, "Redefined variable", name);
  } else {
    add_var_symbol(name, type, line);
  }

  if (child_count(dec) == 3) {
    ExprResult init = analyze_exp(child_at(dec, 2));
    if (!is_invalid_type(type) && !is_invalid_type(init.type) && !assignable(type, init.type)) {
      semantic_error(5, line, "Type mismatched for assignment", NULL);
    }
  }
}

static void analyze_declist(Node* declist, Type* base_type, int in_struct, Field** struct_fields) {
  if (!declist) return;
  analyze_dec(child_at(declist, 0), base_type, in_struct, struct_fields);
  if (child_count(declist) == 3) {
    analyze_declist(child_at(declist, 2), base_type, in_struct, struct_fields);
  }
}

static void analyze_def(Node* def, int in_struct, Field** struct_fields) {
  if (!def) return;
  Type* base_type = analyze_specifier(child_at(def, 0));
  analyze_declist(child_at(def, 1), base_type, in_struct, struct_fields);
}

static void analyze_deflist(Node* def_list, int in_struct, Field** struct_fields) {
  if (!def_list) return;
  analyze_def(child_at(def_list, 0), in_struct, struct_fields);
  analyze_deflist(child_at(def_list, 1), in_struct, struct_fields);
}

static Field* analyze_struct_deflist(Node* def_list) {
  Field* fields = NULL;
  analyze_deflist(def_list, 1, &fields);
  return fields;
}

static Field* build_param_from_paramdec(Node* param_dec) {
  Type* spec_type = analyze_specifier(child_at(param_dec, 0));
  char* name = NULL;
  int line = 0;
  Type* type = build_var_type(child_at(param_dec, 1), spec_type, &name, &line);
  if (!name || is_invalid_type(type)) return NULL;
  return field_new(name, type, line);
}

static Field* build_varlist(Node* var_list) {
  if (!var_list) return NULL;
  Field* first = build_param_from_paramdec(child_at(var_list, 0));
  if (child_count(var_list) == 3) {
    Field* rest = build_varlist(child_at(var_list, 2));
    if (first) first->next = rest;
    return first ? first : rest;
  }
  return first;
}

static Field* build_param_list(Node* fun_dec) {
  if (!fun_dec) return NULL;
  if (child_count(fun_dec) == 4) {
    return build_varlist(child_at(fun_dec, 2));
  }
  return NULL;
}

static void define_params(Field* params) {
  for (Field* cur = params; cur; cur = cur->next) {
    if (find_var(cur->name) || find_struct_symbol(cur->name)) {
      semantic_error(3, cur->line, "Redefined variable", cur->name);
    } else {
      add_var_symbol(cur->name, cur->type, cur->line);
    }
  }
}

static void analyze_function_definition(Node* extdef, Type* return_type) {
  Node* fun_dec = child_at(extdef, 1);
  Node* compst = child_at(extdef, 2);
  Node* id = child_at(fun_dec, 0);
  Field* params = build_param_list(fun_dec);
  if (!id || !id->sval || is_invalid_type(return_type)) return;

  FunctionSymbol* symbol = find_function(id->sval);
  if (!symbol) {
    symbol = add_function_symbol(id->sval, return_type, params, id->line, 1);
  } else if (symbol->has_definition) {
    semantic_error(4, id->line, "Redefined function", id->sval);
    return;
  } else {
    if (!function_signature_equal(symbol->return_type, symbol->params, return_type, params)) {
      semantic_error(19, id->line, "Inconsistent declaration of function", id->sval);
      return;
    }
    symbol->has_definition = 1;
    symbol->defined_line = id->line;
  }

  define_params(params);
  analyze_compst(compst, return_type);
}

static void analyze_function_declaration(Node* extdef, Type* return_type) {
  Node* fun_dec = child_at(extdef, 1);
  Node* id = child_at(fun_dec, 0);
  Field* params = build_param_list(fun_dec);
  if (!id || !id->sval || is_invalid_type(return_type)) return;

  FunctionSymbol* symbol = find_function(id->sval);
  if (!symbol) {
    add_function_symbol(id->sval, return_type, params, id->line, 0);
    return;
  }
  if (!function_signature_equal(symbol->return_type, symbol->params, return_type, params)) {
    semantic_error(19, id->line, "Inconsistent declaration of function", id->sval);
  }
}

static void analyze_extdef(Node* extdef) {
  if (!extdef) return;
  Type* spec_type = analyze_specifier(child_at(extdef, 0));
  Node* second = child_at(extdef, 1);
  Node* third = child_at(extdef, 2);

  if (node_is(second, "ExtDecList")) {
    analyze_extdeclist(second, spec_type);
    return;
  }
  if (node_is(second, "FunDec") && node_is(third, "CompSt")) {
    analyze_function_definition(extdef, spec_type);
    return;
  }
  if (node_is(second, "FunDec") && node_is(third, "SEMI")) {
    analyze_function_declaration(extdef, spec_type);
  }
}

static void analyze_extdef_list(Node* extdef_list) {
  if (!extdef_list) return;
  analyze_extdef(child_at(extdef_list, 0));
  analyze_extdef_list(child_at(extdef_list, 1));
}

static void analyze_compst(Node* compst, Type* return_type) {
  if (!compst) return;
  analyze_deflist(find_named_child(compst, "DefList"), 0, NULL);
  analyze_stmtlist(find_named_child(compst, "StmtList"), return_type);
}

static void analyze_stmtlist(Node* stmt_list, Type* return_type) {
  if (!stmt_list) return;
  analyze_stmt(child_at(stmt_list, 0), return_type);
  analyze_stmtlist(child_at(stmt_list, 1), return_type);
}

static void ensure_condition_int(ExprResult result, int line) {
  if (!is_invalid_type(result.type) && !is_int_type(result.type)) {
    semantic_error(7, line, "Type mismatched for operands", NULL);
  }
}

static void analyze_stmt(Node* stmt, Type* return_type) {
  if (!stmt) return;
  Node* first = child_at(stmt, 0);
  if (!first) return;

  if (node_is(first, "Exp")) {
    analyze_exp(first);
    return;
  }
  if (node_is(first, "CompSt")) {
    analyze_compst(first, return_type);
    return;
  }
  if (node_is(first, "RETURN")) {
    ExprResult result = analyze_exp(child_at(stmt, 1));
    if (!is_invalid_type(result.type) && !type_equal(result.type, return_type)) {
      semantic_error(8, first->line, "Type mismatched for return", NULL);
    }
    return;
  }
  if (node_is(first, "IF")) {
    ExprResult cond = analyze_exp(child_at(stmt, 2));
    ensure_condition_int(cond, child_at(stmt, 2)->line);
    analyze_stmt(child_at(stmt, 4), return_type);
    if (child_count(stmt) == 7) {
      analyze_stmt(child_at(stmt, 6), return_type);
    }
    return;
  }
  if (node_is(first, "WHILE")) {
    ExprResult cond = analyze_exp(child_at(stmt, 2));
    ensure_condition_int(cond, child_at(stmt, 2)->line);
    analyze_stmt(child_at(stmt, 4), return_type);
  }
}

static int match_args(Field* params, Node* args) {
  if (!params && !args) return 1;
  if (!params || !args) return 0;

  ExprResult expr = analyze_exp(child_at(args, 0));
  if (!is_invalid_type(expr.type) && !type_equal(params->type, expr.type)) {
    return 0;
  }

  if (child_count(args) == 3) {
    return match_args(params->next, child_at(args, 2));
  }
  return params->next == NULL;
}

static ExprResult make_expr(Type* type, int is_lvalue) {
  ExprResult result;
  result.type = type;
  result.is_lvalue = is_lvalue;
  return result;
}

static ExprResult analyze_id_exp(Node* id) {
  if (!id || !id->sval) return make_expr(invalid_type(), 0);
  VarSymbol* var = find_var(id->sval);
  if (!var) {
    semantic_error(1, id->line, "Undefined variable", id->sval);
    return make_expr(invalid_type(), 0);
  }
  return make_expr(var->type, 1);
}

static ExprResult analyze_call_exp(Node* exp) {
  Node* id = child_at(exp, 0);
  FunctionSymbol* function = id && id->sval ? find_function(id->sval) : NULL;
  VarSymbol* var = id && id->sval ? find_var(id->sval) : NULL;
  Node* args = child_count(exp) == 4 ? child_at(exp, 2) : NULL;

  if (!function) {
    if (var) {
      semantic_error(11, id->line, "Not a function", id ? id->sval : NULL);
    } else {
      semantic_error(2, id->line, "Undefined function", id ? id->sval : NULL);
    }
    return make_expr(invalid_type(), 0);
  }
  if (!match_args(function->params, args)) {
    semantic_error(9, id->line, "Function is not applicable for arguments", id->sval);
    return make_expr(function->return_type, 0);
  }
  return make_expr(function->return_type, 0);
}

static ExprResult analyze_exp(Node* exp) {
  if (!exp) return make_expr(invalid_type(), 0);
  int count = child_count(exp);

  if (count == 1) {
    Node* first = child_at(exp, 0);
    if (node_is(first, "ID")) return analyze_id_exp(first);
    if (node_is(first, "INT")) return make_expr(type_basic(BASIC_INT), 0);
    if (node_is(first, "FLOAT")) return make_expr(type_basic(BASIC_FLOAT), 0);
    return make_expr(invalid_type(), 0);
  }

  if (count == 2) {
    Node* op = child_at(exp, 0);
    ExprResult rhs = analyze_exp(child_at(exp, 1));
    if (node_is(op, "MINUS")) {
      if (!is_invalid_type(rhs.type) && !is_numeric_type(rhs.type)) {
        semantic_error(7, op->line, "Type mismatched for operands", NULL);
        return make_expr(invalid_type(), 0);
      }
      return make_expr(rhs.type, 0);
    }
    if (node_is(op, "NOT")) {
      if (!is_invalid_type(rhs.type) && !is_int_type(rhs.type)) {
        semantic_error(7, op->line, "Type mismatched for operands", NULL);
        return make_expr(invalid_type(), 0);
      }
      return make_expr(type_basic(BASIC_INT), 0);
    }
  }

  if (count == 3) {
    Node* first = child_at(exp, 0);
    Node* second = child_at(exp, 1);
    Node* third = child_at(exp, 2);

    if (node_is(first, "LP")) {
      return analyze_exp(second);
    }

    if (node_is(first, "ID") && node_is(second, "LP")) {
      return analyze_call_exp(exp);
    }

    if (node_is(second, "DOT")) {
      ExprResult base = analyze_exp(first);
      if (!is_invalid_type(base.type) && base.type->kind != TYPE_STRUCT) {
        semantic_error(13, second->line, "Illegal use of \".\"", NULL);
        return make_expr(invalid_type(), 0);
      }
      if (is_invalid_type(base.type) || base.type->kind != TYPE_STRUCT) {
        return make_expr(invalid_type(), 0);
      }
      if (!third || !third->sval) {
        return make_expr(invalid_type(), 0);
      }
      Field* field = field_find(base.type->info.structure.fields, third->sval);
      if (!field) {
        semantic_error(14, third->line, "Non-existent field", third->sval);
        return make_expr(invalid_type(), 0);
      }
      return make_expr(field->type, 1);
    }

    ExprResult lhs = analyze_exp(first);
    ExprResult rhs = analyze_exp(third);

    if (node_is(second, "ASSIGNOP")) {
      if (!lhs.is_lvalue) {
        semantic_error(6, second->line, "The left-hand side of an assignment must be a variable", NULL);
        return make_expr(lhs.type, 0);
      }
      if (!is_invalid_type(lhs.type) && !is_invalid_type(rhs.type) && !assignable(lhs.type, rhs.type)) {
        semantic_error(5, second->line, "Type mismatched for assignment", NULL);
      }
      return make_expr(lhs.type, 0);
    }

    if (node_is(second, "AND") || node_is(second, "OR")) {
      if (!is_invalid_type(lhs.type) && !is_invalid_type(rhs.type) && (!is_int_type(lhs.type) || !is_int_type(rhs.type))) {
        semantic_error(7, second->line, "Type mismatched for operands", NULL);
        return make_expr(invalid_type(), 0);
      }
      return make_expr(type_basic(BASIC_INT), 0);
    }

    if (node_is(second, "RELOP")) {
      if (!is_invalid_type(lhs.type) && !is_invalid_type(rhs.type) && !same_numeric_type(lhs.type, rhs.type)) {
        semantic_error(7, second->line, "Type mismatched for operands", NULL);
        return make_expr(invalid_type(), 0);
      }
      return make_expr(type_basic(BASIC_INT), 0);
    }

    if (node_is(second, "PLUS") || node_is(second, "MINUS") || node_is(second, "STAR") || node_is(second, "DIV")) {
      if (!is_invalid_type(lhs.type) && !is_invalid_type(rhs.type) && !same_numeric_type(lhs.type, rhs.type)) {
        semantic_error(7, second->line, "Type mismatched for operands", NULL);
        return make_expr(invalid_type(), 0);
      }
      return make_expr(lhs.type, 0);
    }
  }

  if (count == 4) {
    Node* second = child_at(exp, 1);
    if (node_is(second, "LP")) {
      return analyze_call_exp(exp);
    }

    if (node_is(second, "LB")) {
      ExprResult array = analyze_exp(child_at(exp, 0));
      ExprResult index = analyze_exp(child_at(exp, 2));
      if (!is_invalid_type(array.type) && array.type->kind != TYPE_ARRAY) {
        semantic_error(10, second->line, "Not an array", NULL);
        return make_expr(invalid_type(), 0);
      }
      if (!is_invalid_type(index.type) && !is_int_type(index.type)) {
        semantic_error(12, child_at(exp, 2)->line, "Not an integer", NULL);
        return make_expr(invalid_type(), 0);
      }
      if (is_invalid_type(array.type) || array.type->kind != TYPE_ARRAY) {
        return make_expr(invalid_type(), 0);
      }
      return make_expr(array.type->info.array.elem, 1);
    }
  }

  return make_expr(invalid_type(), 0);
}

static void check_undefined_functions(void) {
  for (FunctionSymbol* cur = function_symbols; cur; cur = cur->next) {
    if (cur->has_declaration && !cur->has_definition) {
      semantic_error(18, cur->declared_line, "Undefined function", cur->name);
    }
  }
}

void semantic_analyze(Node* root) {
  analyze_extdef_list(child_at(root, 0));
  check_undefined_functions();
}
