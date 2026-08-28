#include "value.h"
#include "environment.h"
#include "utf8.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Allocates a Value with the requested runtime type. */
static Value *value_new(ValueType type) {
    Value *v = (Value *)calloc(1, sizeof(Value));
    if (v) { v->type = type; v->refcount = 1; }
    return v;
}

/* Creates a 64-bit integer runtime value. */
Value *make_int(long long v) {
    Value *out = value_new(VAL_INT);
    out->int_val = v;
    return out;
}

/* Creates a double precision floating-point runtime value. */
Value *make_float(double v) {
    Value *out = value_new(VAL_FLOAT);
    out->float_val = v;
    return out;
}

/* Creates a heap-owned UTF-8 string runtime value. */
Value *make_string(const char *s) {
    Value *out = value_new(VAL_STRING);
    out->str_val = azr_strdup(s);
    return out;
}

/* Creates a boolean runtime value using 0 or 1 internally. */
Value *make_bool(int v) {
    Value *out = value_new(VAL_BOOL);
    out->bool_val = v ? 1 : 0;
    return out;
}

/* Creates the singleton-like none value object. */
Value *make_none(void) {
    return value_new(VAL_NONE);
}

/* Creates an empty dynamic list value. */
Value *make_list(void) {
    Value *out = value_new(VAL_LIST);
    out->list.capacity = 8;
    out->list.items = (Value **)calloc((size_t)out->list.capacity, sizeof(Value *));
    return out;
}

Value *make_tuple(void) {
    Value *out = value_new(VAL_TUPLE);
    out->list.capacity = 8;
    out->list.items = (Value **)calloc((size_t)out->list.capacity, sizeof(Value *));
    return out;
}

Value *make_dict(void) {
    Value *out = value_new(VAL_DICT);
    out->dict.capacity = 8;
    out->dict.keys = (char **)calloc((size_t)out->dict.capacity, sizeof(char *));
    out->dict.values = (Value **)calloc((size_t)out->dict.capacity, sizeof(Value *));
    return out;
}

Value *make_set(void) {
    Value *out = value_new(VAL_SET);
    out->set.capacity = 8;
    out->set.items = (Value **)calloc((size_t)out->set.capacity, sizeof(Value *));
    return out;
}

Value *make_bytes(const unsigned char *data, int count) {
    Value *out = value_new(VAL_BYTES);
    out->bytes.count = count < 0 ? 0 : count;
    out->bytes.data = (unsigned char *)calloc((size_t)out->bytes.count + 1, 1);
    if (data && out->bytes.count > 0) memcpy(out->bytes.data, data, (size_t)out->bytes.count);
    return out;
}

/* Creates a user-defined function value with lexical closure metadata. */
Value *make_function(const char *name, char **params, struct ASTNode **defaults, int param_count, const char *vararg, struct ASTNode *body, struct Environment *closure, int is_generator) {
    Value *out = value_new(VAL_FUNCTION);
    out->function.name = azr_strdup(name ? name : "<کار>");
    out->function.params = params;
    out->function.defaults = defaults;
    out->function.param_count = param_count;
    out->function.vararg = vararg ? azr_strdup(vararg) : NULL;
    out->function.body = body;
    out->function.closure = closure;
    out->function.owner_class = NULL;
    out->function.is_generator = is_generator;
    return out;
}

/* Creates a C-backed builtin function value. */
Value *make_builtin(const char *name, Value *(*fn)(Value **, int)) {
    Value *out = value_new(VAL_BUILTIN);
    out->builtin.name = azr_strdup(name);
    out->builtin.fn = fn;
    return out;
}

/* Creates an import module value wrapping an environment namespace. */
Value *make_module(const char *name, struct Environment *env) {
    Value *out = value_new(VAL_MODULE);
    out->module.name = azr_strdup(name);
    out->module.env = env;
    return out;
}

Value *make_class(const char *name, struct Environment *env, Value *base) {
    Value *out = value_new(VAL_CLASS);
    out->class_val.name = azr_strdup(name);
    out->class_val.env = env;
    out->class_val.base = base ? value_copy(base) : NULL;
    return out;
}

Value *make_instance(Value *class_obj, struct Environment *fields) {
    Value *out = value_new(VAL_INSTANCE);
    out->instance.class_name = azr_strdup(class_obj && class_obj->type == VAL_CLASS ? class_obj->class_val.name : "نمونه");
    out->instance.class_obj = class_obj ? value_copy(class_obj) : NULL;
    out->instance.fields = fields;
    return out;
}

Value *make_bound_method(Value *function, Value *receiver) {
    Value *out = value_new(VAL_BOUND_METHOD);
    out->bound_method.function = value_copy(function);
    out->bound_method.receiver = value_copy(receiver);
    return out;
}

Value *make_super(Value *class_obj, Value *receiver) {
    Value *out = value_new(VAL_SUPER);
    out->super_val.class_obj = class_obj ? value_copy(class_obj) : NULL;
    out->super_val.receiver = receiver ? value_copy(receiver) : NULL;
    return out;
}

Value *make_class_method(Value *function) {
    Value *out = value_new(VAL_CLASS_METHOD);
    out->class_method.function = function ? value_copy(function) : NULL;
    return out;
}

Value *make_static_method(Value *function) {
    Value *out = value_new(VAL_STATIC_METHOD);
    out->static_method.function = function ? value_copy(function) : NULL;
    return out;
}

Value *make_property(Value *getter, Value *setter) {
    Value *out = value_new(VAL_PROPERTY);
    out->property.getter = getter ? value_copy(getter) : NULL;
    out->property.setter = setter ? value_copy(setter) : NULL;
    return out;
}

Value *make_iterator(int kind, Value *target) {
    Value *out = value_new(VAL_ITERATOR);
    out->iterator.kind = kind;
    out->iterator.index = 0;
    out->iterator.target = target ? value_copy(target) : NULL;
    return out;
}

Value *make_generator(const char *name, struct ASTNode *body, struct Environment *env) {
    Value *out = value_new(VAL_GENERATOR);
    out->generator.name = azr_strdup(name ? name : "<مولد>");
    out->generator.body = body;
    out->generator.env = env;
    out->generator.done = 0;
    out->generator.frames = NULL;
    out->generator.frame_count = 0;
    out->generator.frame_cap = 0;
    return out;
}

Value *make_regex(const char *pattern) {
    Value *out = value_new(VAL_REGEX);
    out->regex.pattern = azr_strdup(pattern ? pattern : "");
    return out;
}

Value *make_datetime(double timestamp) {
    Value *out = value_new(VAL_DATETIME);
    out->datetime.timestamp = timestamp;
    return out;
}

/* Appends an item to a list, growing the backing array when needed. */
void list_append(Value *list, Value *item) {
    if (!list || list->type != VAL_LIST) return;
    if (list->list.count >= list->list.capacity) {
        list->list.capacity *= 2;
        list->list.items = (Value **)realloc(list->list.items, (size_t)list->list.capacity * sizeof(Value *));
    }
    list->list.items[list->list.count++] = item;
}

void tuple_append(Value *tuple, Value *item) {
    if (!tuple || tuple->type != VAL_TUPLE) return;
    if (tuple->list.count >= tuple->list.capacity) {
        tuple->list.capacity *= 2;
        tuple->list.items = (Value **)realloc(tuple->list.items, (size_t)tuple->list.capacity * sizeof(Value *));
    }
    tuple->list.items[tuple->list.count++] = item;
}

void dict_set(Value *dict, const char *key, Value *val) {
    int i = 0;
    if (!dict || dict->type != VAL_DICT) return;
    for (i = 0; i < dict->dict.count; i++) {
        if (strcmp(dict->dict.keys[i], key) == 0) {
            value_free(dict->dict.values[i]);
            dict->dict.values[i] = val;
            return;
        }
    }
    if (dict->dict.count >= dict->dict.capacity) {
        dict->dict.capacity *= 2;
        dict->dict.keys = (char **)realloc(dict->dict.keys, (size_t)dict->dict.capacity * sizeof(char *));
        dict->dict.values = (Value **)realloc(dict->dict.values, (size_t)dict->dict.capacity * sizeof(Value *));
    }
    dict->dict.keys[dict->dict.count] = azr_strdup(key);
    dict->dict.values[dict->dict.count] = val;
    dict->dict.count++;
}

Value *dict_get(Value *dict, const char *key) {
    int i = 0;
    if (!dict || dict->type != VAL_DICT) return NULL;
    for (i = 0; i < dict->dict.count; i++) if (strcmp(dict->dict.keys[i], key) == 0) return dict->dict.values[i];
    return NULL;
}

void dict_delete(Value *dict, const char *key) {
    int i = 0;
    int j = 0;
    if (!dict || dict->type != VAL_DICT) return;
    for (i = 0; i < dict->dict.count; i++) {
        if (strcmp(dict->dict.keys[i], key) != 0) continue;
        free(dict->dict.keys[i]);
        value_free(dict->dict.values[i]);
        for (j = i + 1; j < dict->dict.count; j++) {
            dict->dict.keys[j - 1] = dict->dict.keys[j];
            dict->dict.values[j - 1] = dict->dict.values[j];
        }
        dict->dict.count--;
        if (dict->dict.count >= 0) {
            dict->dict.keys[dict->dict.count] = NULL;
            dict->dict.values[dict->dict.count] = NULL;
        }
        return;
    }
}

void set_add(Value *set, Value *item) {
    int i = 0;
    if (!set || set->type != VAL_SET) return;
    for (i = 0; i < set->set.count; i++) if (value_equals(set->set.items[i], item)) { value_free(item); return; }
    if (set->set.count >= set->set.capacity) {
        set->set.capacity *= 2;
        set->set.items = (Value **)realloc(set->set.items, (size_t)set->set.capacity * sizeof(Value *));
    }
    set->set.items[set->set.count++] = item;
}

/* Produces a shallow-safe copy for primitive values and deep copies lists. */
Value *value_copy(Value *v) {
    if (!v) return make_none();
    switch (v->type) {
        case VAL_INT: return make_int(v->int_val);
        case VAL_FLOAT: return make_float(v->float_val);
        case VAL_STRING: return make_string(v->str_val);
        case VAL_BOOL: return make_bool(v->bool_val);
        case VAL_NONE: return make_none();
        case VAL_TUPLE:
        case VAL_DICT:
        case VAL_SET:
        case VAL_BYTES:
        case VAL_CLASS:
        case VAL_INSTANCE:
        case VAL_BOUND_METHOD:
        case VAL_SUPER:
        case VAL_CLASS_METHOD:
        case VAL_STATIC_METHOD:
        case VAL_PROPERTY:
        case VAL_ITERATOR:
        case VAL_GENERATOR:
        case VAL_REGEX:
        case VAL_DATETIME:
            v->refcount++;
            return v;
        case VAL_FUNCTION:
        case VAL_BUILTIN:
        case VAL_MODULE:
        case VAL_LIST:
            v->refcount++;
            return v;
    }
    return make_none();
}

/* Frees heap memory owned directly by a runtime value. */
void value_free(Value *v) {
    int k = 0;
    if (!v) return;
    v->refcount--;
    if (v->refcount > 0) return;
    switch (v->type) {
        case VAL_STRING:
            free(v->str_val);
            break;
        case VAL_LIST:
        case VAL_TUPLE:
            for (k = 0; k < v->list.count; k++) value_free(v->list.items[k]);
            free(v->list.items);
            break;
        case VAL_SET:
            for (k = 0; k < v->set.count; k++) value_free(v->set.items[k]);
            free(v->set.items);
            break;
        case VAL_DICT:
            for (k = 0; k < v->dict.count; k++) { free(v->dict.keys[k]); value_free(v->dict.values[k]); }
            free(v->dict.keys); free(v->dict.values);
            break;
        case VAL_BYTES:
            free(v->bytes.data);
            break;
        case VAL_CLASS:
            free(v->class_val.name);
            value_free(v->class_val.base);
            break;
        case VAL_INSTANCE:
            free(v->instance.class_name);
            value_free(v->instance.class_obj);
            env_free(v->instance.fields);
            break;
        case VAL_BOUND_METHOD:
            value_free(v->bound_method.function);
            value_free(v->bound_method.receiver);
            break;
        case VAL_CLASS_METHOD:
            value_free(v->class_method.function);
            break;
        case VAL_STATIC_METHOD:
            value_free(v->static_method.function);
            break;
        case VAL_PROPERTY:
            value_free(v->property.getter);
            value_free(v->property.setter);
            break;
        case VAL_ITERATOR:
            value_free(v->iterator.target);
            break;
        case VAL_GENERATOR:
            free(v->generator.name);
            for (k = 0; k < v->generator.frame_count; k++) value_free(v->generator.frames[k].iterator);
            free(v->generator.frames);
            env_free(v->generator.env);
            break;
        case VAL_REGEX:
            free(v->regex.pattern);
            break;
        case VAL_BUILTIN:
            free(v->builtin.name);
            break;
        case VAL_MODULE:
            free(v->module.name);
            env_free(v->module.env);
            break;
        case VAL_FUNCTION:
            free(v->function.name);
            for (k = 0; k < v->function.param_count; k++) free(v->function.params[k]);
            free(v->function.params);
            free(v->function.vararg);
            value_free(v->function.owner_class);
            break;
        case VAL_SUPER:
            value_free(v->super_val.class_obj);
            value_free(v->super_val.receiver);
            break;
        default:
            break;
    }
    free(v);
}

/* Converts a runtime value to a heap string suitable for بنویس output. */
char *value_to_string(Value *v) {
    char buf[128];
    char *out = NULL;
    int i = 0;
    size_t cap = 0, len = 0;
    if (!v) return azr_strdup("هیچ");
    switch (v->type) {
        case VAL_INT:
            snprintf(buf, sizeof(buf), "%lld", v->int_val);
            return azr_strdup(buf);
        case VAL_FLOAT:
            snprintf(buf, sizeof(buf), "%.15g", v->float_val);
            return azr_strdup(buf);
        case VAL_STRING:
            return azr_strdup(v->str_val);
        case VAL_BOOL:
            return azr_strdup(v->bool_val ? "درسته" : "غلطه");
        case VAL_NONE:
            return azr_strdup("هیچ");
        case VAL_FUNCTION:
            return azr_strdup("<کار>");
        case VAL_BUILTIN:
            snprintf(buf, sizeof(buf), "<ساختنی %s>", v->builtin.name);
            return azr_strdup(buf);
        case VAL_MODULE:
            snprintf(buf, sizeof(buf), "<ماژول %s>", v->module.name);
            return azr_strdup(buf);
        case VAL_CLASS:
            snprintf(buf, sizeof(buf), "<نوع %s>", v->class_val.name);
            return azr_strdup(buf);
        case VAL_INSTANCE:
            snprintf(buf, sizeof(buf), "<نمونه %s>", v->instance.class_name);
            return azr_strdup(buf);
        case VAL_BOUND_METHOD:
            return azr_strdup("<متد>");
        case VAL_SUPER:
            return azr_strdup("<ابر>");
        case VAL_CLASS_METHOD:
            return azr_strdup("<کلاس_متد>");
        case VAL_STATIC_METHOD:
            return azr_strdup("<ایستا>");
        case VAL_PROPERTY:
            return azr_strdup("<ویژگی>");
        case VAL_ITERATOR:
            return azr_strdup("<تکرارگر>");
        case VAL_GENERATOR:
            return azr_strdup("<مولد>");
        case VAL_BYTES:
            snprintf(buf, sizeof(buf), "<بایت %d>", v->bytes.count);
            return azr_strdup(buf);
        case VAL_REGEX:
            snprintf(buf, sizeof(buf), "<regex %s>", v->regex.pattern);
            return azr_strdup(buf);
        case VAL_DATETIME:
            snprintf(buf, sizeof(buf), "%.0f", v->datetime.timestamp);
            return azr_strdup(buf);
        case VAL_LIST:
        case VAL_TUPLE:
        case VAL_SET:
        case VAL_DICT:
            if (v->type == VAL_DICT) {
                cap = 32; out = (char *)malloc(cap); strcpy(out, "{"); len = 1;
                for (i = 0; i < v->dict.count; i++) {
                    char *item = value_to_string(v->dict.values[i]);
                    size_t need = len + strlen(v->dict.keys[i]) + strlen(item) + 8;
                    if (need > cap) { while (need > cap) cap *= 2; out = (char *)realloc(out, cap); }
                    if (i > 0) { strcat(out, "، "); len += 4; }
                    strcat(out, v->dict.keys[i]); strcat(out, ": "); strcat(out, item);
                    len += strlen(v->dict.keys[i]) + strlen(item) + 2;
                    free(item);
                }
                if (len + 2 > cap) out = (char *)realloc(out, len + 2);
                strcat(out, "}"); return out;
            }
            cap = 32;
            out = (char *)malloc(cap);
            strcpy(out, v->type == VAL_SET ? "{" : (v->type == VAL_TUPLE ? "(" : "["));
            len = 1;
            for (i = 0; i < (v->type == VAL_SET ? v->set.count : v->list.count); i++) {
                char *item = value_to_string(v->type == VAL_SET ? v->set.items[i] : v->list.items[i]);
                size_t need = len + strlen(item) + 4;
                if (need > cap) {
                    while (need > cap) cap *= 2;
                    out = (char *)realloc(out, cap);
                }
                if (i > 0) { strcat(out, "، "); len += 4; }
                strcat(out, item);
                len += strlen(item);
                free(item);
            }
            if (len + 2 > cap) out = (char *)realloc(out, len + 2);
            if (v->type == VAL_TUPLE && v->list.count == 1) { strcat(out, "،"); len += 2; }
            strcat(out, v->type == VAL_SET ? "}" : (v->type == VAL_TUPLE ? ")" : "]"));
            return out;
    }
    return azr_strdup("هیچ");
}

/* Applies Python-like truthiness to runtime values. */
int value_is_truthy(Value *v) {
    if (!v || v->type == VAL_NONE) return 0;
    if (v->type == VAL_BOOL) return v->bool_val;
    if (v->type == VAL_INT) return v->int_val != 0;
    if (v->type == VAL_FLOAT) return fabs(v->float_val) > 0.0;
    if (v->type == VAL_STRING) return v->str_val && v->str_val[0] != '\0';
    if (v->type == VAL_LIST) return v->list.count > 0;
    if (v->type == VAL_TUPLE) return v->list.count > 0;
    if (v->type == VAL_SET) return v->set.count > 0;
    if (v->type == VAL_DICT) return v->dict.count > 0;
    if (v->type == VAL_BYTES) return v->bytes.count > 0;
    return 1;
}

/* Compares two runtime values for language-level equality. */
int value_equals(Value *a, Value *b) {
    if (!a || !b) return a == b;
    if ((a->type == VAL_INT || a->type == VAL_FLOAT) && (b->type == VAL_INT || b->type == VAL_FLOAT)) {
        double x = a->type == VAL_INT ? (double)a->int_val : a->float_val;
        double y = b->type == VAL_INT ? (double)b->int_val : b->float_val;
        return fabs(x - y) < 1e-12;
    }
    if (a->type != b->type) return 0;
    switch (a->type) {
        case VAL_STRING: return strcmp(a->str_val, b->str_val) == 0;
        case VAL_BOOL: return a->bool_val == b->bool_val;
        case VAL_NONE: return 1;
        case VAL_LIST:
        case VAL_TUPLE:
            if (a->list.count != b->list.count) return 0;
            for (int i = 0; i < a->list.count; i++) if (!value_equals(a->list.items[i], b->list.items[i])) return 0;
            return 1;
        case VAL_DICT:
        case VAL_SET:
        case VAL_BYTES:
        case VAL_CLASS:
        case VAL_INSTANCE:
        case VAL_BOUND_METHOD:
        case VAL_SUPER:
        case VAL_CLASS_METHOD:
        case VAL_STATIC_METHOD:
        case VAL_PROPERTY:
        case VAL_ITERATOR:
        case VAL_GENERATOR:
        case VAL_REGEX:
        case VAL_DATETIME:
            return a == b;
        case VAL_FUNCTION:
        case VAL_BUILTIN:
        case VAL_MODULE:
            return a == b;
        case VAL_INT: return a->int_val == b->int_val;
        case VAL_FLOAT: return fabs(a->float_val - b->float_val) < 1e-12;
    }
    return 0;
}

/* Returns the natural Farsi name of a runtime type. */
const char *value_type_name(Value *v) {
    if (!v) return "هیچ";
    switch (v->type) {
        case VAL_INT: return "عدد_صحیح";
        case VAL_FLOAT: return "عدد_اعشاری";
        case VAL_STRING: return "متن";
        case VAL_BOOL: return "بولی";
        case VAL_NONE: return "هیچ";
        case VAL_LIST: return "لیست";
        case VAL_TUPLE: return "چندتایی";
        case VAL_FUNCTION: return "کار";
        case VAL_BUILTIN: return "ساختنی";
        case VAL_MODULE: return "ماژول";
        case VAL_DICT: return "دیکشنری";
        case VAL_SET: return "مجموعه";
        case VAL_BYTES: return "بایت";
        case VAL_CLASS: return "نوع";
        case VAL_INSTANCE: return "نمونه";
        case VAL_BOUND_METHOD: return "متد";
        case VAL_SUPER: return "ابر";
        case VAL_CLASS_METHOD: return "کلاس_متد";
        case VAL_STATIC_METHOD: return "ایستا";
        case VAL_PROPERTY: return "ویژگی";
        case VAL_ITERATOR: return "تکرارگر";
        case VAL_GENERATOR: return "مولد";
        case VAL_REGEX: return "regex";
        case VAL_DATETIME: return "زمان";
    }
    return "ناشناخته";
}

int value_is_class_or_subclass(Value *maybe_class, Value *class_value) {
    Value *cur = maybe_class;
    if (!cur || !class_value || cur->type != VAL_CLASS || class_value->type != VAL_CLASS) return 0;
    while (cur) {
        if (cur == class_value) return 1;
        cur = cur->class_val.base;
    }
    return 0;
}

int value_is_instance_of(Value *value, Value *class_value) {
    if (!value || value->type != VAL_INSTANCE) return 0;
    return value_is_class_or_subclass(value->instance.class_obj, class_value);
}
