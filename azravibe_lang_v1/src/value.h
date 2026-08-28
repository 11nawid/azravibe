#ifndef AZR_VALUE_H
#define AZR_VALUE_H

struct ASTNode;
struct Environment;
struct GeneratorFrame;

typedef enum {
    VAL_INT,
    VAL_FLOAT,
    VAL_STRING,
    VAL_BOOL,
    VAL_NONE,
    VAL_LIST,
    VAL_TUPLE,
    VAL_DICT,
    VAL_SET,
    VAL_BYTES,
    VAL_FUNCTION,
    VAL_BUILTIN,
    VAL_MODULE,
    VAL_CLASS,
    VAL_INSTANCE,
    VAL_BOUND_METHOD,
    VAL_SUPER,
    VAL_CLASS_METHOD,
    VAL_STATIC_METHOD,
    VAL_PROPERTY,
    VAL_ITERATOR,
    VAL_GENERATOR,
    VAL_REGEX,
    VAL_DATETIME
} ValueType;

typedef struct Value {
    ValueType type;
    int refcount;
    union {
        long long int_val;
        double float_val;
        char *str_val;
        int bool_val;
        struct {
            struct Value **items;
            int count;
            int capacity;
        } list;
        struct {
            struct Value **items;
            int count;
            int capacity;
        } set;
        struct {
            char **keys;
            struct Value **values;
            int count;
            int capacity;
        } dict;
        struct {
            unsigned char *data;
            int count;
        } bytes;
        struct {
            char *name;
            char **params;
            struct ASTNode **defaults;
            int param_count;
            char *vararg;
            struct ASTNode *body;
            struct Environment *closure;
            struct Value *owner_class;
            int is_generator;
        } function;
        struct {
            char *name;
            struct Value *(*fn)(struct Value **, int);
        } builtin;
        struct {
            char *name;
            struct Environment *env;
        } module;
        struct {
            char *name;
            struct Environment *env;
            struct Value *base;
        } class_val;
        struct {
            char *class_name;
            struct Value *class_obj;
            struct Environment *fields;
        } instance;
        struct {
            struct Value *function;
            struct Value *receiver;
        } bound_method;
        struct {
            struct Value *class_obj;
            struct Value *receiver;
        } super_val;
        struct {
            struct Value *function;
        } class_method;
        struct {
            struct Value *function;
        } static_method;
        struct {
            struct Value *getter;
            struct Value *setter;
        } property;
        struct {
            int kind;
            int index;
            struct Value *target;
        } iterator;
        struct {
            char *name;
            struct ASTNode *body;
            struct Environment *env;
            int done;
            struct GeneratorFrame *frames;
            int frame_count;
            int frame_cap;
        } generator;
        struct {
            char *pattern;
        } regex;
        struct {
            double timestamp;
        } datetime;
    };
} Value;

Value *make_int(long long v);
Value *make_float(double v);
Value *make_string(const char *s);
Value *make_bool(int v);
Value *make_none(void);
Value *make_list(void);
Value *make_tuple(void);
Value *make_dict(void);
Value *make_set(void);
Value *make_bytes(const unsigned char *data, int count);
typedef struct GeneratorFrame {
    int kind;
    struct ASTNode *node;
    struct Environment *env;
    int index;
    int phase;
    struct Value *iterator;
} GeneratorFrame;

Value *make_function(const char *name, char **params, struct ASTNode **defaults, int param_count, const char *vararg, struct ASTNode *body, struct Environment *closure, int is_generator);
Value *make_builtin(const char *name, Value *(*fn)(Value **, int));
Value *make_module(const char *name, struct Environment *env);
Value *make_class(const char *name, struct Environment *env, Value *base);
Value *make_instance(Value *class_obj, struct Environment *fields);
Value *make_bound_method(Value *function, Value *receiver);
Value *make_super(Value *class_obj, Value *receiver);
Value *make_class_method(Value *function);
Value *make_static_method(Value *function);
Value *make_property(Value *getter, Value *setter);
Value *make_iterator(int kind, Value *target);
Value *make_generator(const char *name, struct ASTNode *body, struct Environment *env);
Value *make_regex(const char *pattern);
Value *make_datetime(double timestamp);
void list_append(Value *list, Value *item);
void tuple_append(Value *tuple, Value *item);
void dict_set(Value *dict, const char *key, Value *val);
Value *dict_get(Value *dict, const char *key);
void dict_delete(Value *dict, const char *key);
void set_add(Value *set, Value *item);
Value *value_copy(Value *v);
void value_free(Value *v);
char *value_to_string(Value *v);
int value_is_truthy(Value *v);
int value_equals(Value *a, Value *b);
const char *value_type_name(Value *v);
int value_is_class_or_subclass(Value *maybe_class, Value *class_value);
int value_is_instance_of(Value *value, Value *class_value);

#endif
