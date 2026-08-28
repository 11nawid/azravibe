static Value *call_function_value(Interpreter *i, Value *fn, Value *receiver, ASTNode *call_node, Value **args, int argc, Environment *caller_env) {
    Environment *call_env = NULL;
    ExecResult r;
    int k = 0, positional = 0, self_slot = 0;
    Value **bound = NULL;
    (void)caller_env;
    if (!fn || fn->type != VAL_FUNCTION) return runtime_error(i, error_not_callable(value_type_name(fn)));
    if (receiver && fn->function.param_count > 0) {
        const char *first = fn->function.params[0];
        if (strcmp(first, "خود") == 0 || strcmp(first, "self") == 0) self_slot = 1;
        if (receiver->type == VAL_CLASS &&
            (strcmp(first, "کلاس") == 0 || strcmp(first, "cls") == 0)) self_slot = 1;
    }
    positional = self_slot;
    if (!fn->function.vararg && argc > fn->function.param_count - self_slot) {
        return runtime_error(i, error_wrong_args("<کار>", fn->function.param_count - self_slot, argc));
    }
    bound = (Value **)calloc((size_t)fn->function.param_count, sizeof(Value *));
    if (receiver && self_slot) bound[0] = value_copy(receiver);
    for (k = 0; k < argc; k++) {
        int slot = -1, j = 0;
        char *arg_name = call_node && call_node->call.arg_names ? call_node->call.arg_names[k] : NULL;
        if (arg_name) {
            for (j = 0; j < fn->function.param_count; j++) if (strcmp(fn->function.params[j], arg_name) == 0) { slot = j; break; }
            if (slot < 0 || (self_slot && slot == 0)) { free(bound); return runtime_error(i, error_undefined(arg_name)); }
        } else if (positional < fn->function.param_count) {
            slot = positional++;
        }
        if (slot >= 0) {
            if (bound[slot]) { free(bound); return runtime_error(i, error_type("یک پارامتر دوبار مقدار گرفته")); }
            bound[slot] = value_copy(args[k]);
        }
    }
    call_env = env_new(fn->function.closure);
    if (receiver) env_set(call_env, "خود", value_copy(receiver));
    if (receiver && receiver->type == VAL_CLASS) env_set(call_env, "کلاس", value_copy(receiver));
    if (fn->function.owner_class) env_set(call_env, "__class__", value_copy(fn->function.owner_class));
    for (k = 0; k < fn->function.param_count; k++) {
        if (!bound[k] && fn->function.defaults && fn->function.defaults[k]) bound[k] = eval_expression(i, fn->function.defaults[k], call_env);
        if (!bound[k]) {
            free(bound);
            return runtime_error(i, error_wrong_args("<کار>", fn->function.param_count - self_slot, argc));
        }
        env_set(call_env, fn->function.params[k], bound[k]);
    }
    if (fn->function.vararg) {
        Value *rest = make_list();
        for (k = positional; k < argc; k++) {
            char *arg_name = call_node && call_node->call.arg_names ? call_node->call.arg_names[k] : NULL;
            if (!arg_name) list_append(rest, value_copy(args[k]));
        }
        env_set(call_env, fn->function.vararg, rest);
    }
    free(bound);
    if (fn->function.is_generator) {
        Value *generator = make_generator(fn->function.name ? fn->function.name : "<مولد>", fn->function.body, call_env);
        generator_push_frame(generator, GEN_FRAME_BLOCK, fn->function.body, call_env);
        return generator;
    }
    push_frame(i, fn->function.name ? fn->function.name : "<کار>", call_node ? call_node->line : i->current_line);
    if (fn->function.body && fn->function.body->type != NODE_BLOCK) {
        Value *out = eval_expression(i, fn->function.body, call_env);
        pop_frame(i);
        return out;
    }
    r = exec_block(i, fn->function.body, call_env);
    pop_frame(i);
    if (r.signal == SIG_RETURN) return r.value;
    if (r.signal == SIG_ERROR) return runtime_error_object(i, r.value ? r.value : make_exception_instance(i, "خطا", "خطای ناشناخته"));
    return make_none();
}

static Value *lookup_attr_value(Interpreter *i, Value *obj, const char *attr) {
    Value *member = NULL;
    if (!obj) return NULL;
    if (obj->type == VAL_MODULE) {
        member = env_get_local(obj->module.env, attr);
        return member ? value_copy(member) : NULL;
    }
    if (obj->type == VAL_CLASS) {
        member = env_get(obj->class_val.env, attr);
        return bind_member_value(i, member, obj, obj, 1);
    }
    if (obj->type == VAL_DICT) {
        member = dict_get(obj, attr);
        return member ? value_copy(member) : NULL;
    }
    if (obj->type == VAL_SUPER) {
        Value *base = obj->super_val.class_obj ? obj->super_val.class_obj->class_val.base : NULL;
        if (!base) return NULL;
        member = env_get(base->class_val.env, attr);
        return bind_member_value(i, member, obj->super_val.receiver, base, 0);
    }
    if (obj->type == VAL_INSTANCE) {
        member = obj->instance.class_obj ? env_get(obj->instance.class_obj->class_val.env, attr) : env_get(obj->instance.fields->parent, attr);
        if (member && member->type == VAL_PROPERTY) return bind_member_value(i, member, obj, obj->instance.class_obj, 0);
        member = env_get_local(obj->instance.fields, attr);
        if (member) return value_copy(member);
        member = obj->instance.class_obj ? env_get(obj->instance.class_obj->class_val.env, attr) : env_get(obj->instance.fields->parent, attr);
        return bind_member_value(i, member, obj, obj->instance.class_obj, 0);
    }
    return NULL;
}

static char *format_exception(Value *exc) {
    char *out = NULL;
    size_t len = 0, cap = 0;
    char *message = NULL;
    Value *msg_value = NULL;
    Value *traceback = NULL;
    const char *class_name = "خطا";
    int idx = 0;
    if (!exc) return azr_strdup("خطای ناشناخته");
    if (exc->type != VAL_INSTANCE) return value_to_string(exc);
    class_name = exc->instance.class_name;
    msg_value = lookup_attr_value(g_active_interpreter, exc, "پیام");
    if (!msg_value) msg_value = lookup_attr_value(g_active_interpreter, exc, "message");
    message = msg_value ? value_to_string(msg_value) : azr_strdup("");
    append_text(&out, &len, &cap, class_name);
    if (message[0]) {
        append_text(&out, &len, &cap, ": ");
        append_text(&out, &len, &cap, message);
    }
    traceback = lookup_attr_value(g_active_interpreter, exc, "traceback");
    if (traceback && traceback->type == VAL_LIST) {
        for (idx = 0; idx < traceback->list.count; idx++) {
            Value *frame = traceback->list.items[idx];
            Value *file = frame && frame->type == VAL_DICT ? dict_get(frame, "file") : NULL;
            Value *line = frame && frame->type == VAL_DICT ? dict_get(frame, "line") : NULL;
            Value *function_name = frame && frame->type == VAL_DICT ? dict_get(frame, "function") : NULL;
            char line_buf[64];
            append_text(&out, &len, &cap, "\n  در ");
            append_text(&out, &len, &cap, file && file->type == VAL_STRING ? file->str_val : "<ورودی>");
            snprintf(line_buf, sizeof(line_buf), " خط %lld", line && line->type == VAL_INT ? line->int_val : 0LL);
            append_text(&out, &len, &cap, line_buf);
            append_text(&out, &len, &cap, " داخل ");
            append_text(&out, &len, &cap, function_name && function_name->type == VAL_STRING ? function_name->str_val : "سطح_بالا");
        }
    }
    value_free(msg_value);
    value_free(traceback);
    free(message);
    return out ? out : azr_strdup(class_name);
}

static void bind_destructure(Interpreter *i, Environment *env, ASTNode *target, Value *value) {
    int k = 0;
    if (!target || !value || i->had_error) return;
    if (target->type == NODE_IDENTIFIER) {
        env_set(env, target->identifier.name, value_copy(value));
        return;
    }
    if ((target->type == NODE_TUPLE_LITERAL || target->type == NODE_LIST_LITERAL) && (value->type == VAL_LIST || value->type == VAL_TUPLE)) {
        ASTNode **items = target->type == NODE_TUPLE_LITERAL ? target->tuple_lit.items : target->list_lit.items;
        int count = target->type == NODE_TUPLE_LITERAL ? target->tuple_lit.count : target->list_lit.count;
        if (count != value->list.count) { runtime_error(i, error_type("تعداد مقدارها برای بازکردن ساختار یکی نیست")); return; }
        for (k = 0; k < count; k++) bind_destructure(i, env, items[k], value->list.items[k]);
        return;
    }
    runtime_error(i, error_type("هدف انتساب قابل پشتیبانی نیست"));
}

static ExecResult exec_attr_assignment(Interpreter *i, ASTNode *node, Environment *env) {
    Value *obj = eval_expression(i, node->attr_assign.object, env);
    Value *val = eval_expression(i, node->attr_assign.value, env);
    Value *member = NULL;
    Value *setter = NULL;
    Value *args[1];
    if (!obj || !val) { value_free(obj); value_free(val); return current_error_result(i); }
    if (obj->type != VAL_INSTANCE && obj->type != VAL_DICT && obj->type != VAL_CLASS && obj->type != VAL_MODULE) {
        value_free(obj); value_free(val);
        return raise_result(i, error_type("فقط نمونه، نوع، ماژول یا دیکشنری ویژگی می‌گیرد"));
    }
    if (obj->type == VAL_INSTANCE) {
        member = obj->instance.class_obj ? env_get(obj->instance.class_obj->class_val.env, node->attr_assign.attr) : NULL;
        if (member && member->type == VAL_PROPERTY) {
            if (!member->property.setter) {
                value_free(obj);
                value_free(val);
                return raise_result(i, error_type("این ویژگی setter ندارد"));
            }
            args[0] = val;
            setter = call_function_value(i, member->property.setter, obj, NULL, args, 1, NULL);
            value_free(val);
            value_free(obj);
            if (!setter) return current_error_result(i);
            value_free(setter);
            return ok(make_none());
        }
        env_set(obj->instance.fields, node->attr_assign.attr, val);
    } else if (obj->type == VAL_CLASS) {
        env_set(obj->class_val.env, node->attr_assign.attr, val);
    } else if (obj->type == VAL_MODULE) {
        env_set(obj->module.env, node->attr_assign.attr, val);
    } else {
        dict_set(obj, node->attr_assign.attr, val);
    }
    value_free(obj);
    return ok(make_none());
}

/* Applies an augmented assignment operator to two existing runtime values. */
static Value *apply_augmented_values(Interpreter *i, Value *left, const char *op, Value *right) {
    char basic[2] = { op ? op[0] : 0, 0 };
    if (strcmp(basic, "+") == 0 && is_number(left) && is_number(right)) return (left->type == VAL_FLOAT || right->type == VAL_FLOAT) ? make_float(num(left) + num(right)) : make_int(left->int_val + right->int_val);
    if (strcmp(basic, "-") == 0 && is_number(left) && is_number(right)) return (left->type == VAL_FLOAT || right->type == VAL_FLOAT) ? make_float(num(left) - num(right)) : make_int(left->int_val - right->int_val);
    if (strcmp(basic, "*") == 0 && is_number(left) && is_number(right)) return (left->type == VAL_FLOAT || right->type == VAL_FLOAT) ? make_float(num(left) * num(right)) : make_int(left->int_val * right->int_val);
    if (strcmp(basic, "/") == 0 && is_number(left) && is_number(right)) {
        if (num(right) == 0) return runtime_error(i, error_div_zero());
        return make_float(num(left) / num(right));
    }
    if (strcmp(basic, "+") == 0 && left->type == VAL_STRING && right->type == VAL_STRING) {
        char *s = (char *)malloc(strlen(left->str_val) + strlen(right->str_val) + 1);
        strcpy(s, left->str_val);
        strcat(s, right->str_val);
        left = make_string(s);
        free(s);
        return left;
    }
    if (strcmp(basic, "+") == 0 && left->type == VAL_LIST && right->type == VAL_LIST) {
        int k = 0;
        Value *out = make_list();
        for (k = 0; k < left->list.count; k++) list_append(out, value_copy(left->list.items[k]));
        for (k = 0; k < right->list.count; k++) list_append(out, value_copy(right->list.items[k]));
        return out;
    }
    return runtime_error(i, error_type("این مقدارها برای انتساب ترکیبی مناسب نیستن"));
}

/* Assigns or augments a list item selected by an index expression. */
static ExecResult exec_index_assignment(Interpreter *i, ASTNode *object_node, ASTNode *index_node, const char *op, ASTNode *value_node, Environment *env) {
    Value *object = eval_expression(i, object_node, env);
    Value *index_value = eval_expression(i, index_node, env);
    Value *new_value = NULL;
    Value *method = NULL;
    Value *args[2];
    int idx = 0;
    if (!object || !index_value) {
        value_free(object);
        value_free(index_value);
        return current_error_result(i);
    }
    if (object->type == VAL_DICT) {
        char *key = value_key(index_value);
        new_value = eval_expression(i, value_node, env);
        value_free(index_value);
        if (!new_value) { value_free(object); free(key); return current_error_result(i); }
        dict_set(object, key, new_value);
        free(key); value_free(object);
        return ok(make_none());
    }
    if (object->type == VAL_INSTANCE) {
        method = lookup_attr_value(i, object, "__setitem__");
        if (!method) method = lookup_attr_value(i, object, "شاخص_بگذار");
        if (method) {
            new_value = eval_expression(i, value_node, env);
            if (!new_value) {
                value_free(method);
                value_free(object);
                value_free(index_value);
                return current_error_result(i);
            }
            if (op) {
                Value *current = lookup_attr_value(i, object, "__getitem__");
                Value *cur_args[1];
                Value *fetched = NULL;
                Value *augmented = NULL;
                if (!current) current = lookup_attr_value(i, object, "شاخص_بگیر");
                if (!current) {
                    value_free(method);
                    value_free(object);
                    value_free(index_value);
                    value_free(new_value);
                    return raise_result(i, error_type("برای انتساب ترکیبی، __getitem__ لازم است"));
                }
                cur_args[0] = index_value;
                fetched = call_callable_value(i, current, cur_args, 1);
                value_free(current);
                if (!fetched) {
                    value_free(method);
                    value_free(object);
                    value_free(index_value);
                    value_free(new_value);
                    return current_error_result(i);
                }
                augmented = apply_augmented_values(i, fetched, op, new_value);
                value_free(fetched);
                value_free(new_value);
                if (!augmented) {
                    value_free(method);
                    value_free(object);
                    value_free(index_value);
                    return current_error_result(i);
                }
                new_value = augmented;
            }
            args[0] = index_value;
            args[1] = new_value;
            {
                Value *set_result = call_callable_value(i, method, args, 2);
                value_free(method);
                value_free(object);
                value_free(index_value);
                value_free(new_value);
                if (!set_result) return current_error_result(i);
                value_free(set_result);
                return ok(make_none());
            }
        }
    }
    if (object->type != VAL_LIST) {
        value_free(object);
        value_free(index_value);
        return raise_result(i, error_type("فقط آیتم‌های لیست و دیکشنری قابل تغییر هستن؛ چندتایی و بایت تغییرناپذیرن"));
    }
    idx = (int)integer(index_value);
    value_free(index_value);
    if (idx < 0) idx += object->list.count;
    if (idx < 0 || idx >= object->list.count) {
        int len = object->list.count;
        value_free(object);
        return raise_result(i, error_index(idx, len));
    }
    new_value = eval_expression(i, value_node, env);
    if (!new_value) {
        value_free(object);
        return current_error_result(i);
    }
    if (op) {
        Value *augmented = apply_augmented_values(i, object->list.items[idx], op, new_value);
        value_free(new_value);
        if (!augmented) {
            value_free(object);
            return current_error_result(i);
        }
        new_value = augmented;
    }
    value_free(object->list.items[idx]);
    object->list.items[idx] = new_value;
    value_free(object);
    return ok(make_none());
}

/* Creates a fresh interpreter with a global environment and builtins. */
