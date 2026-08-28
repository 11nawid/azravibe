static int is_number(Value *v) { return v && (v->type == VAL_INT || v->type == VAL_FLOAT); }

/* Converts a number value to double. */
static double num(Value *v) { return v->type == VAL_INT ? (double)v->int_val : v->float_val; }

/* Converts a numeric value to long long. */
static long long integer(Value *v) { return v->type == VAL_INT ? v->int_val : (long long)v->float_val; }

static char *value_key(Value *v) {
    return value_to_string(v);
}

static void append_text(char **buf, size_t *len, size_t *cap, const char *text) {
    size_t need = *len + strlen(text) + 1;
    if (need > *cap) {
        while (need > *cap) *cap = *cap ? *cap * 2 : 64;
        *buf = (char *)realloc(*buf, *cap);
    }
    strcpy(*buf + *len, text);
    *len += strlen(text);
}

static Value *eval_inline_expression(Interpreter *i, const char *source, Environment *env) {
    Lexer *lx = lexer_new(source);
    int count = 0, k = 0;
    Token **tokens = lexer_tokenize_all(lx, &count);
    Parser *p = parser_new(tokens, count);
    ASTNode *expr = parse_expression(p);
    Value *out = NULL;
    if (p->error) out = runtime_error(i, azr_strdup(p->error));
    else out = eval_expression(i, expr, env);
    ast_free(expr);
    parser_free(p);
    for (k = 0; k < count; k++) token_free(tokens[k]);
    free(tokens);
    lexer_free(lx);
    return out;
}

static Value *interpolate_string(Interpreter *i, const char *src, Environment *env) {
    char *out = NULL;
    size_t len = 0, cap = 0;
    int pos = 0, start = 0;
    while (src[pos]) {
        if (src[pos] == '{') {
            if (src[pos + 1] == '{') {
                if (pos > start) {
                    char *part = azr_strndup(src + start, pos - start);
                    append_text(&out, &len, &cap, part);
                    free(part);
                }
                append_text(&out, &len, &cap, "{");
                pos += 2;
                start = pos;
                continue;
            }
            if (pos > start) {
                char *part = azr_strndup(src + start, pos - start);
                append_text(&out, &len, &cap, part);
                free(part);
            }
            pos++;
            start = pos;
            while (src[pos] && src[pos] != '}') pos++;
            if (!src[pos]) { free(out); return runtime_error(i, error_type("در متن قالبی '}' کم است")); }
            {
                char *expr_src = azr_strndup(src + start, pos - start);
                Value *v = eval_inline_expression(i, expr_src, env);
                char *text = NULL;
                free(expr_src);
                if (!v) { free(out); return NULL; }
                text = value_to_string(v);
                append_text(&out, &len, &cap, text);
                free(text);
                value_free(v);
            }
            pos++;
            start = pos;
        } else if (src[pos] == '}' && src[pos + 1] == '}') {
            if (pos > start) {
                char *part = azr_strndup(src + start, pos - start);
                append_text(&out, &len, &cap, part);
                free(part);
            }
            append_text(&out, &len, &cap, "}");
            pos += 2;
            start = pos;
        } else {
            pos++;
        }
    }
    if (pos > start) {
        char *part = azr_strndup(src + start, pos - start);
        append_text(&out, &len, &cap, part);
        free(part);
    }
    {
        Value *v = make_string(out ? out : "");
        free(out);
        return v;
    }
}

static void tag_member_owner(Value *member, Value *class_value) {
    if (!member || !class_value) return;
    if (member->type == VAL_FUNCTION) {
        value_free(member->function.owner_class);
        member->function.owner_class = value_copy(class_value);
        return;
    }
    if (member->type == VAL_CLASS_METHOD && member->class_method.function) {
        tag_member_owner(member->class_method.function, class_value);
        return;
    }
    if (member->type == VAL_STATIC_METHOD && member->static_method.function) {
        tag_member_owner(member->static_method.function, class_value);
        return;
    }
    if (member->type == VAL_PROPERTY) {
        if (member->property.getter) tag_member_owner(member->property.getter, class_value);
        if (member->property.setter) tag_member_owner(member->property.setter, class_value);
    }
}

static Value *bind_member_value(Interpreter *i, Value *member, Value *receiver, Value *owner_class, int class_access) {
    if (!member) return NULL;
    if (member->type == VAL_FUNCTION) {
        if (!receiver || class_access) return value_copy(member);
        return make_bound_method(member, receiver);
    }
    if (member->type == VAL_CLASS_METHOD) {
        Value *target_class = owner_class;
        if (receiver && receiver->type == VAL_INSTANCE) target_class = receiver->instance.class_obj;
        if (!target_class) target_class = receiver;
        if (!target_class) return NULL;
        return make_bound_method(member->class_method.function, target_class);
    }
    if (member->type == VAL_STATIC_METHOD) {
        return member->static_method.function ? value_copy(member->static_method.function) : NULL;
    }
    if (member->type == VAL_PROPERTY) {
        if (class_access || !receiver) return value_copy(member);
        if (!member->property.getter) return interpreter_builtin_exception("خطای_نوع", "این ویژگی getter ندارد");
        return call_function_value(i, member->property.getter, receiver, NULL, NULL, 0, NULL);
    }
    return value_copy(member);
}

static Value *call_callable_value(Interpreter *i, Value *callable, Value **args, int argc) {
    if (!callable) return runtime_error(i, error_not_callable("هیچ"));
    if (callable->type == VAL_BOUND_METHOD) {
        return call_function_value(i, callable->bound_method.function, callable->bound_method.receiver, NULL, args, argc, NULL);
    }
    if (callable->type == VAL_FUNCTION) {
        return call_function_value(i, callable, NULL, NULL, args, argc, NULL);
    }
    return runtime_error(i, error_not_callable(value_type_name(callable)));
}

static int value_is_stop_iteration(Interpreter *i, Value *exc) {
    Value *stop_class = get_exception_class(i, "توقف_تکرار");
    if (!exc || !stop_class) return 0;
    if (exc->type == VAL_INSTANCE) return value_is_instance_of(exc, stop_class);
    if (exc->type == VAL_CLASS) return value_is_class_or_subclass(exc, stop_class);
    return 0;
}

static Value *iterator_from_value(Interpreter *i, Value *value) {
    Value *method = NULL;
    Value *result = NULL;
    if (!value) return NULL;
    switch (value->type) {
        case VAL_ITERATOR: return value_copy(value);
        case VAL_GENERATOR: return make_iterator(ITER_GENERATOR, value);
        case VAL_LIST: return make_iterator(ITER_LIST, value);
        case VAL_TUPLE: return make_iterator(ITER_TUPLE, value);
        case VAL_STRING: return make_iterator(ITER_STRING, value);
        case VAL_SET: return make_iterator(ITER_SET, value);
        case VAL_DICT: return make_iterator(ITER_DICT, value);
        case VAL_BYTES: return make_iterator(ITER_BYTES, value);
        default:
            break;
    }
    method = lookup_attr_value(i, value, "__iter__");
    if (!method) method = lookup_attr_value(i, value, "تکرار");
    if (method) {
        result = call_callable_value(i, method, NULL, 0);
        value_free(method);
        if (!result) return NULL;
        if (result->type == VAL_ITERATOR) return result;
        if (result->type == VAL_GENERATOR) { Value *it = make_iterator(ITER_GENERATOR, result); value_free(result); return it; }
        if (result->type == VAL_LIST) { Value *it = make_iterator(ITER_LIST, result); value_free(result); return it; }
        if (result->type == VAL_TUPLE) { Value *it = make_iterator(ITER_TUPLE, result); value_free(result); return it; }
        if (result->type == VAL_STRING) { Value *it = make_iterator(ITER_STRING, result); value_free(result); return it; }
        if (result->type == VAL_SET) { Value *it = make_iterator(ITER_SET, result); value_free(result); return it; }
        if (result->type == VAL_DICT) { Value *it = make_iterator(ITER_DICT, result); value_free(result); return it; }
        if (result->type == VAL_BYTES) { Value *it = make_iterator(ITER_BYTES, result); value_free(result); return it; }
        method = lookup_attr_value(i, result, "__next__");
        if (!method) method = lookup_attr_value(i, result, "بعدی");
        if (method) {
            value_free(method);
            {
                Value *it = make_iterator(ITER_CUSTOM, result);
                value_free(result);
                return it;
            }
        }
        value_free(result);
        return runtime_error(i, error_type("خروجی __iter__ باید تکرارگر باشد"));
    }
    method = lookup_attr_value(i, value, "__next__");
    if (!method) method = lookup_attr_value(i, value, "بعدی");
    if (method) {
        value_free(method);
        return make_iterator(ITER_CUSTOM, value);
    }
    return runtime_error(i, error_type("این مقدار قابل تکرار نیست"));
}

static GeneratorFrame *generator_top_frame(Value *generator) {
    if (!generator || generator->type != VAL_GENERATOR || generator->generator.frame_count <= 0) return NULL;
    return &generator->generator.frames[generator->generator.frame_count - 1];
}

static void generator_push_frame(Value *generator, int kind, ASTNode *node, Environment *env) {
    GeneratorFrame *frame = NULL;
    if (!generator || generator->type != VAL_GENERATOR) return;
    if (generator->generator.frame_count >= generator->generator.frame_cap) {
        generator->generator.frame_cap = generator->generator.frame_cap ? generator->generator.frame_cap * 2 : 8;
        generator->generator.frames = (GeneratorFrame *)realloc(generator->generator.frames, (size_t)generator->generator.frame_cap * sizeof(GeneratorFrame));
    }
    frame = &generator->generator.frames[generator->generator.frame_count++];
    frame->kind = kind;
    frame->node = node;
    frame->env = env;
    frame->index = 0;
    frame->phase = 0;
    frame->iterator = NULL;
}

static void generator_pop_frame(Value *generator) {
    GeneratorFrame *frame = NULL;
    if (!generator || generator->type != VAL_GENERATOR || generator->generator.frame_count <= 0) return;
    frame = &generator->generator.frames[--generator->generator.frame_count];
    value_free(frame->iterator);
    frame->iterator = NULL;
    frame->node = NULL;
    frame->env = NULL;
    frame->index = 0;
    frame->phase = 0;
}

static void generator_finish(Value *generator) {
    while (generator && generator->type == VAL_GENERATOR && generator->generator.frame_count > 0) generator_pop_frame(generator);
    if (generator && generator->type == VAL_GENERATOR) generator->generator.done = 1;
}

static int generator_handle_loop_signal(Interpreter *i, Value *generator, Signal signal) {
    if (generator && generator->type == VAL_GENERATOR && generator->generator.frame_count > 0 &&
        generator_top_frame(generator) && generator_top_frame(generator)->kind == GEN_FRAME_BLOCK) {
        generator_pop_frame(generator);
    }
    while (generator && generator->type == VAL_GENERATOR && generator->generator.frame_count > 0) {
        GeneratorFrame *frame = generator_top_frame(generator);
        if (!frame) break;
        if (frame->kind == GEN_FRAME_BLOCK) {
            generator_pop_frame(generator);
            continue;
        }
        if (frame->kind == GEN_FRAME_WHILE || frame->kind == GEN_FRAME_FOR) {
            if (signal == SIG_BREAK) {
                generator_pop_frame(generator);
            } else {
                frame->phase = 0;
            }
            return 1;
        }
        generator_pop_frame(generator);
    }
    runtime_error(i, error_type(signal == SIG_BREAK ? "بزن بیرون بیرون از حلقه است" : "ادامه بیرون از حلقه است"));
    return 0;
}

static Value *generator_next_value_internal(Interpreter *i, Value *generator, int *stopped) {
    int old_in_generator = 0;
    if (stopped) *stopped = 0;
    if (!generator || generator->type != VAL_GENERATOR) return runtime_error(i, error_type("بعدی فقط روی مولد کار می‌کند"));
    if (generator->generator.done) {
        if (stopped) *stopped = 1;
        return NULL;
    }
    if (generator->generator.frame_count == 0) generator_push_frame(generator, GEN_FRAME_BLOCK, generator->generator.body, generator->generator.env);
    old_in_generator = i->in_generator;
    i->in_generator = 1;
    push_frame(i, generator->generator.name, generator->generator.body ? generator->generator.body->line : i->current_line);
    while (generator->generator.frame_count > 0) {
        GeneratorFrame *frame = generator_top_frame(generator);
        if (!frame || !frame->node) break;
        if (frame->kind == GEN_FRAME_BLOCK) {
            ASTNode *block = frame->node;
            if (frame->index >= block->block.count) {
                generator_pop_frame(generator);
                continue;
            }
            {
                ASTNode *stmt = block->block.stmts[frame->index];
                ExecResult r;
                i->current_line = stmt ? stmt->line : i->current_line;
                if (!stmt) {
                    frame->index++;
                    continue;
                }
                if (stmt->type == NODE_IF) {
                    Value *cond = eval_expression(i, stmt->if_stmt.condition, frame->env);
                    ASTNode *branch = NULL;
                    int idx = 0;
                    if (!cond) { generator_finish(generator); pop_frame(i); i->in_generator = old_in_generator; return NULL; }
                    if (value_is_truthy(cond)) branch = stmt->if_stmt.then_block;
                    value_free(cond);
                    if (!branch) {
                        for (idx = 0; idx < stmt->if_stmt.elif_count; idx++) {
                            cond = eval_expression(i, stmt->if_stmt.elif_conditions[idx], frame->env);
                            if (!cond) { generator_finish(generator); pop_frame(i); i->in_generator = old_in_generator; return NULL; }
                            if (value_is_truthy(cond)) {
                                branch = stmt->if_stmt.elif_blocks[idx];
                                value_free(cond);
                                break;
                            }
                            value_free(cond);
                        }
                    }
                    if (!branch) branch = stmt->if_stmt.else_block;
                    frame->index++;
                    if (branch) generator_push_frame(generator, GEN_FRAME_BLOCK, branch, frame->env);
                    continue;
                }
                if (stmt->type == NODE_WHILE) {
                    frame->index++;
                    generator_push_frame(generator, GEN_FRAME_WHILE, stmt, frame->env);
                    continue;
                }
                if (stmt->type == NODE_FOR) {
                    frame->index++;
                    generator_push_frame(generator, GEN_FRAME_FOR, stmt, frame->env);
                    continue;
                }
                if (stmt->type == NODE_BLOCK || stmt->type == NODE_PROGRAM) {
                    frame->index++;
                    generator_push_frame(generator, GEN_FRAME_BLOCK, stmt, frame->env);
                    continue;
                }
                if (stmt->type != NODE_YIELD && ast_contains_yield(stmt)) {
                    runtime_error(i, error_type("فعلا تحویل فقط در بلوک‌ها و حلقه‌های مستقیم پشتیبانی می‌شود"));
                    generator_finish(generator);
                    pop_frame(i);
                    i->in_generator = old_in_generator;
                    return NULL;
                }
                r = exec_statement(i, stmt, frame->env);
                if (r.signal == SIG_NONE) {
                    value_free(r.value);
                    frame->index++;
                    continue;
                }
                if (r.signal == SIG_YIELD) {
                    frame->index++;
                    pop_frame(i);
                    i->in_generator = old_in_generator;
                    return r.value ? r.value : make_none();
                }
                if (r.signal == SIG_RETURN) {
                    value_free(r.value);
                    generator_finish(generator);
                    pop_frame(i);
                    i->in_generator = old_in_generator;
                    if (stopped) *stopped = 1;
                    return NULL;
                }
                if (r.signal == SIG_BREAK || r.signal == SIG_CONTINUE) {
                    if (!generator_handle_loop_signal(i, generator, r.signal)) {
                        generator_finish(generator);
                        pop_frame(i);
                        i->in_generator = old_in_generator;
                        return NULL;
                    }
                    continue;
                }
                generator_finish(generator);
                pop_frame(i);
                i->in_generator = old_in_generator;
                return runtime_error_object(i, r.value ? r.value : make_exception_instance(i, "خطا", "خطای ناشناخته"));
            }
        } else if (frame->kind == GEN_FRAME_WHILE) {
            if (frame->phase == 0) {
                Value *cond = eval_expression(i, frame->node->while_stmt.condition, frame->env);
                if (!cond) { generator_finish(generator); pop_frame(i); i->in_generator = old_in_generator; return NULL; }
                if (!value_is_truthy(cond)) {
                    value_free(cond);
                    generator_pop_frame(generator);
                    continue;
                }
                value_free(cond);
                frame->phase = 1;
                generator_push_frame(generator, GEN_FRAME_BLOCK, frame->node->while_stmt.body, frame->env);
                continue;
            }
            frame->phase = 0;
            continue;
        } else if (frame->kind == GEN_FRAME_FOR) {
            if (!frame->iterator) {
                Value *iterable = eval_expression(i, frame->node->for_stmt.iterable, frame->env);
                if (!iterable) { generator_finish(generator); pop_frame(i); i->in_generator = old_in_generator; return NULL; }
                frame->iterator = iterator_from_value(i, iterable);
                value_free(iterable);
                if (!frame->iterator) { generator_finish(generator); pop_frame(i); i->in_generator = old_in_generator; return NULL; }
            }
            if (frame->phase == 0) {
                int loop_stopped = 0;
                Value *item = iterator_next_value_internal(i, frame->iterator, &loop_stopped);
                if (!item && loop_stopped) {
                    generator_pop_frame(generator);
                    continue;
                }
                if (!item) { generator_finish(generator); pop_frame(i); i->in_generator = old_in_generator; return NULL; }
                env_set(frame->env, frame->node->for_stmt.var_name, item);
                frame->phase = 1;
                generator_push_frame(generator, GEN_FRAME_BLOCK, frame->node->for_stmt.body, frame->env);
                continue;
            }
            frame->phase = 0;
            continue;
        }
    }
    generator_finish(generator);
    pop_frame(i);
    i->in_generator = old_in_generator;
    if (stopped) *stopped = 1;
    return NULL;
}

static Value *iterator_next_value_internal(Interpreter *i, Value *iterator, int *stopped) {
    if (stopped) *stopped = 0;
    if (!iterator || iterator->type != VAL_ITERATOR) {
        return runtime_error(i, error_type("بعدی فقط روی تکرارگر کار می‌کند"));
    }
    switch (iterator->iterator.kind) {
        case ITER_LIST:
        case ITER_TUPLE: {
            Value *target = iterator->iterator.target;
            if (!target || iterator->iterator.index >= target->list.count) {
                if (stopped) *stopped = 1;
                return NULL;
            }
            return value_copy(target->list.items[iterator->iterator.index++]);
        }
        case ITER_SET: {
            Value *target = iterator->iterator.target;
            if (!target || iterator->iterator.index >= target->set.count) {
                if (stopped) *stopped = 1;
                return NULL;
            }
            return value_copy(target->set.items[iterator->iterator.index++]);
        }
        case ITER_DICT: {
            Value *target = iterator->iterator.target;
            if (!target || iterator->iterator.index >= target->dict.count) {
                if (stopped) *stopped = 1;
                return NULL;
            }
            return make_string(target->dict.keys[iterator->iterator.index++]);
        }
        case ITER_BYTES: {
            Value *target = iterator->iterator.target;
            if (!target || iterator->iterator.index >= target->bytes.count) {
                if (stopped) *stopped = 1;
                return NULL;
            }
            return make_int(target->bytes.data[iterator->iterator.index++]);
        }
        case ITER_STRING: {
            Value *target = iterator->iterator.target;
            int pos = 0, cp = 0, br = 0;
            if (!target) {
                if (stopped) *stopped = 1;
                return NULL;
            }
            while (target->str_val[pos]) {
                utf8_codepoint(target->str_val + pos, &br);
                if (cp == iterator->iterator.index) {
                    Value *out = make_string(azr_strndup(target->str_val + pos, br));
                    iterator->iterator.index++;
                    return out;
                }
                pos += br;
                cp++;
            }
            if (stopped) *stopped = 1;
            return NULL;
        }
        case ITER_CUSTOM: {
            Value *method = lookup_attr_value(i, iterator->iterator.target, "__next__");
            Value *out = NULL;
            if (!method) method = lookup_attr_value(i, iterator->iterator.target, "بعدی");
            if (!method) return runtime_error(i, error_type("این تکرارگر next ندارد"));
            out = call_callable_value(i, method, NULL, 0);
            value_free(method);
            if (!out && i->had_error && value_is_stop_iteration(i, i->current_exception)) {
                clear_runtime_error(i);
                if (stopped) *stopped = 1;
                return NULL;
            }
            return out;
        }
        case ITER_GENERATOR:
            return generator_next_value_internal(i, iterator->iterator.target, stopped);
        default:
            return runtime_error(i, error_type("نوع تکرارگر پشتیبانی نمی‌شود"));
    }
}

int interpreter_try_protocol_string(Value *v, Value **out) {
    Interpreter *i = g_active_interpreter;
    Value *method = NULL;
    Value *result = NULL;
    if (out) *out = NULL;
    if (!i || !v || v->type != VAL_INSTANCE) return 0;
    method = lookup_attr_value(i, v, "__str__");
    if (!method) method = lookup_attr_value(i, v, "متن");
    if (!method) return 0;
    result = call_callable_value(i, method, NULL, 0);
    value_free(method);
    if (!result) return 1;
    if (result->type != VAL_STRING) {
        value_free(result);
        interpreter_builtin_exception("خطای_نوع", "__str__ باید متن برگرداند");
        return 1;
    }
    if (out) *out = result;
    else value_free(result);
    return 1;
}

int interpreter_try_protocol_length(Value *v, Value **out) {
    Interpreter *i = g_active_interpreter;
    Value *method = NULL;
    Value *result = NULL;
    if (out) *out = NULL;
    if (!i || !v || v->type != VAL_INSTANCE) return 0;
    method = lookup_attr_value(i, v, "__len__");
    if (!method) return 0;
    result = call_callable_value(i, method, NULL, 0);
    value_free(method);
    if (!result) return 1;
    if (result->type != VAL_INT && result->type != VAL_FLOAT && result->type != VAL_BOOL) {
        value_free(result);
        interpreter_builtin_exception("خطای_نوع", "__len__ باید عدد برگرداند");
        return 1;
    }
    if (out) *out = make_int(integer(result));
    value_free(result);
    return 1;
}

int interpreter_try_protocol_equals(Value *a, Value *b, int *out, int *handled) {
    Interpreter *i = g_active_interpreter;
    Value *method = NULL;
    Value *result = NULL;
    Value *args[1];
    if (handled) *handled = 0;
    if (out) *out = 0;
    if (!i) return 0;
    if (a && a->type == VAL_INSTANCE) {
        method = lookup_attr_value(i, a, "__eq__");
        if (!method) method = lookup_attr_value(i, a, "برابر");
        if (method) {
            args[0] = b;
            result = call_callable_value(i, method, args, 1);
            value_free(method);
            if (handled) *handled = 1;
            if (!result) return 1;
            if (out) *out = value_is_truthy(result);
            value_free(result);
            return 1;
        }
    }
    if (b && b->type == VAL_INSTANCE) {
        method = lookup_attr_value(i, b, "__eq__");
        if (!method) method = lookup_attr_value(i, b, "برابر");
        if (method) {
            args[0] = a;
            result = call_callable_value(i, method, args, 1);
            value_free(method);
            if (handled) *handled = 1;
            if (!result) return 1;
            if (out) *out = value_is_truthy(result);
            value_free(result);
            return 1;
        }
    }
    return 0;
}

Value *interpreter_builtin_iter(Value *v) {
    return g_active_interpreter ? iterator_from_value(g_active_interpreter, v) : NULL;
}

Value *interpreter_builtin_next(Value *v) {
    int stopped = 0;
    Value *out = NULL;
    Value *owned_iterator = NULL;
    Value *target = v;
    if (!g_active_interpreter) return NULL;
    if (!target || target->type != VAL_ITERATOR) {
        owned_iterator = iterator_from_value(g_active_interpreter, v);
        if (!owned_iterator) return NULL;
        target = owned_iterator;
    }
    out = iterator_next_value_internal(g_active_interpreter, target, &stopped);
    value_free(owned_iterator);
    if (!out && stopped) {
        interpreter_builtin_exception("توقف_تکرار", "");
        return NULL;
    }
    return out;
}

