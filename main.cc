#include <cstring>
#include <iostream>

#include <string>
#include <unordered_map>
#include <variant>

union YYSTYPE;

struct ParseContext {
    char const* source;
    char const* next_char;

    std::string next_value_key;
    std::unordered_map<std::string, std::variant<int, char const*>> values;
};

void yyerror(ParseContext* context, char const* msg)
{
    std::cout << msg << std::endl;
}
int yylex(YYSTYPE*, ParseContext* context);

#include "json.tab.c"

int yylex(YYSTYPE*, ParseContext* context)
{
    while(isspace((int)*context->next_char))
        ++context->next_char;

    switch (*context->next_char)
    {
    case '\0': return YYEOF;
    case '"': ++context->next_char; return QUOTE;
    case '{': ++context->next_char; return LEFT_BRACE;
    case '}': ++context->next_char; return RIGHT_BRACE;
    case '[': ++context->next_char; return LEFT_BRACKET;
    case ']': ++context->next_char; return RIGHT_BRACKET;
    case ',': ++context->next_char; return COMMA;
    case ':': ++context->next_char; return COLON;
    default:
    {
        if (!strncmp(context->next_char, "true", 4)) {
            context->next_char += 4;
            return TRUE;
        }

        if (!strncmp(context->next_char, "false", 5)) {
            context->next_char += 5;
            return FALSE;
        }

        if (!strncmp(context->next_char, "null", 4)) {
            context->next_char += 4;
            return NULL_TOKEN;
        }

        if (*context->next_char >= '0' && *context->next_char <= '9') {
            while (*++context->next_char >= '0' && *context->next_char <= '9'
                   && *context->next_char != '\0');
            return NUMBER;
        }

        while (*++context->next_char != ' '
               && *context->next_char != '"'
               && *context->next_char != '\0');
        return STRING;
    }
    }
}

int main()
{
    char const* source = "{ \"member\" : 0 }";
    ParseContext context = {
        source,
        source
    };
    yyparse(&context);
    return 0;
}
