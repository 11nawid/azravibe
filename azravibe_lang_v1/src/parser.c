#include "parser.h"
#include "error.h"
#include "utf8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Creates a parser over a token array. */
Parser *parser_new(Token **tokens, int count) {
    Parser *p = (Parser *)calloc(1, sizeof(Parser));
    p->tokens = tokens;
    p->count = count;
    return p;
}

/* Releases parser metadata without freeing caller-owned tokens. */
void parser_free(Parser *p) {
    if (!p) return;
    free(p->error);
    free(p);
}

/* Returns the current token without advancing. */
static Token *peek(Parser *p) { return p->tokens[p->pos]; }

/* Returns the previous token. */
static Token *prev(Parser *p) { return p->tokens[p->pos - 1]; }

/* Returns true when the parser has reached EOF. */
static int at_end(Parser *p) { return peek(p)->type == TOK_EOF; }

/* Tests the current token type. */
static int check(Parser *p, TokenType t) { return !at_end(p) && peek(p)->type == t; }

/* Advances and returns the consumed token. */
static Token *advance(Parser *p) {
    if (!at_end(p)) p->pos++;
    return prev(p);
}

/* Consumes a token when it matches the requested type. */
static int match(Parser *p, TokenType t) {
    if (check(p, t)) { advance(p); return 1; }
    return 0;
}

/* Records a parser error in natural Farsi if one is not already present. */
static void set_error(Parser *p, const char *detail) {
    if (!p->error) p->error = error_syntax(peek(p)->line, detail);
}

/* Requires a token type and records an error if it is absent. */
static Token *consume(Parser *p, TokenType t, const char *msg) {
    if (check(p, t)) return advance(p);
    set_error(p, msg);
    return peek(p);
}

static int is_assignment_op(Parser *p) {
    return check(p, TOK_ASSIGN) || check(p, TOK_PLUS_ASSIGN) || check(p, TOK_MINUS_ASSIGN) || check(p, TOK_STAR_ASSIGN) || check(p, TOK_SLASH_ASSIGN);
}

static void append_ast(ASTNode ***items, int *count, ASTNode *node) {
    *items = (ASTNode **)realloc(*items, (size_t)(*count + 1) * sizeof(ASTNode *));
    (*items)[(*count)++] = node;
}

static void parse_param_list(Parser *p, char ***params, ASTNode ***defaults, int *count, char **vararg) {
    *params = NULL; *defaults = NULL; *count = 0; *vararg = NULL;
    if (!check(p, TOK_RPAREN)) {
        do {
            ASTNode *def = NULL;
            Token *param = NULL;
            if (match(p, TOK_STAR)) {
                param = consume(p, TOK_IDENTIFIER, "بعد از * اسم پارامتر لازم داریم");
                *vararg = azr_strdup(param->value);
                continue;
            }
            param = consume(p, TOK_IDENTIFIER, "اسم پارامتر نامعتبره");
            if (match(p, TOK_ASSIGN)) def = parse_expression(p);
            *params = (char **)realloc(*params, (size_t)(*count + 1) * sizeof(char *));
            *defaults = (ASTNode **)realloc(*defaults, (size_t)(*count + 1) * sizeof(ASTNode *));
            (*params)[*count] = azr_strdup(param->value);
            (*defaults)[*count] = def;
            (*count)++;
        } while (match(p, TOK_COMMA));
    }
}

/* Skips blank logical lines. */
static void skip_newlines(Parser *p) {
    while (match(p, TOK_NEWLINE)) {}
}

/* Parses a full program until EOF. */
ASTNode *parse_program(Parser *p) {
    ASTNode *program = ast_make_block(NODE_PROGRAM, 1);
    skip_newlines(p);
    while (!at_end(p)) {
        ASTNode *stmt = parse_statement(p);
        if (stmt) ast_block_add(program, stmt);
        skip_newlines(p);
        if (p->error) break;
    }
    return program;
}

/* Parses one statement based on its leading token. */
ASTNode *parse_statement(Parser *p) {
    skip_newlines(p);
    if (match(p, TOK_AGE)) return parse_if(p);
    if (match(p, TOK_TA_VAGTI)) return parse_while(p);
    if (match(p, TOK_BARAY_HAR)) return parse_for(p);
    if (match(p, TOK_KAR)) return parse_function_def(p);
    if (match(p, TOK_NOE)) return parse_class_def(p);
    if (match(p, TOK_BA)) return parse_with(p);
    if (match(p, TOK_BIAND)) return parse_throw(p);
    if (match(p, TOK_BEDE) || match(p, TOK_BARGASHT)) return parse_return(p);
    if (match(p, TOK_YIELD)) {
        int line = prev(p)->line;
        if (check(p, TOK_NEWLINE) || check(p, TOK_DEDENT) || check(p, TOK_EOF)) return ast_make_yield(NULL, line);
        return ast_make_yield(parse_expression(p), line);
    }
    if (match(p, TOK_BIYAR)) return parse_import(p);
    if (match(p, TOK_SAY_KON)) return parse_try_except(p);
    if (match(p, TOK_BEZAN_BIRON)) return ast_make_simple(NODE_BREAK, prev(p)->line);
    if (match(p, TOK_EDAME)) return ast_make_simple(NODE_CONTINUE, prev(p)->line);
    if (match(p, TOK_HICHICHI)) return ast_make_simple(NODE_PASS, prev(p)->line);
    if (match(p, TOK_AZ)) return parse_import(p);
    if (check(p, TOK_IDENTIFIER) && p->pos + 1 < p->count && p->tokens[p->pos + 1]->type == TOK_COMMA) {
        ASTNode **targets = NULL;
        int count = 0, line = peek(p)->line;
        do {
            Token *name = consume(p, TOK_IDENTIFIER, "در انتساب چندتایی اسم متغیر لازم داریم");
            append_ast(&targets, &count, ast_make_identifier(name->value, name->line));
        } while (match(p, TOK_COMMA));
        consume(p, TOK_ASSIGN, "انتساب چندتایی باید '=' داشته باشه");
        return ast_make_multi_assign(targets, count, parse_assignment(p), line);
    }
    {
        ASTNode *expr = parse_assignment(p);
        if (expr && (expr->type == NODE_ASSIGN || expr->type == NODE_AUGMENTED_ASSIGN || expr->type == NODE_INDEX_ASSIGN || expr->type == NODE_INDEX_AUGMENTED_ASSIGN || expr->type == NODE_MULTI_ASSIGN || expr->type == NODE_ATTR_ASSIGN)) return expr;
        return ast_make_expr_stmt(expr, expr ? expr->line : peek(p)->line);
    }
}

/* Parses an indented block after a colon. */
ASTNode *parse_block(Parser *p) {
    ASTNode *block = ast_make_block(NODE_BLOCK, peek(p)->line);
    consume(p, TOK_COLON, "بعد از دستور باید ':' بیاد");
    consume(p, TOK_NEWLINE, "بعد از ':' باید خط جدید بیاد");
    skip_newlines(p);
    consume(p, TOK_INDENT, "بلوک باید با چهار فاصله تورفته باشه");
    skip_newlines(p);
    while (!check(p, TOK_DEDENT) && !at_end(p)) {
        ast_block_add(block, parse_statement(p));
        skip_newlines(p);
        if (p->error) break;
    }
    consume(p, TOK_DEDENT, "تورفتگی بلوک درست بسته نشده");
    return block;
}

/* Parses if/elif/else using Farsi keywords. */
ASTNode *parse_if(Parser *p) {
    int line = prev(p)->line;
    ASTNode *cond = parse_expression(p);
    ASTNode *then_block = parse_block(p);
    ASTNode *node = ast_make_if(cond, then_block, line);
    while (match(p, TOK_VAGARNA_AGE)) {
        ASTNode *elif_cond = parse_expression(p);
        ASTNode *elif_block = parse_block(p);
        ast_if_add_elif(node, elif_cond, elif_block);
    }
    if (match(p, TOK_VAGARNA)) ast_if_set_else(node, parse_block(p));
    return node;
}

/* Parses a while loop. */
ASTNode *parse_while(Parser *p) {
    int line = prev(p)->line;
    ASTNode *cond = parse_expression(p);
    ASTNode *body = parse_block(p);
    return ast_make_while(cond, body, line);
}

/* Parses a for-each loop. */
ASTNode *parse_for(Parser *p) {
    int line = prev(p)->line;
    Token *name = consume(p, TOK_IDENTIFIER, "بعد از 'برای هر' اسم متغیر لازم داریم");
    consume(p, TOK_TOOY, "در حلقه باید 'توی' بیاد");
    {
        ASTNode *iterable = parse_expression(p);
        ASTNode *body = parse_block(p);
        return ast_make_for(name->value, iterable, body, line);
    }
}

/* Parses a function definition with comma-separated parameters. */
ASTNode *parse_function_def(Parser *p) {
    int line = prev(p)->line;
    char **params = NULL;
    ASTNode **defaults = NULL;
    int count = 0;
    char *vararg = NULL;
    Token *name = consume(p, TOK_IDENTIFIER, "بعد از 'کار' اسم تابع لازم داریم");
    consume(p, TOK_LPAREN, "بعد از اسم کار باید '(' بیاد");
    parse_param_list(p, &params, &defaults, &count, &vararg);
    consume(p, TOK_RPAREN, "پرانتز بسته نشده");
    { ASTNode *n = ast_make_function_def(name->value, params, defaults, count, vararg, parse_block(p), line); free(vararg); return n; }
}

/* Parses a return statement. */
ASTNode *parse_return(Parser *p) {
    int line = prev(p)->line;
    if (check(p, TOK_NEWLINE) || check(p, TOK_DEDENT) || check(p, TOK_EOF)) return ast_make_return(NULL, line);
    return ast_make_return(parse_expression(p), line);
}

static char *parse_module_path(Parser *p, int allow_relative) {
    char buffer[512];
    size_t len = 0;
    int saw_dot = 0;
    int saw_name = 0;
    buffer[0] = '\0';
    while (allow_relative && match(p, TOK_DOT)) {
        if (len + 1 >= sizeof(buffer)) {
            set_error(p, "اسم ماژول زیادی بلنده");
            return azr_strdup("");
        }
        buffer[len++] = '.';
        buffer[len] = '\0';
        saw_dot = 1;
    }
    if (check(p, TOK_IDENTIFIER)) {
        Token *name = advance(p);
        size_t part_len = strlen(name->value);
        if (len + part_len >= sizeof(buffer)) {
            set_error(p, "اسم ماژول زیادی بلنده");
            return azr_strdup("");
        }
        memcpy(buffer + len, name->value, part_len + 1);
        len += part_len;
        saw_name = 1;
        while (match(p, TOK_DOT)) {
            Token *part = NULL;
            if (len + 1 >= sizeof(buffer)) {
                set_error(p, "اسم ماژول زیادی بلنده");
                return azr_strdup("");
            }
            buffer[len++] = '.';
            buffer[len] = '\0';
            part = consume(p, TOK_IDENTIFIER, "بعد از نقطه اسم ماژول لازم داریم");
            part_len = strlen(part->value);
            if (len + part_len >= sizeof(buffer)) {
                set_error(p, "اسم ماژول زیادی بلنده");
                return azr_strdup("");
            }
            memcpy(buffer + len, part->value, part_len + 1);
            len += part_len;
        }
    }
    if (!saw_name && !saw_dot) {
        set_error(p, allow_relative ? "بعد از دستور واردکردن اسم ماژول لازم داریم" : "اسم ماژول نامعتبره");
        return azr_strdup("");
    }
    if (!saw_name && !allow_relative) {
        set_error(p, "اسم ماژول نامعتبره");
        return azr_strdup("");
    }
    return azr_strdup(buffer);
}

/* Parses an import statement with optional alias. */
ASTNode *parse_import(Parser *p) {
    int line = prev(p)->line;
    if (prev(p)->type == TOK_AZ) {
        char *module = parse_module_path(p, 1);
        char **names = NULL, **aliases = NULL;
        int count = 0;
        consume(p, TOK_BIYAR, "بعد از اسم ماژول باید 'بیار' بیاد");
        do {
            Token *name = consume(p, TOK_IDENTIFIER, "اسم واردشده نامعتبره");
            char *alias = NULL;
            if (match(p, TOK_BE_ONVAN)) alias = azr_strdup(consume(p, TOK_IDENTIFIER, "بعد از 'به عنوان' اسم لازم داریم")->value);
            names = (char **)realloc(names, (size_t)(count + 1) * sizeof(char *));
            aliases = (char **)realloc(aliases, (size_t)(count + 1) * sizeof(char *));
            names[count] = azr_strdup(name->value);
            aliases[count] = alias;
            count++;
        } while (match(p, TOK_COMMA));
        { ASTNode *n = ast_make_from_import(module, names, aliases, count, line); free(module); return n; }
    }
    char *module = parse_module_path(p, 1);
    char *alias = NULL;
    if (match(p, TOK_BE_ONVAN)) alias = consume(p, TOK_IDENTIFIER, "بعد از 'به عنوان' اسم لازم داریم")->value;
    { ASTNode *n = ast_make_import(module, alias, line); free(module); return n; }
}

/* Parses a try/except/finally statement. */
ASTNode *parse_try_except(Parser *p) {
    int line = prev(p)->line;
    ASTNode *try_block = parse_block(p);
    ASTNode *finally_block = NULL;
    ASTNode *node = ast_make_try_except(try_block, NULL, line);
    while (match(p, TOK_AGE_KHATA)) {
        ASTNode *exception_type = NULL;
        ASTNode *except_block = NULL;
        char *name = NULL;
        if (!check(p, TOK_BE_ONVAN) && !check(p, TOK_COLON)) exception_type = parse_expression(p);
        if (match(p, TOK_BE_ONVAN)) name = azr_strdup(consume(p, TOK_IDENTIFIER, "بعد از 'به عنوان' اسم خطا لازم داریم")->value);
        except_block = parse_block(p);
        ast_try_add_except(node, exception_type, name, except_block);
        free(name);
    }
    if (match(p, TOK_BALAKHARE)) {
        finally_block = parse_block(p);
        node->try_except.finally_block = finally_block;
    }
    if (node->try_except.except_count == 0 && !finally_block) set_error(p, "بعد از 'سعی کن' باید 'اگه خطا' یا 'بالاخره' بیاد");
    return node;
}

ASTNode *parse_class_def(Parser *p) {
    int line = prev(p)->line;
    Token *name = consume(p, TOK_IDENTIFIER, "بعد از 'نوع' اسم کلاس لازم داریم");
    ASTNode *base_class = NULL;
    if (match(p, TOK_LPAREN)) {
        base_class = parse_expression(p);
        consume(p, TOK_RPAREN, "پرانتز بسته نشده");
    }
    return ast_make_class_def(name->value, base_class, parse_block(p), line);
}

ASTNode *parse_with(Parser *p) {
    int line = prev(p)->line;
    ASTNode *expr = parse_expression(p);
    char *alias = NULL;
    if (match(p, TOK_BE_ONVAN)) alias = azr_strdup(consume(p, TOK_IDENTIFIER, "بعد از 'به عنوان' اسم لازم داریم")->value);
    { ASTNode *n = ast_make_with(expr, alias, parse_block(p), line); free(alias); return n; }
}

ASTNode *parse_throw(Parser *p) {
    return ast_make_throw(parse_expression(p), prev(p)->line);
}

static ASTNode *parse_expression_no_ternary(Parser *p) { return parse_or(p); }

/* Parses a full expression. */
ASTNode *parse_expression(Parser *p) { return parse_ternary(p); }

ASTNode *parse_ternary(Parser *p) {
    ASTNode *expr = parse_or(p);
    if (match(p, TOK_AGE)) {
        ASTNode *cond = parse_or(p);
        consume(p, TOK_VAGARNA, "در عبارت شرطی باید 'وگرنه' بیاد");
        expr = ast_make_ternary(expr, cond, parse_expression(p), expr->line);
    } else if (match(p, TOK_QUESTION)) {
        ASTNode *then_expr = parse_expression(p);
        consume(p, TOK_COLON, "در عبارت شرطی باید ':' بیاد");
        expr = ast_make_ternary(then_expr, expr, parse_expression(p), expr->line);
    }
    return expr;
}

/* Parses assignment and augmented assignment expressions. */
ASTNode *parse_assignment(Parser *p) {
    ASTNode *left = parse_expression(p);
    if (left && match(p, TOK_COMMA)) {
        ASTNode **targets = NULL;
        int count = 0;
        append_ast(&targets, &count, left);
        do {
            append_ast(&targets, &count, parse_expression(p));
        } while (match(p, TOK_COMMA));
        left = ast_make_tuple(targets, count, left->line);
    }
    if (left && left->type == NODE_TUPLE_LITERAL && is_assignment_op(p)) {
        Token *op = advance(p);
        if (op->type != TOK_ASSIGN) set_error(p, "انتساب چندتایی فقط با '=' پشتیبانی میشه");
        return ast_make_multi_assign(left->tuple_lit.items, left->tuple_lit.count, parse_assignment(p), left->line);
    }
    if (left && (left->type == NODE_IDENTIFIER || left->type == NODE_INDEX || left->type == NODE_ATTRIBUTE) && is_assignment_op(p)) {
        Token *op = advance(p);
        ASTNode *value = parse_assignment(p);
        int line = left->line;
        if (left->type == NODE_IDENTIFIER) {
            char *name = azr_strdup(left->identifier.name);
            ast_free(left);
            if (op->type == TOK_ASSIGN) {
                ASTNode *n = ast_make_assign(name, value, line);
                free(name);
                return n;
            }
            {
                ASTNode *n = ast_make_aug_assign(name, op->value, value, line);
                free(name);
                return n;
            }
        } else if (left->type == NODE_ATTRIBUTE) {
            ASTNode *object = left->attribute.object;
            char *attr = azr_strdup(left->attribute.attr);
            left->attribute.object = NULL;
            ast_free(left);
            if (op->type != TOK_ASSIGN) { free(attr); set_error(p, "ویژگی فعلا فقط با '=' تغییر می‌کنه"); return value; }
            { ASTNode *n = ast_make_attr_assign(object, attr, value, line); free(attr); return n; }
        } else {
            ASTNode *object = left->index.object;
            ASTNode *index = left->index.index;
            left->index.object = NULL;
            left->index.index = NULL;
            ast_free(left);
            if (op->type == TOK_ASSIGN) return ast_make_index_assign(object, index, value, line);
            {
                ASTNode *n = ast_make_index_aug_assign(object, index, op->value, value, line);
                return n;
            }
        }
    }
    return left;
}

/* Parses logical or expressions. */
ASTNode *parse_or(Parser *p) {
    ASTNode *expr = parse_and(p);
    while (match(p, TOK_YA)) expr = ast_make_binary("یا", expr, parse_and(p), prev(p)->line);
    return expr;
}

/* Parses logical and expressions. */
ASTNode *parse_and(Parser *p) {
    ASTNode *expr = parse_not(p);
    while (match(p, TOK_VA)) expr = ast_make_binary("و", expr, parse_not(p), prev(p)->line);
    return expr;
}

/* Parses logical not expressions. */
ASTNode *parse_not(Parser *p) {
    if (match(p, TOK_NA)) return ast_make_unary("نه", parse_not(p), prev(p)->line);
    return parse_comparison(p);
}

/* Parses comparison expressions. */
ASTNode *parse_comparison(Parser *p) {
    ASTNode *expr = parse_addition(p);
    while (check(p, TOK_EQ) || check(p, TOK_NEQ) || check(p, TOK_GT) || check(p, TOK_LT) || check(p, TOK_GTE) || check(p, TOK_LTE)) {
        Token *op = advance(p);
        expr = ast_make_binary(op->value, expr, parse_addition(p), op->line);
    }
    return expr;
}

/* Parses addition and subtraction. */
ASTNode *parse_addition(Parser *p) {
    ASTNode *expr = parse_multiplication(p);
    while (check(p, TOK_PLUS) || check(p, TOK_MINUS)) {
        Token *op = advance(p);
        expr = ast_make_binary(op->value, expr, parse_multiplication(p), op->line);
    }
    return expr;
}

/* Parses multiplication, division, floor division, and modulo. */
ASTNode *parse_multiplication(Parser *p) {
    ASTNode *expr = parse_power(p);
    while (check(p, TOK_STAR) || check(p, TOK_SLASH) || check(p, TOK_DOUBLESLASH) || check(p, TOK_PERCENT)) {
        Token *op = advance(p);
        expr = ast_make_binary(op->value, expr, parse_power(p), op->line);
    }
    return expr;
}

/* Parses right-associative exponentiation. */
ASTNode *parse_power(Parser *p) {
    ASTNode *expr = parse_unary(p);
    if (match(p, TOK_DOUBLESTAR)) expr = ast_make_binary("**", expr, parse_power(p), prev(p)->line);
    return expr;
}

/* Parses unary minus and not. */
ASTNode *parse_unary(Parser *p) {
    if (match(p, TOK_MINUS)) return ast_make_unary("-", parse_unary(p), prev(p)->line);
    if (match(p, TOK_NA)) return ast_make_unary("نه", parse_unary(p), prev(p)->line);
    return parse_call_or_index(p);
}

/* Parses calls, list indexing, and attribute access chains. */
ASTNode *parse_call_or_index(Parser *p) {
    ASTNode *expr = parse_primary(p);
    while (1) {
        if (match(p, TOK_LPAREN)) {
            ASTNode **args = NULL;
            char **arg_names = NULL;
            int count = 0;
            if (!check(p, TOK_RPAREN)) {
                do {
                    char *arg_name = NULL;
                    if (check(p, TOK_IDENTIFIER) && p->pos + 1 < p->count && p->tokens[p->pos + 1]->type == TOK_ASSIGN) {
                        arg_name = azr_strdup(peek(p)->value);
                        advance(p); advance(p);
                    }
                    args = (ASTNode **)realloc(args, (size_t)(count + 1) * sizeof(ASTNode *));
                    arg_names = (char **)realloc(arg_names, (size_t)(count + 1) * sizeof(char *));
                    arg_names[count] = arg_name;
                    args[count++] = parse_expression(p);
                } while (match(p, TOK_COMMA));
            }
            consume(p, TOK_RPAREN, "پرانتز بسته نشده");
            expr = ast_make_call(expr, args, arg_names, count, expr->line);
        } else if (match(p, TOK_LBRACKET)) {
            ASTNode *idx = NULL;
            ASTNode *end = NULL;
            if (!check(p, TOK_COLON)) idx = parse_expression(p);
            if (match(p, TOK_COLON)) {
                if (!check(p, TOK_RBRACKET)) end = parse_expression(p);
                consume(p, TOK_RBRACKET, "براکت بسته نشده");
                expr = ast_make_slice(expr, idx, end, expr->line);
                continue;
            }
            consume(p, TOK_RBRACKET, "براکت بسته نشده");
            expr = ast_make_index(expr, idx, expr->line);
        } else if (match(p, TOK_DOT)) {
            Token *name = consume(p, TOK_IDENTIFIER, "بعد از نقطه اسم ویژگی لازم داریم");
            expr = ast_make_attribute(expr, name->value, expr->line);
        } else {
            break;
        }
    }
    return expr;
}

static ASTNode *parse_comprehension_tail(Parser *p, NodeType type, ASTNode *item_expr, ASTNode *value_expr, int line) {
    char **var_names = NULL;
    ASTNode **iterables = NULL;
    ASTNode **conditions = NULL;
    int loop_count = 0;
    while (1) {
        Token *name = NULL;
        ASTNode *condition = NULL;
        var_names = (char **)realloc(var_names, (size_t)(loop_count + 1) * sizeof(char *));
        iterables = (ASTNode **)realloc(iterables, (size_t)(loop_count + 1) * sizeof(ASTNode *));
        conditions = (ASTNode **)realloc(conditions, (size_t)(loop_count + 1) * sizeof(ASTNode *));
        name = consume(p, TOK_IDENTIFIER, "بعد از 'برای هر' اسم متغیر لازم داریم");
        consume(p, TOK_TOOY, "در comprehension باید 'توی' بیاد");
        var_names[loop_count] = azr_strdup(name->value);
        iterables[loop_count] = parse_expression_no_ternary(p);
        if (match(p, TOK_AGE)) condition = parse_expression_no_ternary(p);
        conditions[loop_count] = condition;
        loop_count++;
        if (!match(p, TOK_BARAY_HAR)) break;
    }
    return ast_make_comprehension(type, item_expr, value_expr, var_names, iterables, conditions, loop_count, line);
}

/* Parses primitive literals, identifiers, parenthesized expressions, and list literals. */
ASTNode *parse_primary(Parser *p) {
    if (match(p, TOK_INT)) return ast_make_int(strtoll(prev(p)->value, NULL, 10), prev(p)->line);
    if (match(p, TOK_FLOAT)) return ast_make_float(strtod(prev(p)->value, NULL), prev(p)->line);
    if (match(p, TOK_STRING)) return ast_make_string(prev(p)->value, prev(p)->line);
    if (match(p, TOK_DERASTE)) return ast_make_bool(1, prev(p)->line);
    if (match(p, TOK_GHALATE)) return ast_make_bool(0, prev(p)->line);
    if (match(p, TOK_HICHI)) return ast_make_none(prev(p)->line);
    if (match(p, TOK_BENEVIS)) return ast_make_identifier("بنویس", prev(p)->line);
    if (match(p, TOK_BEGIR)) return ast_make_identifier("بگیر", prev(p)->line);
    if (match(p, TOK_IDENTIFIER)) return ast_make_identifier(prev(p)->value, prev(p)->line);
    if (match(p, TOK_LPAREN)) {
        ASTNode **items = NULL;
        int count = 0, line = prev(p)->line;
        ASTNode *expr = parse_expression(p);
        append_ast(&items, &count, expr);
        while (match(p, TOK_COMMA)) {
            if (check(p, TOK_RPAREN)) break;
            append_ast(&items, &count, parse_expression(p));
        }
        consume(p, TOK_RPAREN, "پرانتز بسته نشده");
        if (count == 1) { ASTNode *only = items[0]; free(items); return only; }
        return ast_make_tuple(items, count, line);
    }
    if (match(p, TOK_LBRACKET)) return parse_list_literal(p);
    if (match(p, TOK_LBRACE)) return parse_dict_or_set_literal(p);
    if (match(p, TOK_LAMBDA)) {
        char **params = NULL; ASTNode **defaults = NULL; int count = 0; char *vararg = NULL;
        int line = prev(p)->line;
        consume(p, TOK_LPAREN, "بعد از لامبدا باید '(' بیاد");
        parse_param_list(p, &params, &defaults, &count, &vararg);
        consume(p, TOK_RPAREN, "پرانتز بسته نشده");
        consume(p, TOK_COLON, "بعد از پارامترهای لامبدا باید ':' بیاد");
        { ASTNode *n = ast_make_lambda(params, defaults, count, vararg, parse_expression(p), line); free(vararg); return n; }
    }
    set_error(p, "عبارت نامعتبره");
    return ast_make_none(peek(p)->line);
}

ASTNode *parse_dict_or_set_literal(Parser *p) {
    ASTNode **keys = NULL, **values = NULL;
    int count = 0, is_set = 0, line = prev(p)->line;
    if (!check(p, TOK_RBRACE)) {
        ASTNode *first = parse_expression(p);
        ASTNode *val = NULL;
        if (match(p, TOK_COLON)) {
            val = parse_expression(p);
            if (match(p, TOK_BARAY_HAR)) {
                ASTNode *comp = parse_comprehension_tail(p, NODE_DICT_COMPREHENSION, first, val, line);
                consume(p, TOK_RBRACE, "آکولاد بسته نشده");
                return comp;
            }
        } else {
            is_set = 1;
            if (match(p, TOK_BARAY_HAR)) {
                ASTNode *comp = parse_comprehension_tail(p, NODE_SET_COMPREHENSION, first, NULL, line);
                consume(p, TOK_RBRACE, "آکولاد بسته نشده");
                return comp;
            }
        }
        keys = (ASTNode **)realloc(keys, (size_t)(count + 1) * sizeof(ASTNode *));
        values = (ASTNode **)realloc(values, (size_t)(count + 1) * sizeof(ASTNode *));
        keys[count] = first;
        values[count] = val;
        count++;
        while (match(p, TOK_COMMA)) {
            first = parse_expression(p);
            val = NULL;
            if (match(p, TOK_COLON)) val = parse_expression(p);
            else is_set = 1;
            keys = (ASTNode **)realloc(keys, (size_t)(count + 1) * sizeof(ASTNode *));
            values = (ASTNode **)realloc(values, (size_t)(count + 1) * sizeof(ASTNode *));
            keys[count] = first;
            values[count] = val;
            count++;
        }
    }
    consume(p, TOK_RBRACE, "آکولاد بسته نشده");
    return ast_make_dict(keys, values, count, is_set, line);
}

/* Parses a list literal after the opening bracket has already been consumed. */
ASTNode *parse_list_literal(Parser *p) {
    ASTNode **items = NULL;
    int count = 0;
    int line = prev(p)->line;
    if (!check(p, TOK_RBRACKET)) {
        ASTNode *first = parse_expression(p);
        if (match(p, TOK_BARAY_HAR)) {
            ASTNode *comp = parse_comprehension_tail(p, NODE_LIST_COMPREHENSION, first, NULL, line);
            consume(p, TOK_RBRACKET, "براکت بسته نشده");
            return comp;
        }
        items = (ASTNode **)realloc(items, (size_t)(count + 1) * sizeof(ASTNode *));
        items[count++] = first;
        while (match(p, TOK_COMMA)) {
            items = (ASTNode **)realloc(items, (size_t)(count + 1) * sizeof(ASTNode *));
            items[count++] = parse_expression(p);
        }
    }
    consume(p, TOK_RBRACKET, "براکت بسته نشده");
    return ast_make_list(items, count, line);
}
