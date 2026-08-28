#ifndef AZR_LEXER_H
#define AZR_LEXER_H

typedef enum {
    TOK_INT, TOK_FLOAT, TOK_STRING, TOK_IDENTIFIER,
    TOK_AGE, TOK_VAGARNA, TOK_VAGARNA_AGE, TOK_TA_VAGTI, TOK_BARAY_HAR, TOK_TOOY,
    TOK_KAR, TOK_BEDE, TOK_BENEVIS, TOK_BEGIR, TOK_BIYAR, TOK_DERASTE, TOK_GHALATE,
    TOK_HICHI, TOK_VA, TOK_YA, TOK_NA, TOK_BEZAN_BIRON, TOK_EDAME, TOK_HICHICHI,
    TOK_NOE, TOK_SAY_KON, TOK_AGE_KHATA, TOK_BALAKHARE, TOK_BA, TOK_BE_ONVAN,
    TOK_AZ, TOK_TA, TOK_BARGASHT,
    TOK_BIAND, TOK_LAMBDA, TOK_YIELD,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_DOUBLESLASH, TOK_PERCENT, TOK_DOUBLESTAR,
    TOK_EQ, TOK_NEQ, TOK_GT, TOK_LT, TOK_GTE, TOK_LTE, TOK_ASSIGN, TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN, TOK_STAR_ASSIGN, TOK_SLASH_ASSIGN,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACKET, TOK_RBRACKET, TOK_LBRACE, TOK_RBRACE,
    TOK_COLON, TOK_COMMA, TOK_DOT, TOK_QUESTION, TOK_NEWLINE, TOK_INDENT, TOK_DEDENT, TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    int line;
    int col;
} Token;

typedef struct {
    const char *source;
    int pos;
    int line;
    int col;
    int at_line_start;
    int nesting;
    int indents[256];
    int indent_top;
    Token **pending;
    int pending_count;
    int pending_cap;
} Lexer;

Lexer *lexer_new(const char *source);
Token *lexer_next_token(Lexer *l);
Token **lexer_tokenize_all(Lexer *l, int *count);
void lexer_free(Lexer *l);
void token_free(Token *t);
char *token_type_name(TokenType t);

#endif
