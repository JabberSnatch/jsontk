%union {
	   char* string;
	   int number;
}

%{

//int yylex(ParseContext* context);

%}

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

tag:
	QUOTE STRING QUOTE
	;

key:
	tag
	;

boolean:
	TRUE
	| FALSE

value:
	tag
	| NUMBER
	| object
	| array
	| boolean
	| NULL_TOKEN
	;

%%
