#include "lexer.h"
#include "utf8.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Creates a heap token with copied raw token text and source location. */
static Token *make_token(TokenType type, const char *value, int line, int col) {
    Token *t = (Token *)calloc(1, sizeof(Token));
    t->type = type;
    t->value = azr_strdup(value ? value : "");
    t->line = line;
    t->col = col;
    return t;
}

/* Queues a token for delayed emission, used by indentation handling. */
static void push_pending(Lexer *l, Token *t) {
    if (l->pending_count >= l->pending_cap) {
        l->pending_cap = l->pending_cap ? l->pending_cap * 2 : 8;
        l->pending = (Token **)realloc(l->pending, (size_t)l->pending_cap * sizeof(Token *));
    }
    l->pending[l->pending_count++] = t;
}

/* Pops the oldest pending token from the lexer queue. */
static Token *pop_pending(Lexer *l) {
    Token *t = NULL;
    int i = 0;
    if (l->pending_count == 0) return NULL;
    t = l->pending[0];
    for (i = 1; i < l->pending_count; i++) l->pending[i - 1] = l->pending[i];
    l->pending_count--;
    return t;
}

/* Creates a delimiter token and updates grouping depth for multiline expressions. */
static Token *delimiter_token(Lexer *l, TokenType type, const char *value, int line, int col) {
    if (type == TOK_LPAREN || type == TOK_LBRACKET || type == TOK_LBRACE) l->nesting++;
    if ((type == TOK_RPAREN || type == TOK_RBRACKET || type == TOK_RBRACE) && l->nesting > 0) l->nesting--;
    return make_token(type, value, line, col);
}

/* Returns true when the source position is at end of input. */
static int is_at_end(Lexer *l) {
    return l->source[l->pos] == '\0';
}

static void advance_cp(Lexer *l);

/* Returns true if the lexer is positioned at a triple-quoted block comment. */
static int at_triple_comment(Lexer *l) {
    return strncmp(l->source + l->pos, "\"\"\"", 3) == 0;
}

/* Skips a # line comment while preserving the newline for normal statement ending. */
static void skip_line_comment(Lexer *l) {
    while (!is_at_end(l) && l->source[l->pos] != '\n') {
        l->pos++;
        l->col++;
    }
}

/* Skips a triple-quoted multiline comment and updates line/column counters. */
static void skip_triple_comment(Lexer *l) {
    l->pos += 3;
    l->col += 3;
    while (!is_at_end(l)) {
        if (at_triple_comment(l)) {
            l->pos += 3;
            l->col += 3;
            return;
        }
        if (l->source[l->pos] == '\r') {
            l->pos++;
            continue;
        }
        if (l->source[l->pos] == '\n') {
            l->pos++;
            l->line++;
            l->col = 1;
            l->at_line_start = 1;
            continue;
        }
        advance_cp(l);
    }
}

/* Advances one UTF-8 codepoint and updates codepoint column. */
static void advance_cp(Lexer *l) {
    int len = utf8_charlen((unsigned char)l->source[l->pos]);
    l->pos += len;
    l->col++;
}

/* Returns true for bytes that can start or continue an identifier. */
static int is_ident_cp(uint32_t cp) {
    if (cp == 0x060C) return 0;
    if (cp == 0x200C || cp == 0x200D) return 1;
    return utf8_is_farsi(cp) || cp == '_' || (cp >= 'A' && cp <= 'Z') || (cp >= 'a' && cp <= 'z') || (cp >= '0' && cp <= '9');
}

/* Maps natural Farsi words and phrases to keyword token types. */
static TokenType keyword_type(const char *word) {
    if (utf8_match(word, "اگه")) return TOK_AGE;
    if (utf8_match(word, "وگرنه")) return TOK_VAGARNA;
    if (utf8_match(word, "تا")) return TOK_TA;
    if (utf8_match(word, "وقتی")) return TOK_IDENTIFIER;
    if (utf8_match(word, "برای")) return TOK_IDENTIFIER;
    if (utf8_match(word, "هر")) return TOK_IDENTIFIER;
    if (utf8_match(word, "توی")) return TOK_TOOY;
    if (utf8_match(word, "کار")) return TOK_KAR;
    if (utf8_match(word, "بده")) return TOK_BEDE;
    if (utf8_match(word, "برگشت")) return TOK_BARGASHT;
    if (utf8_match(word, "بینداز")) return TOK_BIAND;
    if (utf8_match(word, "لامبدا")) return TOK_LAMBDA;
    if (utf8_match(word, "تحویل") || strcmp(word, "yield") == 0) return TOK_YIELD;
    if (utf8_match(word, "بنویس")) return TOK_BENEVIS;
    if (utf8_match(word, "بگیر")) return TOK_BEGIR;
    if (utf8_match(word, "بیار")) return TOK_BIYAR;
    if (utf8_match(word, "درسته")) return TOK_DERASTE;
    if (utf8_match(word, "غلطه")) return TOK_GHALATE;
    if (utf8_match(word, "هیچ")) return TOK_HICHI;
    if (utf8_match(word, "و")) return TOK_VA;
    if (utf8_match(word, "یا")) return TOK_YA;
    if (utf8_match(word, "نه")) return TOK_NA;
    if (utf8_match(word, "بزن")) return TOK_IDENTIFIER;
    if (utf8_match(word, "بیرون")) return TOK_IDENTIFIER;
    if (utf8_match(word, "ادامه")) return TOK_EDAME;
    if (utf8_match(word, "هیچی")) return TOK_HICHICHI;
    if (utf8_match(word, "نوع")) return TOK_NOE;
    if (utf8_match(word, "سعی")) return TOK_IDENTIFIER;
    if (utf8_match(word, "کن")) return TOK_IDENTIFIER;
    if (utf8_match(word, "خطا")) return TOK_IDENTIFIER;
    if (utf8_match(word, "بالاخره")) return TOK_BALAKHARE;
    if (utf8_match(word, "با")) return TOK_BA;
    if (utf8_match(word, "به")) return TOK_IDENTIFIER;
    if (utf8_match(word, "عنوان")) return TOK_IDENTIFIER;
    if (utf8_match(word, "از")) return TOK_AZ;
    return TOK_IDENTIFIER;
}

/* Creates a lexer over a UTF-8 source buffer. */
Lexer *lexer_new(const char *source) {
    Lexer *l = (Lexer *)calloc(1, sizeof(Lexer));
    l->source = source ? source : "";
    if ((unsigned char)l->source[0] == 0xEF && (unsigned char)l->source[1] == 0xBB && (unsigned char)l->source[2] == 0xBF) l->pos = 3;
    l->line = 1;
    l->col = 1;
    l->at_line_start = 1;
    l->indents[0] = 0;
    l->indent_top = 0;
    return l;
}

/* Processes Python-style leading indentation at the start of a logical line. */
static void handle_indent(Lexer *l) {
    int spaces = 0;
    int saw_space = 0, saw_tab = 0;
    while (l->source[l->pos] == ' ' || l->source[l->pos] == '\t') {
        if (l->source[l->pos] == ' ') { spaces++; saw_space = 1; }
        else { spaces += 4; saw_tab = 1; }
        l->pos++;
        l->col++;
    }
    if (l->source[l->pos] == '\n' || l->source[l->pos] == '\r' || l->source[l->pos] == '#' || at_triple_comment(l) || l->source[l->pos] == '\0') return;
    if (saw_space && saw_tab) push_pending(l, make_token(TOK_NEWLINE, "خطا: فاصله و تب قاطی شده", l->line, l->col));
    if (spaces > l->indents[l->indent_top]) {
        l->indents[++l->indent_top] = spaces;
        push_pending(l, make_token(TOK_INDENT, "", l->line, l->col));
    } else {
        while (spaces < l->indents[l->indent_top]) {
            l->indent_top--;
            push_pending(l, make_token(TOK_DEDENT, "", l->line, l->col));
        }
    }
    l->at_line_start = 0;
}

/* Peeks ahead to combine multi-word Farsi keywords such as وگرنه اگه. */
static Token *combine_keyword_if_needed(Lexer *l, Token *first) {
    int save_pos = l->pos, save_col = l->col;
    while (l->source[l->pos] == ' ' || l->source[l->pos] == '\t') { l->pos++; l->col++; }
    if (first->type == TOK_VAGARNA && strncmp(l->source + l->pos, "اگه", strlen("اگه")) == 0) {
        l->pos += (int)strlen("اگه");
        l->col += 3;
        token_free(first);
        return make_token(TOK_VAGARNA_AGE, "وگرنه اگه", l->line, save_col);
    }
    if (strcmp(first->value, "تا") == 0 && strncmp(l->source + l->pos, "وقتی", strlen("وقتی")) == 0) {
        l->pos += (int)strlen("وقتی");
        l->col += 4;
        token_free(first);
        return make_token(TOK_TA_VAGTI, "تا وقتی", l->line, save_col);
    }
    if (strcmp(first->value, "برای") == 0 && strncmp(l->source + l->pos, "هر", strlen("هر")) == 0) {
        l->pos += (int)strlen("هر");
        l->col += 2;
        token_free(first);
        return make_token(TOK_BARAY_HAR, "برای هر", l->line, save_col);
    }
    if (strcmp(first->value, "بزن") == 0 && strncmp(l->source + l->pos, "بیرون", strlen("بیرون")) == 0) {
        l->pos += (int)strlen("بیرون");
        l->col += 5;
        token_free(first);
        return make_token(TOK_BEZAN_BIRON, "بزن بیرون", l->line, save_col);
    }
    if (strcmp(first->value, "سعی") == 0 && strncmp(l->source + l->pos, "کن", strlen("کن")) == 0) {
        l->pos += (int)strlen("کن");
        l->col += 2;
        token_free(first);
        return make_token(TOK_SAY_KON, "سعی کن", l->line, save_col);
    }
    if (strcmp(first->value, "اگه") == 0 && strncmp(l->source + l->pos, "خطا", strlen("خطا")) == 0) {
        l->pos += (int)strlen("خطا");
        l->col += 3;
        token_free(first);
        return make_token(TOK_AGE_KHATA, "اگه خطا", l->line, save_col);
    }
    if (strcmp(first->value, "به") == 0 && strncmp(l->source + l->pos, "عنوان", strlen("عنوان")) == 0) {
        l->pos += (int)strlen("عنوان");
        l->col += 5;
        token_free(first);
        return make_token(TOK_BE_ONVAN, "به عنوان", l->line, save_col);
    }
    l->pos = save_pos;
    l->col = save_col;
    return first;
}

/* Lexes one number literal token from the current byte position. */
static Token *lex_number(Lexer *l) {
    int start = l->pos, col = l->col;
    int is_float = 0;
    while (isdigit((unsigned char)l->source[l->pos])) { l->pos++; l->col++; }
    if (l->source[l->pos] == '.' && isdigit((unsigned char)l->source[l->pos + 1])) {
        is_float = 1;
        l->pos++; l->col++;
        while (isdigit((unsigned char)l->source[l->pos])) { l->pos++; l->col++; }
    }
    return make_token(is_float ? TOK_FLOAT : TOK_INT, azr_strndup(l->source + start, l->pos - start), l->line, col);
}

static void append_byte(char **buf, int *len, int *cap, char c) {
    if (*len + 2 >= *cap) {
        *cap = *cap ? *cap * 2 : 32;
        *buf = (char *)realloc(*buf, (size_t)*cap);
    }
    (*buf)[(*len)++] = c;
    (*buf)[*len] = '\0';
}

static int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void append_utf8_codepoint(char **buf, int *len, int *cap, unsigned int cp) {
    if (cp <= 0x7F) append_byte(buf, len, cap, (char)cp);
    else if (cp <= 0x7FF) {
        append_byte(buf, len, cap, (char)(0xC0 | (cp >> 6)));
        append_byte(buf, len, cap, (char)(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        append_byte(buf, len, cap, (char)(0xE0 | (cp >> 12)));
        append_byte(buf, len, cap, (char)(0x80 | ((cp >> 6) & 0x3F)));
        append_byte(buf, len, cap, (char)(0x80 | (cp & 0x3F)));
    } else if (cp <= 0x10FFFF) {
        append_byte(buf, len, cap, (char)(0xF0 | (cp >> 18)));
        append_byte(buf, len, cap, (char)(0x80 | ((cp >> 12) & 0x3F)));
        append_byte(buf, len, cap, (char)(0x80 | ((cp >> 6) & 0x3F)));
        append_byte(buf, len, cap, (char)(0x80 | (cp & 0x3F)));
    }
}

static int read_hex_escape(Lexer *l, int digits, unsigned int *out) {
    int k = 0, h = 0;
    unsigned int cp = 0;
    for (k = 0; k < digits; k++) {
        h = hex_value(l->source[l->pos]);
        if (h < 0) return 0;
        cp = (cp << 4) | (unsigned int)h;
        l->pos++;
        l->col++;
    }
    *out = cp;
    return 1;
}

/* Lexes a quoted UTF-8 string literal and decodes common C/Python escapes. */
static Token *lex_string(Lexer *l) {
    char quote = l->source[l->pos];
    int col = l->col;
    char *buf = NULL;
    int len = 0, cap = 0;
    l->pos++; l->col++;
    while (!is_at_end(l) && l->source[l->pos] != quote) {
        if (l->source[l->pos] == '\\' && l->source[l->pos + 1]) {
            char e = 0;
            unsigned int cp = 0;
            l->pos++; l->col++;
            e = l->source[l->pos++];
            l->col++;
            switch (e) {
                case 'n': append_byte(&buf, &len, &cap, '\n'); break;
                case 't': append_byte(&buf, &len, &cap, '\t'); break;
                case 'r': append_byte(&buf, &len, &cap, '\r'); break;
                case 'b': append_byte(&buf, &len, &cap, '\b'); break;
                case 'f': append_byte(&buf, &len, &cap, '\f'); break;
                case 'v': append_byte(&buf, &len, &cap, '\v'); break;
                case '0': append_byte(&buf, &len, &cap, '\0'); break;
                case '\\': append_byte(&buf, &len, &cap, '\\'); break;
                case '"': append_byte(&buf, &len, &cap, '"'); break;
                case '\'': append_byte(&buf, &len, &cap, '\''); break;
                case '{': append_byte(&buf, &len, &cap, '{'); break;
                case '}': append_byte(&buf, &len, &cap, '}'); break;
                case 'x':
                    if (read_hex_escape(l, 2, &cp)) append_byte(&buf, &len, &cap, (char)cp);
                    break;
                case 'u':
                    if (read_hex_escape(l, 4, &cp)) append_utf8_codepoint(&buf, &len, &cap, cp);
                    break;
                case 'U':
                    if (read_hex_escape(l, 8, &cp)) append_utf8_codepoint(&buf, &len, &cap, cp);
                    break;
                default:
                    append_byte(&buf, &len, &cap, e);
                    break;
            }
        } else {
            int br = utf8_charlen((unsigned char)l->source[l->pos]);
            int k = 0;
            for (k = 0; k < br; k++) append_byte(&buf, &len, &cap, l->source[l->pos + k]);
            l->pos += br;
            l->col++;
        }
    }
    {
        Token *t = make_token(TOK_STRING, buf ? buf : "", l->line, col);
        free(buf);
        if (l->source[l->pos] == quote) { l->pos++; l->col++; }
        return t;
    }
}

/* Lexes a UTF-8 identifier or Farsi keyword token. */
static Token *lex_identifier(Lexer *l) {
    int start = l->pos, col = l->col;
    int br = 0;
    while (!is_at_end(l)) {
        uint32_t cp = utf8_codepoint(l->source + l->pos, &br);
        if (!is_ident_cp(cp)) break;
        l->pos += br;
        l->col++;
    }
    {
        char *raw = azr_strndup(l->source + start, l->pos - start);
        Token *t = make_token(keyword_type(raw), raw, l->line, col);
        free(raw);
        return combine_keyword_if_needed(l, t);
    }
}

/* Returns the next token from the UTF-8 source stream. */
Token *lexer_next_token(Lexer *l) {
    Token *pending = pop_pending(l);
    if (pending) return pending;
    if (l->at_line_start && l->nesting > 0) {
        while (l->source[l->pos] == ' ' || l->source[l->pos] == '\t') { l->pos++; l->col++; }
        l->at_line_start = 0;
    }
    if (l->at_line_start) {
        handle_indent(l);
        pending = pop_pending(l);
        if (pending) return pending;
    }
    while (l->source[l->pos] == ' ' || l->source[l->pos] == '\t') { l->pos++; l->col++; }
    if (is_at_end(l)) {
        if (l->indent_top > 0) {
            l->indent_top--;
            return make_token(TOK_DEDENT, "", l->line, l->col);
        }
        return make_token(TOK_EOF, "", l->line, l->col);
    }
    if (l->source[l->pos] == '#') {
        skip_line_comment(l);
        return lexer_next_token(l);
    }
    if (at_triple_comment(l)) {
        skip_triple_comment(l);
        return lexer_next_token(l);
    }
    if (l->source[l->pos] == '\r') l->pos++;
    if (l->source[l->pos] == '\n') {
        l->pos++;
        l->line++;
        l->col = 1;
        l->at_line_start = 1;
        if (l->nesting > 0) return lexer_next_token(l);
        return make_token(TOK_NEWLINE, "", l->line - 1, 1);
    }
    if (isdigit((unsigned char)l->source[l->pos])) return lex_number(l);
    if (l->source[l->pos] == '"' || l->source[l->pos] == '\'') return lex_string(l);
    if (strncmp(l->source + l->pos, "،", strlen("،")) == 0) {
        int col = l->col;
        l->pos += (int)strlen("،"); l->col++;
        return make_token(TOK_COMMA, "،", l->line, col);
    }
    {
        int br = 0, col = l->col;
        uint32_t cp = utf8_codepoint(l->source + l->pos, &br);
        if (is_ident_cp(cp) && !isdigit((unsigned char)l->source[l->pos])) return lex_identifier(l);
        (void)col;
    }
    {
        char c = l->source[l->pos++];
        int col = l->col++;
        char two[3] = { c, l->source[l->pos], 0 };
        if (strcmp(two, "//") == 0) { l->pos++; l->col++; return make_token(TOK_DOUBLESLASH, "//", l->line, col); }
        if (strcmp(two, "**") == 0) { l->pos++; l->col++; return make_token(TOK_DOUBLESTAR, "**", l->line, col); }
        if (strcmp(two, "==") == 0) { l->pos++; l->col++; return make_token(TOK_EQ, "==", l->line, col); }
        if (strcmp(two, "!=") == 0) { l->pos++; l->col++; return make_token(TOK_NEQ, "!=", l->line, col); }
        if (strcmp(two, ">=") == 0) { l->pos++; l->col++; return make_token(TOK_GTE, ">=", l->line, col); }
        if (strcmp(two, "<=") == 0) { l->pos++; l->col++; return make_token(TOK_LTE, "<=", l->line, col); }
        if (strcmp(two, "+=") == 0) { l->pos++; l->col++; return make_token(TOK_PLUS_ASSIGN, "+=", l->line, col); }
        if (strcmp(two, "-=") == 0) { l->pos++; l->col++; return make_token(TOK_MINUS_ASSIGN, "-=", l->line, col); }
        if (strcmp(two, "*=") == 0) { l->pos++; l->col++; return make_token(TOK_STAR_ASSIGN, "*=", l->line, col); }
        if (strcmp(two, "/=") == 0) { l->pos++; l->col++; return make_token(TOK_SLASH_ASSIGN, "/=", l->line, col); }
        switch (c) {
            case '+': return make_token(TOK_PLUS, "+", l->line, col);
            case '-': return make_token(TOK_MINUS, "-", l->line, col);
            case '*': return make_token(TOK_STAR, "*", l->line, col);
            case '/': return make_token(TOK_SLASH, "/", l->line, col);
            case '%': return make_token(TOK_PERCENT, "%", l->line, col);
            case '>': return make_token(TOK_GT, ">", l->line, col);
            case '<': return make_token(TOK_LT, "<", l->line, col);
            case '=': return make_token(TOK_ASSIGN, "=", l->line, col);
            case '(': return delimiter_token(l, TOK_LPAREN, "(", l->line, col);
            case ')': return delimiter_token(l, TOK_RPAREN, ")", l->line, col);
            case '[': return delimiter_token(l, TOK_LBRACKET, "[", l->line, col);
            case ']': return delimiter_token(l, TOK_RBRACKET, "]", l->line, col);
            case '{': return delimiter_token(l, TOK_LBRACE, "{", l->line, col);
            case '}': return delimiter_token(l, TOK_RBRACE, "}", l->line, col);
            case ':': return make_token(TOK_COLON, ":", l->line, col);
            case ',': return make_token(TOK_COMMA, ",", l->line, col);
            case '.': return make_token(TOK_DOT, ".", l->line, col);
            case '?': return make_token(TOK_QUESTION, "?", l->line, col);
            default: return make_token(TOK_NEWLINE, "نویسه ناشناخته", l->line, col);
        }
    }
}

/* Tokenizes the entire source into a null-terminated dynamic token list. */
Token **lexer_tokenize_all(Lexer *l, int *count) {
    int cap = 128;
    Token **tokens = (Token **)calloc((size_t)cap, sizeof(Token *));
    *count = 0;
    while (1) {
        Token *t = lexer_next_token(l);
        if (*count >= cap) {
            cap *= 2;
            tokens = (Token **)realloc(tokens, (size_t)cap * sizeof(Token *));
        }
        tokens[(*count)++] = t;
        if (t->type == TOK_EOF) break;
    }
    return tokens;
}

/* Frees the lexer and queued pending tokens. */
void lexer_free(Lexer *l) {
    int i = 0;
    if (!l) return;
    for (i = 0; i < l->pending_count; i++) token_free(l->pending[i]);
    free(l->pending);
    free(l);
}

/* Frees one token and its copied raw text. */
void token_free(Token *t) {
    if (!t) return;
    free(t->value);
    free(t);
}

/* Returns a debug name for a token type. */
char *token_type_name(TokenType t) {
    static char *names[] = {
        "INT","FLOAT","STRING","IDENTIFIER","AGE","VAGARNA","VAGARNA_AGE","TA_VAGTI","BARAY_HAR","TOOY",
        "KAR","BEDE","BENEVIS","BEGIR","BIYAR","DERASTE","GHALATE","HICHI","VA","YA","NA","BEZAN_BIRON",
        "EDAME","HICHICHI","NOE","SAY_KON","AGE_KHATA","BALAKHARE","BA","BE_ONVAN","AZ","TA","BARGASHT","BIAND","LAMBDA","YIELD",
        "PLUS","MINUS","STAR","SLASH","DOUBLESLASH","PERCENT","DOUBLESTAR","EQ","NEQ","GT","LT","GTE","LTE",
        "ASSIGN","PLUS_ASSIGN","MINUS_ASSIGN","STAR_ASSIGN","SLASH_ASSIGN","LPAREN","RPAREN","LBRACKET",
        "RBRACKET","LBRACE","RBRACE","COLON","COMMA","DOT","QUESTION","NEWLINE","INDENT","DEDENT","EOF"
    };
    return names[(int)t];
}
