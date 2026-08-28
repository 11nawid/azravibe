#ifndef AZR_PARSER_H
#define AZR_PARSER_H

#include "ast.h"
#include "lexer.h"

typedef struct {
    Token **tokens;
    int pos;
    int count;
    char *error;
} Parser;

Parser *parser_new(Token **tokens, int count);
ASTNode *parse_program(Parser *p);
ASTNode *parse_statement(Parser *p);
ASTNode *parse_block(Parser *p);
ASTNode *parse_if(Parser *p);
ASTNode *parse_while(Parser *p);
ASTNode *parse_for(Parser *p);
ASTNode *parse_function_def(Parser *p);
ASTNode *parse_return(Parser *p);
ASTNode *parse_import(Parser *p);
ASTNode *parse_try_except(Parser *p);
ASTNode *parse_class_def(Parser *p);
ASTNode *parse_with(Parser *p);
ASTNode *parse_throw(Parser *p);
ASTNode *parse_expression(Parser *p);
ASTNode *parse_ternary(Parser *p);
ASTNode *parse_assignment(Parser *p);
ASTNode *parse_or(Parser *p);
ASTNode *parse_and(Parser *p);
ASTNode *parse_not(Parser *p);
ASTNode *parse_comparison(Parser *p);
ASTNode *parse_addition(Parser *p);
ASTNode *parse_multiplication(Parser *p);
ASTNode *parse_power(Parser *p);
ASTNode *parse_unary(Parser *p);
ASTNode *parse_call_or_index(Parser *p);
ASTNode *parse_primary(Parser *p);
ASTNode *parse_list_literal(Parser *p);
ASTNode *parse_dict_or_set_literal(Parser *p);
void parser_free(Parser *p);

#endif
