#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "token.h"

static void token_skipSpaces(FILE *file)
{
    int c;
    while ((c = fgetc(file)) != EOF)
    {
        if (strchr("\t ", c))
            continue;
        else if (c == '#')
        {
            while (((c = fgetc(file)) != EOF) && (c != '\n'));

            if (c == '\n')
                ungetc(c, file);
        }
        else
        {
            ungetc(c, file);
            break;
        }
    }
}

int token_readToken(FILE *file, Token *token)
{
    int c, i = 0;
    token_skipSpaces(file);

    if (!token)
        printf("Error: token is a NULL pointer.\n");

    switch(c = fgetc(file))
    {
    case EOF:
        token->type = TOKEN_EOF;
        return TOKEN_EOF;
    case '"':
        while (1)
        {
            if (i >= TOKEN_MAXSIZE-1)
            {
                printf("Error: String bigger than TOKEN_MAXSIZE.\n");
                token->str[0] = '\0';
                return TOKEN_ERR;
            }

            c = fgetc(file);
            if (c == '\\')
                c = fgetc(file);
            else if (c == '"')
                break;

            if ((c == '\n') || (c == EOF))
            {
                printf("Warning: Unterminated string.");
                ungetc(c, file);
                token->str[i] = '\0';
                token->type = TOKEN_STRING;
                return TOKEN_STRING;
            }

            token->str[i++] = (char)c;
        }

        token->str[i] = '\0';
        token->type = TOKEN_STRING;
        return TOKEN_STRING;
    case '=':
        token->type = TOKEN_EQ;
        return TOKEN_EQ;
    case '[':
        token->type = TOKEN_OPENBRACK;
        return TOKEN_OPENBRACK;
    case ']':
        token->type = TOKEN_CLOSEBRACK;
        return TOKEN_CLOSEBRACK;
    case '\n':
    case '\r':
        token->type = TOKEN_NL;
        return TOKEN_NL;
    default:
        ungetc(c, file);

        while(1)
        {
            if (i >= TOKEN_MAXSIZE-1)
            {
                printf("Error: String bigger than TOKEN_MAXSIZE.\n");
                token->str[0] = '\0';
                return TOKEN_ERR;
            }

            c = fgetc(file);

            if (strchr("=#[]\"\t\n\r ", c) || (c == EOF))
            {
                ungetc(c, file);
                break;
            }

            token->str[i++] = (char)c;
        }

        token->str[i] = '\0';
        token->type = TOKEN_STRING;
        return TOKEN_STRING;
    }
}
