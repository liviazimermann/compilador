#ifndef CONSTANTS_H
#define CONSTANTS_H

enum TokenId 
{
    EPSILON  = 0,
    DOLLAR   = 1,
    t_VAR = 2,
    t_SE = 3,
    t_ENTAO = 4,
    t_SENAO = 5,
    t_FIM = 6,
    t_TIPO_INTEIRO = 7,
    t_TIPO_REAL = 8,
    t_TIPO_CHAR = 9,
    t_TIPO_STRING = 10,
    t_TIPO_BOOLEANO = 11,
    t_TIPO_DOBRO = 12,
    t_BOOL_VERDADEIRO = 13,
    t_BOOL_FALSO = 14,
    t_ENQUANTO = 15,
    t_PARA = 16,
    t_FACA = 17,
    t_LEIA = 18,
    t_ESCREVA = 19,
    t_RETORNE = 20,
    t_NULO = 21,
    t_OP_MAIOR_IGUAL = 22,
    t_OP_MENOR_IGUAL = 23,
    t_OP_IGUAL = 24,
    t_OP_DIFERENTE = 25,
    t_OP_MAIOR = 26,
    t_OP_MENOR = 27,
    t_OP_SOMA = 28,
    t_OP_SUB = 29,
    t_OP_MULT = 30,
    t_OP_DIV = 31,
    t_OP_MOD = 32,
    t_OP_AND = 33,
    t_OP_OR = 34,
    t_OP_NOT = 35,
    t_BIT_SHIFT_R = 36,
    t_BIT_SHIFT_L = 37,
    t_BIT_AND = 38,
    t_BIT_OR = 39,
    t_BIT_NOT = 40,
    t_BIT_XOR = 41,
    t_DELIM_LPAREN = 42,
    t_DELIM_RPAREN = 43,
    t_DELIM_LSB = 44,
    t_DELIM_RSB = 45,
    t_DELIM_LBRACE = 46,
    t_DELIM_RBRACE = 47,
    t_PONT_SEMICOLON = 48,
    t_PONT_COMMA = 49,
    t_PONT_DOT = 50,
    t_PONT_COLON = 51,
    t_OP_ASSIGN = 52,
    t_INT_LITERAL = 53,
    t_REAL_LITERAL = 54,
    t_BIN_LITERAL = 55,
    t_HEX_LITERAL = 56,
    t_CHAR_LITERAL = 57,
    t_STRING_LITERAL = 58,
    t_IDENTIFICADOR = 59
};

const int STATES_COUNT = 144;

extern int SCANNER_TABLE[STATES_COUNT][256];

extern int TOKEN_STATE[STATES_COUNT];

extern const char *SCANNER_ERROR[STATES_COUNT];

const int FIRST_SEMANTIC_ACTION = 105;

const int SHIFT  = 0;
const int REDUCE = 1;
const int ACTION = 2;
const int ACCEPT = 3;
const int GO_TO  = 4;
const int ERROR  = 5;

extern const int PARSER_TABLE[299][209][2];

extern const int PRODUCTIONS[107][2];

extern const char *PARSER_ERROR[299];

#endif
