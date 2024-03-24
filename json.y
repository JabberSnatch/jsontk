%union {
	   char* string;
	   int number;
}

%parse-param {ParseContext* context}
%lex-param {context}

%pure-parser

%token QUOTE
%token LEFT_BRACE RIGHT_BRACE LEFT_BRACKET RIGHT_BRACKET
%token COMMA
%token COLON
%token TRUE FALSE NULL_TOKEN

%token <string> STRING
%token <number> NUMBER

%%

input:
	object
	;

object:
	LEFT_BRACE fields RIGHT_BRACE
	;

fields:
	%empty
	| field
	| field COMMA fields
	;

field:
	key COLON value
	;

array:
	LEFT_BRACKET values RIGHT_BRACKET
	;

values:
	%empty
	| value
	| value COMMA values
	;

string:
	QUOTE STRING QUOTE { context->values.emplace(context->next_value_key, std::variant<int, char const*>{ $2 }); }
	;

key:
	QUOTE STRING QUOTE  { context->next_value_key = std::string($2); }
	;

boolean:
	TRUE
	| FALSE
	;

value:
	string
	| NUMBER { context->values.emplace(context->next_value_key, std::variant<int, char const*>{ $1 }); }
	| object
	| array
	| boolean
	| NULL_TOKEN
	;

%%
