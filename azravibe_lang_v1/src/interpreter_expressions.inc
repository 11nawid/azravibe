static int eval_comprehension_loop(Interpreter *i, ASTNode *node, Environment *env, int depth, Value *out) {
    Value *iterable = NULL;
    Value *iterator = NULL;
    if (depth >= node->comprehension.loop_count) {
        if (node->type == NODE_DICT_COMPREHENSION) {
            Value *key = eval_expression(i, node->comprehension.item_expr, env);
            Value *val = eval_expression(i, node->comprehension.value_expr, env);
            char *ks = NULL;
            if (!key || !val) {
                value_free(key);
                value_free(val);
                return 0;
            }
            ks = value_key(key);
            dict_set(out, ks, val);
            free(ks);
            value_free(key);
            return 1;
        }
        {
            Value *item = eval_expression(i, node->comprehension.item_expr, env);
            if (!item) return 0;
            if (node->type == NODE_LIST_COMPREHENSION) list_append(out, item);
            else set_add(out, item);
            return 1;
        }
    }
    iterable = eval_expression(i, node->comprehension.iterables[depth], env);
    if (!iterable) return 0;
    iterator = iterator_from_value(i, iterable);
    value_free(iterable);
    if (!iterator) return 0;
    while (1) {
        Value *item = NULL;
        Value *cond = NULL;
        int stopped = 0;
        int passed = 1;
        item = iterator_next_value_internal(i, iterator, &stopped);
        if (!item && stopped) break;
        if (!item) {
            value_free(iterator);
            return 0;
        }
        env_set(env, node->comprehension.var_names[depth], item);
        cond = node->comprehension.conditions[depth] ? eval_expression(i, node->comprehension.conditions[depth], env) : NULL;
        if (node->comprehension.conditions[depth]) {
            if (!cond) {
                value_free(iterator);
                return 0;
            }
            passed = value_is_truthy(cond);
            value_free(cond);
        }
        if (passed && !eval_comprehension_loop(i, node, env, depth + 1, out)) {
            value_free(iterator);
            return 0;
        }
    }
    value_free(iterator);
    return 1;
}

/* Evaluates any expression node into a runtime value. */
Value *eval_expression(Interpreter *i, ASTNode *node, Environment *env) {
    i->current_line = node ? node->line : i->current_line;
    if (!node) return make_none();
    switch (node->type) {
        case NODE_INT_LITERAL: return make_int(node->int_lit.value);
        case NODE_FLOAT_LITERAL: return make_float(node->float_lit.value);
        case NODE_STRING_LITERAL: return (strchr(node->str_lit.value, '{') || strstr(node->str_lit.value, "}}")) ? interpolate_string(i, node->str_lit.value, env) : make_string(node->str_lit.value);
        case NODE_BOOL_LITERAL: return make_bool(node->bool_lit.value);
        case NODE_NONE_LITERAL: return make_none();
        case NODE_IDENTIFIER: {
            Value *v = env_get(env, node->identifier.name);
            if (!v) return runtime_error(i, error_undefined(node->identifier.name));
            return value_copy(v);
        }
        case NODE_LIST_LITERAL: {
            int k = 0;
            Value *list = make_list();
            for (k = 0; k < node->list_lit.count; k++) {
                Value *item = eval_expression(i, node->list_lit.items[k], env);
                if (!item) { value_free(list); return NULL; }
                list_append(list, item);
            }
            return list;
        }
        case NODE_TUPLE_LITERAL: {
            int k = 0;
            Value *list = make_tuple();
            for (k = 0; k < node->tuple_lit.count; k++) {
                Value *item = eval_expression(i, node->tuple_lit.items[k], env);
                if (!item) { value_free(list); return NULL; }
                tuple_append(list, item);
            }
            return list;
        }
        case NODE_DICT_LITERAL: {
            int k = 0;
            Value *dict = make_dict();
            for (k = 0; k < node->dict_lit.count; k++) {
                Value *key = eval_expression(i, node->dict_lit.keys[k], env);
                Value *val = eval_expression(i, node->dict_lit.values[k], env);
                char *ks = NULL;
                if (!key || !val) { value_free(key); value_free(val); value_free(dict); return NULL; }
                ks = value_key(key); dict_set(dict, ks, val); free(ks); value_free(key);
            }
            return dict;
        }
        case NODE_SET_LITERAL: {
            int k = 0;
            Value *set = make_set();
            for (k = 0; k < node->dict_lit.count; k++) {
                Value *item = eval_expression(i, node->dict_lit.keys[k], env);
                if (!item) { value_free(set); return NULL; }
                set_add(set, item);
            }
            return set;
        }
        case NODE_LIST_COMPREHENSION:
        case NODE_SET_COMPREHENSION:
        case NODE_DICT_COMPREHENSION: {
            Value *out = node->type == NODE_LIST_COMPREHENSION ? make_list() : (node->type == NODE_SET_COMPREHENSION ? make_set() : make_dict());
            if (!eval_comprehension_loop(i, node, env, 0, out)) {
                value_free(out);
                return NULL;
            }
            return out;
        }
        case NODE_BINARY_OP: return eval_binary(i, node, env);
        case NODE_UNARY_OP: return eval_unary(i, node, env);
        case NODE_CALL: return eval_call(i, node, env);
        case NODE_INDEX: return eval_index(i, node, env);
        case NODE_SLICE: {
            Value *obj = eval_expression(i, node->slice.object, env);
            Value *sv = node->slice.start ? eval_expression(i, node->slice.start, env) : NULL;
            Value *ev = node->slice.end ? eval_expression(i, node->slice.end, env) : NULL;
            int start = sv ? (int)integer(sv) : 0, end = ev ? (int)integer(ev) : 0;
            value_free(sv); value_free(ev);
            if (!obj) return NULL;
            if (obj->type == VAL_LIST || obj->type == VAL_TUPLE) {
                int len = obj->list.count, k = 0;
                Value *out = obj->type == VAL_TUPLE ? make_tuple() : make_list();
                if (!node->slice.end) end = len;
                if (start < 0) start += len;
                if (end < 0) end += len;
                if (start < 0) start = 0;
                if (end > len) end = len;
                for (k = start; k < end; k++) {
                    if (out->type == VAL_TUPLE) tuple_append(out, value_copy(obj->list.items[k]));
                    else list_append(out, value_copy(obj->list.items[k]));
                }
                value_free(obj); return out;
            }
            if (obj->type == VAL_BYTES) {
                int len = obj->bytes.count;
                Value *out = NULL;
                if (!node->slice.end) end = len;
                if (start < 0) start += len;
                if (end < 0) end += len;
                if (start < 0) start = 0;
                if (end > len) end = len;
                if (end < start) end = start;
                out = make_bytes(obj->bytes.data + start, end - start);
                value_free(obj); return out;
            }
            if (obj->type == VAL_STRING) {
                Value *args[3];
                args[0] = obj; args[1] = make_int(start); args[2] = make_int(node->slice.end ? end : utf8_strlen(obj->str_val));
                {
                    const char *s = obj->str_val;
                    int cp = 0, pos = 0, bs = 0, be = (int)strlen(s), br = 0;
                    while (s[pos]) { if (cp == start) bs = pos; if (cp == args[2]->int_val) { be = pos; break; } utf8_codepoint(s + pos, &br); pos += br; cp++; }
                    Value *out = make_string(azr_strndup(s + bs, be - bs));
                    value_free(args[1]); value_free(args[2]); value_free(obj); return out;
                }
            }
            value_free(obj); return runtime_error(i, error_type("برش فقط برای لیست، چندتایی، بایت و متن پشتیبانی میشه"));
        }
        case NODE_ATTRIBUTE: {
            Value *obj = eval_expression(i, node->attribute.object, env);
            Value *v = NULL;
            if (!obj) return NULL;
            if (obj->type == VAL_INSTANCE || obj->type == VAL_SUPER || obj->type == VAL_MODULE || obj->type == VAL_CLASS || obj->type == VAL_DICT) {
                v = lookup_attr_value(i, obj, node->attribute.attr);
            } else {
                value_free(obj);
                return runtime_error(i, error_type("این مقدار ویژگی نداره"));
            }
            if (!v) { value_free(obj); return runtime_error(i, error_undefined(node->attribute.attr)); }
            value_free(obj);
            return v;
        }
        case NODE_LAMBDA: {
            int k = 0;
            char **params = (char **)calloc((size_t)node->lambda.param_count, sizeof(char *));
            for (k = 0; k < node->lambda.param_count; k++) params[k] = azr_strdup(node->lambda.params[k]);
            return make_function("<lambda>", params, node->lambda.defaults, node->lambda.param_count, node->lambda.vararg, node->lambda.body, env, ast_contains_yield(node->lambda.body));
        }
        case NODE_TERNARY: {
            Value *cond = eval_expression(i, node->ternary.condition, env);
            int truth = 0;
            if (!cond) return NULL;
            truth = value_is_truthy(cond); value_free(cond);
            return eval_expression(i, truth ? node->ternary.then_expr : node->ternary.else_expr, env);
        }
        default:
            return make_none();
    }
}

/* Evaluates a binary operation including arithmetic, comparisons, and logic. */
Value *eval_binary(Interpreter *i, ASTNode *node, Environment *env) {
    Value *a = NULL, *b = NULL, *out = NULL;
    const char *op = node->binary.op;
    int handled = 0;
    int eq_result = 0;
    if (strcmp(op, "و") == 0) {
        a = eval_expression(i, node->binary.left, env); if (!a) return NULL;
        if (!value_is_truthy(a)) { value_free(a); return make_bool(0); }
        b = eval_expression(i, node->binary.right, env); value_free(a); if (!b) return NULL;
        out = make_bool(value_is_truthy(b)); value_free(b); return out;
    }
    if (strcmp(op, "یا") == 0) {
        a = eval_expression(i, node->binary.left, env); if (!a) return NULL;
        if (value_is_truthy(a)) { value_free(a); return make_bool(1); }
        b = eval_expression(i, node->binary.right, env); value_free(a); if (!b) return NULL;
        out = make_bool(value_is_truthy(b)); value_free(b); return out;
    }
    a = eval_expression(i, node->binary.left, env); if (!a) return NULL;
    b = eval_expression(i, node->binary.right, env); if (!b) { value_free(a); return NULL; }
    if ((strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) && interpreter_try_protocol_equals(a, b, &eq_result, &handled) && handled) {
        if (i->had_error) out = NULL;
        else out = make_bool(strcmp(op, "==") == 0 ? eq_result : !eq_result);
    }
    else if (strcmp(op, "==") == 0) out = make_bool(value_equals(a, b));
    else if (strcmp(op, "!=") == 0) out = make_bool(!value_equals(a, b));
    else if (is_number(a) && is_number(b) && strcmp(op, ">") == 0) out = make_bool(num(a) > num(b));
    else if (is_number(a) && is_number(b) && strcmp(op, "<") == 0) out = make_bool(num(a) < num(b));
    else if (is_number(a) && is_number(b) && strcmp(op, ">=") == 0) out = make_bool(num(a) >= num(b));
    else if (is_number(a) && is_number(b) && strcmp(op, "<=") == 0) out = make_bool(num(a) <= num(b));
    else if (strcmp(op, "+") == 0 && is_number(a) && is_number(b)) out = (a->type == VAL_FLOAT || b->type == VAL_FLOAT) ? make_float(num(a) + num(b)) : make_int(a->int_val + b->int_val);
    else if (strcmp(op, "-") == 0 && is_number(a) && is_number(b)) out = (a->type == VAL_FLOAT || b->type == VAL_FLOAT) ? make_float(num(a) - num(b)) : make_int(a->int_val - b->int_val);
    else if (strcmp(op, "*") == 0 && is_number(a) && is_number(b)) out = (a->type == VAL_FLOAT || b->type == VAL_FLOAT) ? make_float(num(a) * num(b)) : make_int(a->int_val * b->int_val);
    else if (strcmp(op, "/") == 0 && is_number(a) && is_number(b)) { if (num(b) == 0) out = runtime_error(i, error_div_zero()); else out = make_float(num(a) / num(b)); }
    else if (strcmp(op, "//") == 0 && is_number(a) && is_number(b)) { if (num(b) == 0) out = runtime_error(i, error_div_zero()); else out = make_int((long long)floor(num(a) / num(b))); }
    else if (strcmp(op, "%") == 0 && is_number(a) && is_number(b)) { if (num(b) == 0) out = runtime_error(i, error_div_zero()); else out = make_int(integer(a) % integer(b)); }
    else if (strcmp(op, "**") == 0 && is_number(a) && is_number(b)) out = make_float(pow(num(a), num(b)));
    else if (strcmp(op, "+") == 0 && a->type == VAL_STRING && b->type == VAL_STRING) { char *s = (char *)malloc(strlen(a->str_val) + strlen(b->str_val) + 1); strcpy(s, a->str_val); strcat(s, b->str_val); out = make_string(s); free(s); }
    else if (strcmp(op, "*") == 0 && a->type == VAL_INT && b->type == VAL_STRING) { int k; char *s = (char *)calloc(strlen(b->str_val) * (size_t)a->int_val + 1, 1); for (k = 0; k < a->int_val; k++) strcat(s, b->str_val); out = make_string(s); free(s); }
    else if (strcmp(op, "*") == 0 && a->type == VAL_STRING && b->type == VAL_INT) { int k; char *s = (char *)calloc(strlen(a->str_val) * (size_t)b->int_val + 1, 1); for (k = 0; k < b->int_val; k++) strcat(s, a->str_val); out = make_string(s); free(s); }
    else if (strcmp(op, "+") == 0 && a->type == VAL_LIST && b->type == VAL_LIST) { int k; out = make_list(); for (k = 0; k < a->list.count; k++) list_append(out, value_copy(a->list.items[k])); for (k = 0; k < b->list.count; k++) list_append(out, value_copy(b->list.items[k])); }
    else out = runtime_error(i, error_type("این دو مقدار با این عملگر جور نیستن"));
    value_free(a); value_free(b);
    return out;
}

/* Evaluates unary minus and logical not. */
Value *eval_unary(Interpreter *i, ASTNode *node, Environment *env) {
    Value *v = eval_expression(i, node->unary.operand, env);
    Value *out = NULL;
    if (!v) return NULL;
    if (strcmp(node->unary.op, "نه") == 0) out = make_bool(!value_is_truthy(v));
    else if (strcmp(node->unary.op, "-") == 0 && v->type == VAL_INT) out = make_int(-v->int_val);
    else if (strcmp(node->unary.op, "-") == 0 && v->type == VAL_FLOAT) out = make_float(-v->float_val);
    else out = runtime_error(i, error_type("این عملگر یک‌تایی برای این مقدار درست نیست"));
    value_free(v);
    return out;
}

/* Evaluates function and builtin calls. */
Value *eval_call(Interpreter *i, ASTNode *node, Environment *env) {
    int k = 0;
    Value *callee = NULL;
    Value **args = (Value **)calloc((size_t)node->call.arg_count, sizeof(Value *));
    Value *out = NULL;
    if (node->call.callee &&
        node->call.callee->type == NODE_IDENTIFIER &&
        (strcmp(node->call.callee->identifier.name, "ابر") == 0 || strcmp(node->call.callee->identifier.name, "super") == 0)) {
        Value *owner_class = env_get(env, "__class__");
        Value *receiver = env_get(env, "خود");
        if (!owner_class || !receiver || owner_class->type != VAL_CLASS) {
            free(args);
            return runtime_error(i, error_type("ابر() فقط داخل متدهای نمونه قابل استفاده است"));
        }
        if (!owner_class->class_val.base) {
            free(args);
            return runtime_error(i, error_type("این نوع پدر ندارد"));
        }
        if (node->call.arg_count != 0) {
            free(args);
            return runtime_error(i, error_wrong_args("ابر", 0, node->call.arg_count));
        }
        return make_super(owner_class, receiver);
    }
    callee = eval_expression(i, node->call.callee, env);
    if (!callee) { free(args); return NULL; }
    for (k = 0; k < node->call.arg_count; k++) {
        args[k] = eval_expression(i, node->call.args[k], env);
        if (!args[k]) { value_free(callee); free(args); return NULL; }
    }
    if (callee->type == VAL_BUILTIN) out = callee->builtin.fn(args, node->call.arg_count);
    else if (callee->type == VAL_FUNCTION) {
        out = call_function_value(i, callee, NULL, node, args, node->call.arg_count, env);
    } else if (callee->type == VAL_BOUND_METHOD) {
        out = call_function_value(i, callee->bound_method.function, callee->bound_method.receiver, node, args, node->call.arg_count, env);
    } else if (callee->type == VAL_CLASS) {
        Environment *fields = env_new(callee->class_val.env);
        Value *ctor = env_get(callee->class_val.env, "سازنده");
        if (!ctor) ctor = env_get(callee->class_val.env, "__init__");
        out = make_instance(callee, fields);
        if (ctor && ctor->type == VAL_FUNCTION) {
            Value *ctor_result = call_function_value(i, ctor, out, node, args, node->call.arg_count, env);
            if (!ctor_result) { value_free(out); out = NULL; }
            else value_free(ctor_result);
        }
    } else if (callee->type == VAL_INSTANCE) {
        Value *call_method = lookup_attr_value(i, callee, "__call__");
        if (!call_method) call_method = lookup_attr_value(i, callee, "فراخوان");
        if (!call_method) out = runtime_error(i, error_not_callable(value_type_name(callee)));
        else {
            out = call_callable_value(i, call_method, args, node->call.arg_count);
            value_free(call_method);
        }
    } else out = runtime_error(i, error_not_callable(value_type_name(callee)));
    for (k = 0; k < node->call.arg_count; k++) value_free(args[k]);
    free(args);
    value_free(callee);
    return out;
}

/* Evaluates list or string indexing. */
Value *eval_index(Interpreter *i, ASTNode *node, Environment *env) {
    Value *obj = eval_expression(i, node->index.object, env);
    Value *idxv = eval_expression(i, node->index.index, env);
    Value *method = NULL;
    Value *args[1];
    int idx = 0;
    if (!obj || !idxv) { value_free(obj); value_free(idxv); return NULL; }
    if (obj->type == VAL_DICT) {
        char *key = value_key(idxv);
        Value *v = dict_get(obj, key);
        free(key); value_free(idxv);
        if (!v) { value_free(obj); return runtime_error(i, error_undefined("کلید")); }
        v = value_copy(v); value_free(obj); return v;
    }
    if (obj->type == VAL_INSTANCE) {
        method = lookup_attr_value(i, obj, "__getitem__");
        if (!method) method = lookup_attr_value(i, obj, "شاخص_بگیر");
        if (method) {
            Value *out = NULL;
            args[0] = idxv;
            out = call_callable_value(i, method, args, 1);
            value_free(method);
            value_free(idxv);
            value_free(obj);
            return out;
        }
    }
    idx = (int)integer(idxv);
    value_free(idxv);
    if (obj->type == VAL_LIST || obj->type == VAL_TUPLE) {
        Value *out = NULL;
        if (idx < 0) idx += obj->list.count;
        if (idx < 0 || idx >= obj->list.count) { int len = obj->list.count; value_free(obj); return runtime_error(i, error_index(idx, len)); }
        out = value_copy(obj->list.items[idx]);
        value_free(obj);
        return out;
    }
    if (obj->type == VAL_BYTES) {
        int len = obj->bytes.count;
        if (idx < 0) idx += len;
        if (idx < 0 || idx >= len) { value_free(obj); return runtime_error(i, error_index(idx, len)); }
        {
            Value *out = make_int(obj->bytes.data[idx]);
            value_free(obj);
            return out;
        }
    }
    if (obj->type == VAL_STRING) {
        int pos = 0, cp = 0, br = 0;
        int len = utf8_strlen(obj->str_val);
        if (idx < 0) idx += len;
        if (idx < 0 || idx >= len) { value_free(obj); return runtime_error(i, error_index(idx, len)); }
        while (obj->str_val[pos]) {
            utf8_codepoint(obj->str_val + pos, &br);
            if (cp == idx) {
                char *s = azr_strndup(obj->str_val + pos, br);
                Value *out = make_string(s);
                free(s); value_free(obj); return out;
            }
            pos += br; cp++;
        }
    }
    value_free(obj);
    return runtime_error(i, error_type("فقط لیست، چندتایی، بایت و متن اندیس می‌گیرن"));
}

/* Frees interpreter global resources. */
void interpreter_free(Interpreter *i) {
    int idx = 0;
    if (!i) return;
    if (g_active_interpreter == i) g_active_interpreter = NULL;
    env_free(i->global_env);
    value_free(i->module_cache);
    free(i->error_message);
    value_free(i->current_exception);
    free(i->stdlib_path);
    free(i->current_file);
    free(i->entry_dir);
    for (idx = 0; idx < i->import_root_count; idx++) free(i->import_roots[idx]);
    free(i->import_roots);
    free(i->module_name);
    free(i->module_package);
    for (idx = 0; idx < i->frame_count; idx++) {
        free(i->frames[idx].name);
        free(i->frames[idx].file);
    }
    free(i->frames);
    free(i);
}
