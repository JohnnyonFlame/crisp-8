#ifndef __TOKEN_H__
#define __TOKEN_H__

#define TOKEN_MAXSIZE 80

enum
{
    TOKEN_ERR = -1,
    TOKEN_STRING,
    TOKEN_OPENBRACK,
    TOKEN_CLOSEBRACK,
    TOKEN_EQ,
    TOKEN_NL,
    TOKEN_EOF
};

typedef struct Token
{
    int  type;
    char str[TOKEN_MAXSIZE];
} Token;

int token_readToken(FILE *file, Token *token);

#endif //__TOKEN_H__
