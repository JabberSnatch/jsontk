%union {
    struct {
        char const* begin;
        char const* end;
    } string;
    int number;
}

%parse-param {ParseContext* context}
%lex-param {context}

%pure-parser

%token                  QUOTE
%token                  LEFT_BRACE RIGHT_BRACE LEFT_BRACKET RIGHT_BRACKET
%token                  COMMA
%token                  COLON
%token                  TRUE FALSE NULL_TOKEN

%token  <string>        STRING
%token  <number>        NUMBER

%%

input:
                object;

object:
                LEFT_BRACE { context->BeginObject(); } fields RIGHT_BRACE { context->PopValue(); };

fields:
                %empty
        |       field
        |       field COMMA fields;

field:
                key COLON value { context->PushObjectField(); };

array:
                LEFT_BRACKET { context->BeginArray(); } elements RIGHT_BRACKET { context->PopValue(); };

elements:
                %empty
        |       element
        |       element COMMA elements;

element:
                value { context->PushArrayElement(); };

string:
                QUOTE STRING QUOTE
                {
                    context->next_value = JsonValue{
                        std::string{ $2.begin, (size_t)std::distance($2.begin, $2.end) }
                    };
                };

key:
                QUOTE STRING QUOTE
                {
                    context->next_key = std::string($2.begin, std::distance($2.begin, $2.end));
                };

boolean:
                TRUE
                {
                    context->next_value = JsonValue{ true };
                }
        |       FALSE
                {
                    context->next_value = JsonValue{ false };
                };

value:
                string
        |       NUMBER { context->next_value = JsonValue{ $1 }; }
        |       object
        |       array
        |       boolean
        |       NULL_TOKEN { context->next_value = JsonValue{ nullptr }; };

%%
