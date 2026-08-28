Interpreter *interpreter_new(void) {
    Interpreter *i = (Interpreter *)calloc(1, sizeof(Interpreter));
    Environment *error_env = NULL;
    Value *base_error = NULL;
    g_active_interpreter = i;
    i->global_env = env_new(NULL);
    i->in_generator = 0;
    i->module_cache = make_dict();
    i->stdlib_path = azr_strdup("stdlib");
    builtins_register(i->global_env);
    error_env = env_new(i->global_env);
    base_error = make_class("خطا", error_env, NULL);
    env_set(i->global_env, "خطا", base_error);
    env_set(i->global_env, "خطای_نام", make_class("خطای_نام", env_new(base_error->class_val.env), base_error));
    env_set(i->global_env, "خطای_نوع", make_class("خطای_نوع", env_new(base_error->class_val.env), base_error));
    env_set(i->global_env, "خطای_تقسیم_بر_صفر", make_class("خطای_تقسیم_بر_صفر", env_new(base_error->class_val.env), base_error));
    env_set(i->global_env, "خطای_شاخص", make_class("خطای_شاخص", env_new(base_error->class_val.env), base_error));
    env_set(i->global_env, "خطای_فایل", make_class("خطای_فایل", env_new(base_error->class_val.env), base_error));
    env_set(i->global_env, "خطای_واردکردن", make_class("خطای_واردکردن", env_new(base_error->class_val.env), base_error));
    env_set(i->global_env, "خطای_فراخوانی", make_class("خطای_فراخوانی", env_new(base_error->class_val.env), base_error));
    env_set(i->global_env, "خطای_ادعا", make_class("خطای_ادعا", env_new(base_error->class_val.env), base_error));
    env_set(i->global_env, "توقف_تکرار", make_class("توقف_تکرار", env_new(base_error->class_val.env), base_error));
    return i;
}

/* Sets the standard-library directory used by بیار imports. */
void interpreter_set_stdlib_path(Interpreter *i, const char *path) {
    if (!i || !path) return;
    free(i->stdlib_path);
    i->stdlib_path = azr_strdup(path);
}

void interpreter_set_current_file(Interpreter *i, const char *path) {
    if (!i) return;
    free(i->current_file);
    i->current_file = azr_strdup(path ? path : "<ورودی>");
}

void interpreter_set_main_file(Interpreter *i, const char *path) {
    if (!i) return;
    interpreter_set_current_file(i, path ? path : "<ورودی>");
    free(i->entry_dir);
    i->entry_dir = path_dirname_dup(path ? path : ".");
    free(i->module_name);
    i->module_name = azr_strdup("__main__");
    free(i->module_package);
    i->module_package = azr_strdup("");
    set_module_metadata(i->global_env, "__main__", i->current_file, "");
}

void interpreter_add_import_root(Interpreter *i, const char *path) {
    int idx = 0;
    if (!i || !path || !*path) return;
    for (idx = 0; idx < i->import_root_count; idx++) {
        if (strcmp(i->import_roots[idx], path) == 0) return;
    }
    if (i->import_root_count == i->import_root_cap) {
        i->import_root_cap = i->import_root_cap ? i->import_root_cap * 2 : 4;
        i->import_roots = (char **)realloc(i->import_roots, (size_t)i->import_root_cap * sizeof(char *));
    }
    i->import_roots[i->import_root_count++] = azr_strdup(path);
}

static void tag_class_methods(Environment *class_env, Value *class_value) {
    int idx = 0;
    if (!class_env || !class_value) return;
    for (idx = 0; idx < class_env->count; idx++) {
        tag_member_owner(class_env->values[idx], class_value);
    }
}

/* Executes a program AST in the interpreter global scope. */
ExecResult exec_program(Interpreter *i, ASTNode *node) {
    return exec_block(i, node, i->global_env);
}

/* Executes each statement in a block using the supplied environment. */
ExecResult exec_block(Interpreter *i, ASTNode *node, Environment *env) {
    int k = 0;
    ExecResult last = ok(make_none());
    for (k = 0; node && k < node->block.count; k++) {
        value_free(last.value);
        last = exec_statement(i, node->block.stmts[k], env);
        if (last.signal != SIG_NONE) return last;
        if (i->had_error) return current_error_result(i);
    }
    return last;
}

/* Evaluates a statement and returns control-flow information. */
ExecResult exec_statement(Interpreter *i, ASTNode *node, Environment *env) {
    i->current_line = node ? node->line : i->current_line;
    if (!node) return ok(make_none());
    switch (node->type) {
        case NODE_BLOCK:
        case NODE_PROGRAM: {
            Environment *child = env_new(env);
            ExecResult r = exec_block(i, node, child);
            child->owns_values = 0;
            env_free(child);
            return r;
        }
        case NODE_ASSIGN: {
            Value *v = eval_expression(i, node->assign.value, env);
            if (!v) return current_error_result(i);
            env_set(env, node->assign.name, v);
            return ok(make_none());
        }
        case NODE_AUGMENTED_ASSIGN: {
            Value *oldv = env_get(env, node->aug_assign.name);
            Value *right = eval_expression(i, node->aug_assign.value, env);
            Value *newv = NULL;
            if (!oldv) return raise_result(i, error_undefined(node->aug_assign.name));
            if (!right) return current_error_result(i);
            newv = apply_augmented_values(i, oldv, node->aug_assign.op, right);
            value_free(right);
            if (!newv) return current_error_result(i);
            env_set(env, node->aug_assign.name, newv);
            return ok(make_none());
        }
        case NODE_MULTI_ASSIGN: {
            int k = 0;
            Value *v = eval_expression(i, node->multi_assign.value, env);
            if (!v) return current_error_result(i);
            if ((v->type != VAL_LIST && v->type != VAL_TUPLE) || v->list.count != node->multi_assign.target_count) {
                value_free(v);
                return raise_result(i, error_type("انتساب چندتایی به تعداد برابر مقدار نیاز داره"));
            }
            for (k = 0; k < node->multi_assign.target_count; k++) bind_destructure(i, env, node->multi_assign.targets[k], v->list.items[k]);
            value_free(v);
            if (i->had_error) return current_error_result(i);
            return ok(make_none());
        }
        case NODE_INDEX_ASSIGN:
            return exec_index_assignment(i, node->index_assign.object, node->index_assign.index, NULL, node->index_assign.value, env);
        case NODE_INDEX_AUGMENTED_ASSIGN:
            return exec_index_assignment(i, node->index_aug_assign.object, node->index_aug_assign.index, node->index_aug_assign.op, node->index_aug_assign.value, env);
        case NODE_ATTR_ASSIGN:
            return exec_attr_assignment(i, node, env);
        case NODE_IF: return exec_if(i, node, env);
        case NODE_WHILE: return exec_while(i, node, env);
        case NODE_FOR: return exec_for(i, node, env);
        case NODE_FUNCTION_DEF: return exec_function_def(i, node, env);
        case NODE_CLASS_DEF: {
            Value *base = NULL;
            Environment *class_env = NULL;
            Value *class_value = NULL;
            ExecResult r;
            if (node->class_def.base_class) {
                base = eval_expression(i, node->class_def.base_class, env);
                if (!base) return current_error_result(i);
                if (base->type != VAL_CLASS) {
                    value_free(base);
                    return raise_result(i, error_type("پایه‌ی نوع باید خودش یک نوع باشد"));
                }
            }
            class_env = env_new(base && base->type == VAL_CLASS ? base->class_val.env : env);
            class_value = make_class(node->class_def.name, class_env, base);
            r = exec_block(i, node->class_def.body, class_env);
            value_free(base);
            if (r.signal != SIG_NONE) return r;
            value_free(r.value);
            class_env->owns_values = 1;
            tag_class_methods(class_env, class_value);
            env_set(env, node->class_def.name, class_value);
            return ok(make_none());
        }
        case NODE_RETURN: return exec_return(i, node, env);
        case NODE_YIELD: {
            Value *v = NULL;
            if (!i->in_generator) return raise_result(i, error_type("تحویل فقط داخل کارهای مولد مجاز است"));
            v = node->yield_stmt.value ? eval_expression(i, node->yield_stmt.value, env) : make_none();
            if (!v) return current_error_result(i);
            return sig(SIG_YIELD, v);
        }
        case NODE_THROW: {
            Value *v = eval_expression(i, node->throw_stmt.value, env);
            Value *exc = NULL;
            if (!v) return current_error_result(i);
            exc = coerce_exception_value(i, v);
            value_free(v);
            runtime_error_object(i, exc);
            return current_error_result(i);
        }
        case NODE_BREAK: return sig(SIG_BREAK, NULL);
        case NODE_CONTINUE: return sig(SIG_CONTINUE, NULL);
        case NODE_PASS: return ok(make_none());
        case NODE_IMPORT: {
            Value *module = NULL;
            Value *bind_value = NULL;
            char *abs_name = resolve_import_name(i, node->import.module_name);
            const char *bind_name = node->import.alias;
            if (!abs_name) return current_error_result(i);
            module = load_module_absolute(i, abs_name);
            if (!module) { free(abs_name); return current_error_result(i); }
            if (bind_name) {
                env_set(env, bind_name, value_copy(module));
            } else if (node->import.module_name[0] == '.') {
                char *leaf = module_leaf_name(abs_name);
                env_set(env, leaf, value_copy(module));
                free(leaf);
            } else {
                const char *dot = strchr(abs_name, '.');
                if (dot) {
                    char *top_name = azr_strndup(abs_name, (int)(dot - abs_name));
                    bind_value = dict_get(i->module_cache, top_name);
                    env_set(env, top_name, value_copy(bind_value ? bind_value : module));
                    free(top_name);
                } else {
                    env_set(env, abs_name, value_copy(module));
                }
            }
            value_free(module);
            free(abs_name);
            return ok(make_none());
        }
        case NODE_FROM_IMPORT: {
            Value *mod = NULL;
            char *abs_name = resolve_import_name(i, node->from_import.module_name);
            int k = 0;
            if (!abs_name) return current_error_result(i);
            if (*abs_name) {
                mod = load_module_absolute(i, abs_name);
                if (!mod) { free(abs_name); return current_error_result(i); }
            }
            for (k = 0; k < node->from_import.count; k++) {
                Value *v = mod ? env_get_local(mod->module.env, node->from_import.names[k]) : NULL;
                if (!v) {
                    char *qualified = NULL;
                    Value *child = NULL;
                    char *tmp_path = NULL;
                    int tmp_is_package = 0;
                    if (*abs_name) {
                        qualified = (char *)malloc(strlen(abs_name) + strlen(node->from_import.names[k]) + 2);
                        sprintf(qualified, "%s.%s", abs_name, node->from_import.names[k]);
                    } else {
                        qualified = azr_strdup(node->from_import.names[k]);
                    }
                    if (resolve_module_spec(i, qualified, &tmp_path, &tmp_is_package, NULL)) {
                        free(tmp_path);
                        child = load_module_absolute(i, qualified);
                        if (!child) {
                            value_free(mod);
                            free(abs_name);
                            free(qualified);
                            return current_error_result(i);
                        }
                        env_set(env, node->from_import.aliases[k] ? node->from_import.aliases[k] : node->from_import.names[k], child);
                        free(qualified);
                        continue;
                    }
                    free(qualified);
                    value_free(mod);
                    free(abs_name);
                    return raise_result(i, error_undefined(node->from_import.names[k]));
                }
                env_set(env, node->from_import.aliases[k] ? node->from_import.aliases[k] : node->from_import.names[k], value_copy(v));
            }
            value_free(mod);
            free(abs_name);
            return ok(make_none());
        }
        case NODE_TRY_EXCEPT: return exec_try_except(i, node, env);
        case NODE_WITH: {
            Value *ctx = eval_expression(i, node->with_stmt.expr, env);
            Value *entered = NULL;
            Value *enter = NULL;
            Value *exitm = NULL;
            ExecResult r;
            Environment *child = env_new(env);
            if (!ctx) { env_free(child); return current_error_result(i); }
            enter = lookup_attr_value(i, ctx, "ورود");
            if (!enter) enter = lookup_attr_value(i, ctx, "__enter__");
            if (enter) entered = call_callable_value(i, enter, NULL, 0);
            if (i->had_error) { env_free(child); value_free(ctx); value_free(enter); return current_error_result(i); }
            if (node->with_stmt.alias) env_set(child, node->with_stmt.alias, value_copy(entered ? entered : ctx));
            r = exec_block(i, node->with_stmt.body, child);
            exitm = lookup_attr_value(i, ctx, "خروج");
            if (!exitm) exitm = lookup_attr_value(i, ctx, "__exit__");
            if (exitm) {
                Value *tmp = call_callable_value(i, exitm, NULL, 0);
                value_free(tmp);
            }
            env_free(child);
            value_free(entered);
            value_free(enter);
            value_free(exitm);
            value_free(ctx);
            return r;
        }
        case NODE_EXPR_STMT: {
            Value *v = eval_expression(i, node->expr_stmt.expr, env);
            if (!v) return current_error_result(i);
            return ok(v);
        }
        default: {
            Value *v = eval_expression(i, node, env);
            if (!v) return current_error_result(i);
            return ok(v);
        }
    }
}

/* Executes an if statement with elif and else support. */
ExecResult exec_if(Interpreter *i, ASTNode *node, Environment *env) {
    int k = 0;
    Value *cond = eval_expression(i, node->if_stmt.condition, env);
    if (!cond) return current_error_result(i);
    if (value_is_truthy(cond)) { value_free(cond); return exec_block(i, node->if_stmt.then_block, env); }
    value_free(cond);
    for (k = 0; k < node->if_stmt.elif_count; k++) {
        cond = eval_expression(i, node->if_stmt.elif_conditions[k], env);
        if (!cond) return current_error_result(i);
        if (value_is_truthy(cond)) { value_free(cond); return exec_block(i, node->if_stmt.elif_blocks[k], env); }
        value_free(cond);
    }
    if (node->if_stmt.else_block) return exec_block(i, node->if_stmt.else_block, env);
    return ok(make_none());
}

/* Executes a while loop and honors break, continue, and return. */
ExecResult exec_while(Interpreter *i, ASTNode *node, Environment *env) {
    while (1) {
        Value *cond = eval_expression(i, node->while_stmt.condition, env);
        ExecResult r;
        if (!cond) return current_error_result(i);
        if (!value_is_truthy(cond)) { value_free(cond); break; }
        value_free(cond);
        r = exec_block(i, node->while_stmt.body, env);
        if (r.signal == SIG_BREAK) return ok(make_none());
        if (r.signal == SIG_CONTINUE) continue;
        if (r.signal != SIG_NONE) return r;
        value_free(r.value);
    }
    return ok(make_none());
}

/* Executes a for-each loop through the iterator protocol. */
ExecResult exec_for(Interpreter *i, ASTNode *node, Environment *env) {
    Value *iter = eval_expression(i, node->for_stmt.iterable, env);
    Value *iterator = NULL;
    if (!iter) return current_error_result(i);
    iterator = iterator_from_value(i, iter);
    value_free(iter);
    if (!iterator) return current_error_result(i);
    while (1) {
        Value *item = NULL;
        ExecResult r;
        int stopped = 0;
        item = iterator_next_value_internal(i, iterator, &stopped);
        if (!item && stopped) break;
        if (!item) {
            value_free(iterator);
            return current_error_result(i);
        }
        env_set(env, node->for_stmt.var_name, item);
        r = exec_block(i, node->for_stmt.body, env);
        if (r.signal == SIG_BREAK) break;
        if (r.signal == SIG_CONTINUE) continue;
        if (r.signal != SIG_NONE) { value_free(iterator); return r; }
        value_free(r.value);
    }
    value_free(iterator);
    return ok(make_none());
}

/* Defines a function in the current scope while capturing the lexical environment. */
ExecResult exec_function_def(Interpreter *i, ASTNode *node, Environment *env) {
    char **params = (char **)calloc((size_t)node->func_def.param_count, sizeof(char *));
    int k = 0;
    (void)i;
    for (k = 0; k < node->func_def.param_count; k++) params[k] = azr_strdup(node->func_def.params[k]);
    env_set(env, node->func_def.name, make_function(node->func_def.name, params, node->func_def.defaults, node->func_def.param_count, node->func_def.vararg, node->func_def.body, env, ast_contains_yield(node->func_def.body)));
    return ok(make_none());
}

/* Executes a return statement and packages the returned value. */
ExecResult exec_return(Interpreter *i, ASTNode *node, Environment *env) {
    Value *v = node->return_stmt.value ? eval_expression(i, node->return_stmt.value, env) : make_none();
    if (!v) return current_error_result(i);
    return sig(SIG_RETURN, v);
}

static int exception_matches(Interpreter *i, Value *exc, ASTNode *type_node, Environment *env) {
    Value *expected = NULL;
    int match = 0;
    Value *base_error = NULL;
    if (!type_node) return 1;
    expected = eval_expression(i, type_node, env);
    if (!expected) return -1;
    if (expected->type != VAL_CLASS) {
        value_free(expected);
        runtime_error(i, error_type("بعد از 'اگه خطا' باید نام یک نوع خطا بیاد"));
        return -1;
    }
    base_error = get_exception_class(i, "خطا");
    if (exc && exc->type == VAL_INSTANCE) match = value_is_instance_of(exc, expected);
    else if (exc && exc->type == VAL_CLASS && base_error && value_is_class_or_subclass(exc, expected)) match = 1;
    value_free(expected);
    return match;
}

/* Executes try/except/finally with errors routed to the except block. */
ExecResult exec_try_except(Interpreter *i, ASTNode *node, Environment *env) {
    ExecResult r = exec_block(i, node->try_except.try_block, env);
    if (r.signal == SIG_ERROR || i->had_error) {
        Value *exc = r.value ? r.value : (i->current_exception ? value_copy(i->current_exception) : make_exception_instance(i, "خطا", "خطای ناشناخته"));
        int handled = 0;
        int idx = 0;
        clear_runtime_error(i);
        for (idx = 0; idx < node->try_except.except_count; idx++) {
            int match = exception_matches(i, exc, node->try_except.exception_types[idx], env);
            if (match < 0) {
                value_free(exc);
                return current_error_result(i);
            }
            if (!match) continue;
            if (node->try_except.exception_vars[idx]) env_set(env, node->try_except.exception_vars[idx], value_copy(exc));
            r = exec_block(i, node->try_except.except_blocks[idx], env);
            handled = 1;
            break;
        }
        if (!handled) {
            runtime_error_object(i, exc);
            r = current_error_result(i);
        } else {
            value_free(exc);
        }
    }
    if (node->try_except.finally_block) {
        ExecResult fr = exec_block(i, node->try_except.finally_block, env);
        if (fr.signal != SIG_NONE) return fr;
        value_free(fr.value);
    }
    return r;
}

