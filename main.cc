#include <cstring>
#include <iostream>

#include <string>
#include <unordered_map>
#include <variant>

union YYSTYPE;

using SemanticValue = std::variant<int, std::string>;

struct ParseContext {
    char const* source;
    char const* next_char;

    std::string next_value_key;
    std::unordered_map<std::string, SemanticValue> values;
};

void yyerror(ParseContext* context, char const* msg)
{
    std::cout << msg << std::endl;
}
int yylex(YYSTYPE*, ParseContext* context);

#include "json.tab.c"

int yylex(YYSTYPE* yylval, ParseContext* context)
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
            char const* number_begin = context->next_char;
            while (*++context->next_char >= '0' && *context->next_char <= '9'
                   && *context->next_char != '\0');
            char const* number_end = context->next_char;
            yylval->number = std::stoi(
                std::string{ number_begin, (size_t)std::distance(number_begin, number_end) });
            return NUMBER;
        }

        yylval->string.begin = context->next_char;
        while (*++context->next_char != ' '
               && *context->next_char != '"'
               && *context->next_char != '\0');
        yylval->string.end = context->next_char;
        return STRING;
    }
    }
}

int main()
{
    char const* source = "{ \"A\" : 0, \"B\":1234, \"C\" : \"COUCOU\" }";
    ParseContext context = {
        source,
        source
    };
    yyparse(&context);
    return 0;
}
