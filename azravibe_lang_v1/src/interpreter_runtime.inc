static ExecResult ok(Value *v) { ExecResult r; r.signal = SIG_NONE; r.value = v; return r; }

/* Builds a control-flow execution result. */
static ExecResult sig(Signal s, Value *v) { ExecResult r; r.signal = s; r.value = v; return r; }

static int ast_contains_yield(ASTNode *node) {
    int idx = 0;
    if (!node) return 0;
    if (node->type == NODE_YIELD) return 1;
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_BLOCK:
            for (idx = 0; idx < node->block.count; idx++) if (ast_contains_yield(node->block.stmts[idx])) return 1;
            return 0;
        case NODE_ASSIGN:
            return ast_contains_yield(node->assign.value);
        case NODE_MULTI_ASSIGN:
            if (ast_contains_yield(node->multi_assign.value)) return 1;
            for (idx = 0; idx < node->multi_assign.target_count; idx++) if (ast_contains_yield(node->multi_assign.targets[idx])) return 1;
            return 0;
        case NODE_AUGMENTED_ASSIGN:
            return ast_contains_yield(node->aug_assign.value);
        case NODE_IF:
            if (ast_contains_yield(node->if_stmt.condition) || ast_contains_yield(node->if_stmt.then_block) || ast_contains_yield(node->if_stmt.else_block)) return 1;
            for (idx = 0; idx < node->if_stmt.elif_count; idx++) {
                if (ast_contains_yield(node->if_stmt.elif_conditions[idx]) || ast_contains_yield(node->if_stmt.elif_blocks[idx])) return 1;
            }
            return 0;
        case NODE_WHILE:
            return ast_contains_yield(node->while_stmt.condition) || ast_contains_yield(node->while_stmt.body);
        case NODE_FOR:
            return ast_contains_yield(node->for_stmt.iterable) || ast_contains_yield(node->for_stmt.body);
        case NODE_FUNCTION_DEF:
        case NODE_LAMBDA:
            return 0;
        case NODE_CLASS_DEF:
            return ast_contains_yield(node->class_def.base_class) || ast_contains_yield(node->class_def.body);
        case NODE_RETURN:
            return ast_contains_yield(node->return_stmt.value);
        case NODE_CALL:
            if (ast_contains_yield(node->call.callee)) return 1;
            for (idx = 0; idx < node->call.arg_count; idx++) if (ast_contains_yield(node->call.args[idx])) return 1;
            return 0;
        case NODE_BINARY_OP:
            return ast_contains_yield(node->binary.left) || ast_contains_yield(node->binary.right);
        case NODE_UNARY_OP:
            return ast_contains_yield(node->unary.operand);
        case NODE_LIST_LITERAL:
        case NODE_TUPLE_LITERAL:
            for (idx = 0; idx < node->list_lit.count; idx++) if (ast_contains_yield(node->list_lit.items[idx])) return 1;
            return 0;
        case NODE_DICT_LITERAL:
        case NODE_SET_LITERAL:
            for (idx = 0; idx < node->dict_lit.count; idx++) {
                if (ast_contains_yield(node->dict_lit.keys[idx]) || ast_contains_yield(node->dict_lit.values[idx])) return 1;
            }
            return 0;
        case NODE_INDEX:
            return ast_contains_yield(node->index.object) || ast_contains_yield(node->index.index);
        case NODE_SLICE:
            return ast_contains_yield(node->slice.object) || ast_contains_yield(node->slice.start) || ast_contains_yield(node->slice.end);
        case NODE_IMPORT:
        case NODE_FROM_IMPORT:
        case NODE_BREAK:
        case NODE_CONTINUE:
        case NODE_PASS:
        case NODE_YIELD:
        case NODE_IDENTIFIER:
        case NODE_INT_LITERAL:
        case NODE_FLOAT_LITERAL:
        case NODE_STRING_LITERAL:
        case NODE_BOOL_LITERAL:
        case NODE_NONE_LITERAL:
            return 0;
        case NODE_TRY_EXCEPT:
            if (ast_contains_yield(node->try_except.try_block) || ast_contains_yield(node->try_except.finally_block)) return 1;
            for (idx = 0; idx < node->try_except.except_count; idx++) {
                if (ast_contains_yield(node->try_except.exception_types[idx]) || ast_contains_yield(node->try_except.except_blocks[idx])) return 1;
            }
            return 0;
        case NODE_THROW:
            return ast_contains_yield(node->throw_stmt.value);
        case NODE_ATTRIBUTE:
            return ast_contains_yield(node->attribute.object);
        case NODE_ATTR_ASSIGN:
            return ast_contains_yield(node->attr_assign.object) || ast_contains_yield(node->attr_assign.value);
        case NODE_WITH:
            return ast_contains_yield(node->with_stmt.expr) || ast_contains_yield(node->with_stmt.body);
        case NODE_TERNARY:
            return ast_contains_yield(node->ternary.then_expr) || ast_contains_yield(node->ternary.condition) || ast_contains_yield(node->ternary.else_expr);
        case NODE_LIST_COMPREHENSION:
        case NODE_SET_COMPREHENSION:
        case NODE_DICT_COMPREHENSION:
            if (ast_contains_yield(node->comprehension.item_expr) || ast_contains_yield(node->comprehension.value_expr)) return 1;
            for (idx = 0; idx < node->comprehension.loop_count; idx++) {
                if (ast_contains_yield(node->comprehension.iterables[idx]) || ast_contains_yield(node->comprehension.conditions[idx])) return 1;
            }
            return 0;
        case NODE_INDEX_ASSIGN:
            return ast_contains_yield(node->index_assign.object) || ast_contains_yield(node->index_assign.index) || ast_contains_yield(node->index_assign.value);
        case NODE_INDEX_AUGMENTED_ASSIGN:
            return ast_contains_yield(node->index_aug_assign.object) || ast_contains_yield(node->index_aug_assign.index) || ast_contains_yield(node->index_aug_assign.value);
        case NODE_EXPR_STMT:
            return ast_contains_yield(node->expr_stmt.expr);
    }
    return 0;
}

static void clear_runtime_error(Interpreter *i) {
    if (!i) return;
    i->had_error = 0;
    free(i->error_message);
    i->error_message = NULL;
    value_free(i->current_exception);
    i->current_exception = NULL;
}

static ExecResult current_error_result(Interpreter *i) {
    return sig(SIG_ERROR, i && i->current_exception ? value_copy(i->current_exception) : NULL);
}

static const char *infer_exception_class(const char *msg) {
    if (!msg) return "خطا";
    if (strncmp(msg, "خطای نوع:", strlen("خطای نوع:")) == 0) return "خطای_نوع";
    if (strstr(msg, "تقسیم بر صفر")) return "خطای_تقسیم_بر_صفر";
    if (strstr(msg, "اندیس")) return "خطای_شاخص";
    if (strstr(msg, "متغیر") || strstr(msg, "کلید")) return "خطای_نام";
    if (strstr(msg, "فایل")) return "خطای_فایل";
    if (strstr(msg, "بیار")) return "خطای_واردکردن";
    if (strstr(msg, "آرگومان") || strstr(msg, "قابل فراخوانی")) return "خطای_فراخوانی";
    return "خطا";
}

static void push_frame(Interpreter *i, const char *name, int line) {
    CallFrame *frame = NULL;
    if (!i) return;
    if (i->frame_count >= i->frame_cap) {
        i->frame_cap = i->frame_cap ? i->frame_cap * 2 : 8;
        i->frames = (CallFrame *)realloc(i->frames, (size_t)i->frame_cap * sizeof(CallFrame));
    }
    frame = &i->frames[i->frame_count++];
    frame->name = azr_strdup(name ? name : "<کار>");
    frame->file = azr_strdup(i->current_file ? i->current_file : "<ورودی>");
    frame->line = line;
}

static void pop_frame(Interpreter *i) {
    CallFrame *frame = NULL;
    if (!i || i->frame_count <= 0) return;
    frame = &i->frames[--i->frame_count];
    free(frame->name);
    free(frame->file);
    frame->name = NULL;
    frame->file = NULL;
    frame->line = 0;
}

static Value *get_exception_class(Interpreter *i, const char *name) {
    Value *cls = i ? env_get(i->global_env, name) : NULL;
    if (cls && cls->type == VAL_CLASS) return cls;
    return NULL;
}

static Value *make_traceback_frame(const char *file, int line, const char *function_name) {
    Value *frame = make_dict();
    dict_set(frame, "file", make_string(file ? file : "<ورودی>"));
    dict_set(frame, "line", make_int(line > 0 ? line : 0));
    dict_set(frame, "function", make_string(function_name ? function_name : "سطح_بالا"));
    return frame;
}

static void attach_traceback(Interpreter *i, Value *exc) {
    Value *traceback = NULL;
    int idx = 0;
    const char *current_function = "سطح_بالا";
    if (!i || !exc || exc->type != VAL_INSTANCE) return;
    traceback = make_list();
    if (i->frame_count > 0) current_function = i->frames[i->frame_count - 1].name;
    list_append(traceback, make_traceback_frame(i->current_file, i->current_line, current_function));
    for (idx = i->frame_count - 1; idx >= 0; idx--) {
        list_append(traceback, make_traceback_frame(i->frames[idx].file, i->frames[idx].line, i->frames[idx].name));
    }
    env_set(exc->instance.fields, "traceback", traceback);
    env_set(exc->instance.fields, "فایل", make_string(i->current_file ? i->current_file : "<ورودی>"));
    env_set(exc->instance.fields, "خط", make_int(i->current_line > 0 ? i->current_line : 0));
    env_set(exc->instance.fields, "تابع", make_string(current_function));
}

static Value *make_exception_instance(Interpreter *i, const char *class_name, const char *message) {
    Value *class_obj = get_exception_class(i, class_name ? class_name : "خطا");
    Environment *fields = NULL;
    Value *exc = NULL;
    if (!class_obj) class_obj = get_exception_class(i, "خطا");
    if (!class_obj) return make_string(message ? message : "خطای ناشناخته");
    fields = env_new(class_obj->class_val.env);
    exc = make_instance(class_obj, fields);
    env_set(fields, "پیام", make_string(message ? message : ""));
    env_set(fields, "message", make_string(message ? message : ""));
    env_set(fields, "نوع", make_string(class_obj->class_val.name));
    attach_traceback(i, exc);
    return exc;
}

static Value *coerce_exception_value(Interpreter *i, Value *raised) {
    Value *base_error = get_exception_class(i, "خطا");
    if (!raised) return make_exception_instance(i, "خطا", "خطای ناشناخته");
    if (raised->type == VAL_INSTANCE && base_error && value_is_instance_of(raised, base_error)) {
        attach_traceback(i, raised);
        return value_copy(raised);
    }
    if (raised->type == VAL_CLASS && base_error && value_is_class_or_subclass(raised, base_error)) {
        Environment *fields = env_new(raised->class_val.env);
        Value *exc = make_instance(raised, fields);
        env_set(fields, "پیام", make_string(""));
        env_set(fields, "message", make_string(""));
        env_set(fields, "نوع", make_string(raised->class_val.name));
        attach_traceback(i, exc);
        return exc;
    }
    {
        char *message = value_to_string(raised);
        Value *exc = make_exception_instance(i, "خطا", message);
        free(message);
        return exc;
    }
}

static Value *runtime_error_object(Interpreter *i, Value *exc) {
    if (!i || !exc) return NULL;
    value_free(i->current_exception);
    i->current_exception = exc;
    i->had_error = 1;
    free(i->error_message);
    i->error_message = format_exception(exc);
    return NULL;
}

/* Records an interpreter error and returns NULL for expression evaluation. */
static Value *runtime_error(Interpreter *i, char *msg) {
    Value *exc = make_exception_instance(i, infer_exception_class(msg), msg ? msg : "خطا");
    free(msg);
    return runtime_error_object(i, exc);
}

static ExecResult raise_result(Interpreter *i, char *msg) {
    runtime_error(i, msg);
    return current_error_result(i);
}

Value *interpreter_builtin_exception(const char *class_name, const char *message) {
    if (!g_active_interpreter) return NULL;
    return runtime_error_object(g_active_interpreter, make_exception_instance(g_active_interpreter, class_name ? class_name : "خطا", message ? message : ""));
}

static char *path_dirname_dup(const char *path) {
    const char *slash1 = NULL;
    const char *slash2 = NULL;
    const char *slash = NULL;
    size_t len = 0;
    if (!path || !*path) return azr_strdup(".");
    slash1 = strrchr(path, '/');
    slash2 = strrchr(path, '\\');
    slash = slash1;
    if (!slash || (slash2 && slash2 > slash)) slash = slash2;
    if (!slash) return azr_strdup(".");
    len = (size_t)(slash - path);
    if (len == 0) return azr_strdup(".");
    return azr_strndup(path, (int)len);
}

static int file_exists(const char *path) {
    FILE *f = NULL;
    if (!path) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static char *read_text_file(const char *path) {
    FILE *f = fopen(path, "rb");
    long len = 0;
    char *buf = NULL;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    rewind(f);
    buf = (char *)malloc((size_t)len + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, (size_t)len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

static char *module_name_to_relpath(const char *module_name) {
    char *path = azr_strdup(module_name ? module_name : "");
    int idx = 0;
    while (path[idx]) {
        if (path[idx] == '.') path[idx] = '/';
        idx++;
    }
    return path;
}

static char *module_parent_name(const char *module_name) {
    const char *dot = NULL;
    if (!module_name || !*module_name) return azr_strdup("");
    dot = strrchr(module_name, '.');
    if (!dot) return azr_strdup("");
    return azr_strndup(module_name, (int)(dot - module_name));
}

static char *module_leaf_name(const char *module_name) {
    const char *dot = NULL;
    if (!module_name) return azr_strdup("");
    dot = strrchr(module_name, '.');
    return azr_strdup(dot ? dot + 1 : module_name);
}

static int resolve_under_root(const char *root, const char *rel, char **out_path, int *is_package, int *is_native) {
    char path[1024];
    if (!root || !*root || !rel || !*rel) return 0;
    snprintf(path, sizeof(path), "%s/%s.azr", root, rel);
    if (file_exists(path)) {
        *out_path = azr_strdup(path);
        if (is_package) *is_package = 0;
        if (is_native) *is_native = 0;
        return 1;
    }
    snprintf(path, sizeof(path), "%s/%s/__init__.azr", root, rel);
    if (file_exists(path)) {
        *out_path = azr_strdup(path);
        if (is_package) *is_package = 1;
        if (is_native) *is_native = 0;
        return 1;
    }
    if (azr_native_path_for_module(root, rel, out_path)) {
        if (is_package) *is_package = 0;
        if (is_native) *is_native = 1;
        return 1;
    }
    return 0;
}

static int module_is_package(Value *module) {
    Value *name = NULL;
    Value *package_name = NULL;
    if (!module || module->type != VAL_MODULE) return 0;
    name = env_get_local(module->module.env, "__name__");
    package_name = env_get_local(module->module.env, "__package__");
    if (!name || !package_name || name->type != VAL_STRING || package_name->type != VAL_STRING) return 0;
    return strcmp(name->str_val, package_name->str_val) == 0;
}

static void set_module_metadata(Environment *env, const char *module_name, const char *file, const char *package_name) {
    if (!env) return;
    env_set(env, "__name__", make_string(module_name ? module_name : ""));
    env_set(env, "__file__", make_string(file ? file : "<ورودی>"));
    env_set(env, "__package__", make_string(package_name ? package_name : ""));
}

static char *resolve_import_name(Interpreter *i, const char *requested) {
    const char *rest = requested ? requested : "";
    int leading = 0;
    char *base = NULL;
    char *result = NULL;
    while (*rest == '.') {
        leading++;
        rest++;
    }
    if (leading == 0) return azr_strdup(requested ? requested : "");
    if (!i || !i->module_package || !*i->module_package) {
        runtime_error(i, error_import(requested ? requested : ""));
        return NULL;
    }
    base = azr_strdup(i->module_package);
    while (leading > 1) {
        char *dot = NULL;
        if (!*base) {
            free(base);
            runtime_error(i, error_import(requested ? requested : ""));
            return NULL;
        }
        dot = strrchr(base, '.');
        if (dot) *dot = '\0';
        else base[0] = '\0';
        leading--;
    }
    if (*base && *rest) {
        result = (char *)malloc(strlen(base) + strlen(rest) + 2);
        sprintf(result, "%s.%s", base, rest);
    } else if (*rest) {
        result = azr_strdup(rest);
    } else {
        result = azr_strdup(base);
    }
    free(base);
    return result;
}

static int resolve_module_spec(Interpreter *i, const char *abs_name, char **out_path, int *is_package, int *is_native) {
    char *rel = NULL;
    int idx = 0;
    if (!abs_name || !*abs_name) return 0;
    rel = module_name_to_relpath(abs_name);
    if (i) {
        for (idx = 0; idx < i->import_root_count; idx++) {
            if (resolve_under_root(i->import_roots[idx], rel, out_path, is_package, is_native)) {
                free(rel);
                return 1;
            }
        }
    }
    if (i && i->entry_dir && *i->entry_dir && resolve_under_root(i->entry_dir, rel, out_path, is_package, is_native)) {
        free(rel);
        return 1;
    }
    if (resolve_under_root(".", rel, out_path, is_package, is_native)) {
        free(rel);
        return 1;
    }
    if (i && i->stdlib_path && *i->stdlib_path && resolve_under_root(i->stdlib_path, rel, out_path, is_package, is_native)) {
        free(rel);
        return 1;
    }
    if (resolve_under_root("stdlib", rel, out_path, is_package, is_native)) {
        free(rel);
        return 1;
    }
    if (resolve_under_root("../stdlib", rel, out_path, is_package, is_native)) {
        free(rel);
        return 1;
    }
    free(rel);
    return 0;
}

static Value *load_module_absolute(Interpreter *i, const char *abs_name) {
    Value *cached = NULL;
    Value *module = NULL;
    Value *parent_module = NULL;
    Environment *module_env = NULL;
    char *parent_name = NULL;
    char *package_name = NULL;
    char *path = NULL;
    char *source = NULL;
    char *old_file = NULL;
    char *old_module_name = NULL;
    char *old_package = NULL;
    Lexer *lx = NULL;
    Token **tokens = NULL;
    Parser *p = NULL;
    ASTNode *program = NULL;
    ExecResult r;
    int count = 0;
    int k = 0;
    int is_package = 0;
    int is_native = 0;
    int old_line = 0;
    if (!abs_name || !*abs_name) return runtime_error(i, error_import(abs_name ? abs_name : ""));
    cached = dict_get(i->module_cache, abs_name);
    if (cached) return value_copy(cached);
    parent_name = module_parent_name(abs_name);
    if (*parent_name) {
        parent_module = load_module_absolute(i, parent_name);
        if (!parent_module) { free(parent_name); return NULL; }
        if (!module_is_package(parent_module)) {
            value_free(parent_module);
            free(parent_name);
            return runtime_error(i, error_import(abs_name));
        }
    }
    if (!resolve_module_spec(i, abs_name, &path, &is_package, &is_native)) {
        value_free(parent_module);
        free(parent_name);
        return runtime_error(i, error_import(abs_name));
    }
    if (is_native) {
        char *native_error = NULL;
        module_env = env_new(i->global_env);
        module = make_module(abs_name, module_env);
        package_name = module_parent_name(abs_name);
        set_module_metadata(module_env, abs_name, path, package_name);
        env_set(module_env, "__native__", make_bool(1));
        if (!azr_native_load_module(path, module_env, &native_error)) {
            char *msg = NULL;
            size_t len = strlen(abs_name) + (native_error ? strlen(native_error) : 0) + 64;
            msg = (char *)malloc(len);
            snprintf(msg, len, "واردکردن ماژول native ناموفق بود: %s: %s", abs_name, native_error ? native_error : "");
            free(native_error);
            value_free(module);
            value_free(parent_module);
            free(parent_name);
            free(package_name);
            free(path);
            return runtime_error(i, msg);
        }
        dict_set(i->module_cache, abs_name, value_copy(module));
        if (parent_module) {
            char *leaf = module_leaf_name(abs_name);
            env_set(parent_module->module.env, leaf, value_copy(module));
            free(leaf);
        }
        value_free(parent_module);
        free(parent_name);
        free(package_name);
        free(path);
        return module;
    }
    source = read_text_file(path);
    if (!source) {
        value_free(parent_module);
        free(parent_name);
        free(path);
        return runtime_error(i, error_file_not_found(abs_name));
    }
    lx = lexer_new(source);
    tokens = lexer_tokenize_all(lx, &count);
    p = parser_new(tokens, count);
    program = parse_program(p);
    if (p->error) {
        char *msg = azr_strdup(p->error);
        parser_free(p);
        for (k = 0; k < count; k++) token_free(tokens[k]);
        free(tokens);
        lexer_free(lx);
        free(source);
        free(path);
        value_free(parent_module);
        free(parent_name);
        ast_free(program);
        return runtime_error(i, msg);
    }
    module_env = env_new(i->global_env);
    module = make_module(abs_name, module_env);
    package_name = is_package ? azr_strdup(abs_name) : module_parent_name(abs_name);
    set_module_metadata(module_env, abs_name, path, package_name);
    dict_set(i->module_cache, abs_name, value_copy(module));
    old_file = azr_strdup(i->current_file ? i->current_file : "<ورودی>");
    old_module_name = azr_strdup(i->module_name ? i->module_name : "");
    old_package = azr_strdup(i->module_package ? i->module_package : "");
    old_line = i->current_line;
    interpreter_set_current_file(i, path);
    free(i->module_name);
    i->module_name = azr_strdup(abs_name);
    free(i->module_package);
    i->module_package = azr_strdup(package_name ? package_name : "");
    i->current_line = 0;
    r = exec_block(i, program, module_env);
    interpreter_set_current_file(i, old_file);
    free(i->module_name);
    i->module_name = old_module_name;
    old_module_name = NULL;
    free(i->module_package);
    i->module_package = old_package;
    old_package = NULL;
    i->current_line = old_line;
    free(old_file);
    old_file = NULL;
    if (r.signal == SIG_ERROR || i->had_error) {
        value_free(r.value);
        dict_delete(i->module_cache, abs_name);
        value_free(module);
        parser_free(p);
        for (k = 0; k < count; k++) token_free(tokens[k]);
        free(tokens);
        lexer_free(lx);
        free(source);
        free(path);
        value_free(parent_module);
        free(parent_name);
        free(package_name);
        ast_free(program);
        return NULL;
    }
    value_free(r.value);
    if (parent_module) {
        char *leaf = module_leaf_name(abs_name);
        env_set(parent_module->module.env, leaf, value_copy(module));
        free(leaf);
    }
    parser_free(p);
    for (k = 0; k < count; k++) token_free(tokens[k]);
    free(tokens);
    lexer_free(lx);
    free(source);
    free(path);
    value_free(parent_module);
    free(parent_name);
    free(package_name);
    return module;
}

/* Returns true if a value is an int or float. */
