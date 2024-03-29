#include <cstring>
#include <iostream>

#include <string>
#include <unordered_map>
#include <variant>

union YYSTYPE;


struct JsonObject;
struct JsonArray;

using JsonObjectStorage = std::unique_ptr<JsonObject>;
using JsonArrayStorage = std::unique_ptr<JsonArray>;
using JsonValue =
    std::variant<int, std::string, bool, JsonObjectStorage, JsonArrayStorage, nullptr_t>;

struct JsonObject : public std::unordered_map<std::string, JsonValue>
{};

struct JsonArray : public std::vector<JsonValue>
{};

struct ParseContext {
    ParseContext(char const* _source)
        : source{ _source }
        , next_char{ _source }
        , next_key{ }
        , next_value{ }
        , key_stack{ }
        , value_stack{ }
    {}

    JsonObject& ParseOutput() { return *std::get<JsonObjectStorage>(next_value); }

    char const* source;
    char const* next_char;

    std::string next_key;
    JsonValue next_value;

    std::vector<std::string> key_stack;
    std::vector<JsonValue> value_stack;

    void PushObjectField()
    {
        JsonObject* object = std::get<JsonObjectStorage>(value_stack.back()).get();
        object->emplace(next_key, std::move(next_value));
    }

    void PushArrayElement()
    {
        JsonArray* array = std::get<JsonArrayStorage>(value_stack.back()).get();
        array->emplace_back(std::move(next_value));
    }

    void BeginObject() {
        key_stack.push_back(next_key);
        value_stack.emplace_back(std::make_unique<JsonObject>());
    }

    void BeginArray() {
        key_stack.push_back(next_key);
        value_stack.emplace_back(std::make_unique<JsonArray>());
    }

    void PopValue() {
        next_key = std::move(key_stack.back());
        key_stack.pop_back();
        next_value = std::move(value_stack.back());
        value_stack.pop_back();
    }
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
        while (*++context->next_char != '"'
               && *context->next_char != '\0');
        yylval->string.end = context->next_char;
        return STRING;
    }
    }
}


void PrintJsonValue(JsonValue const& value, uint32_t depth);

void PrintDepth(uint32_t depth)
{
    for (uint32_t index = 0; index < depth; ++index)
        std::cout << "\t";
}

void PrintJsonArray(JsonArray const& array, uint32_t depth)
{
    std::cout << "[" << std::endl;
    for (auto it = array.begin(); it != array.end(); ++it)
    {
        JsonValue const& v = *it;
        PrintDepth(depth+1);
        PrintJsonValue(v, depth+1);
        if (std::next(it) != array.end())
            std::cout << ",";
        std::cout << std::endl;
    }

    PrintDepth(depth);
    std::cout << "]";
}

void PrintJsonObject(JsonObject const& object, uint32_t depth)
{
    std::cout << "{" << std::endl;
    for (auto it = object.begin(); it != object.end(); ++it)
    {
        auto const& pair = *it;
        PrintDepth(depth+1);
        std::cout << "\"" << pair.first << "\": ";
        PrintJsonValue(pair.second, depth+1);
        if (std::next(it) != object.end())
            std::cout << ",";
        std::cout << std::endl;
    }

    PrintDepth(depth);
    std::cout << "}";
}

void PrintJsonValue(JsonValue const& value, uint32_t depth)
{
    std::visit([depth](auto&& v){
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, int>
                      || std::is_same_v<T, bool>) {
            std::cout << v;
        }
        if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "\"" << v << "\"";
        }
        if constexpr (std::is_same_v<T, nullptr_t>) {
            std::cout << "null";
        }
        if constexpr (std::is_same_v<T, JsonObjectStorage>) {
            PrintJsonObject(*v, depth);
        }
        if constexpr (std::is_same_v<T, JsonArrayStorage>) {
            PrintJsonArray(*v, depth);
        }
    }, value);
}

int main(int argc, char const** argv)
{
    if (argc == 2)
    {
        std::FILE* file = std::fopen(argv[1], "r");
        std::fseek(file, 0, SEEK_END);
        uint64_t size = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);
        std::string contents(size, '\0');
        std::fread(contents.data(), 1, size, file);
        std::fclose(file);

        ParseContext context{ contents.c_str() };
        yyparse(&context);
        PrintJsonObject(context.ParseOutput(), 0);
        return 0;
    }
    else
    {

        char const* source = R"(
{ "z":
{ "a": 0, "b": "muc"},
 "A" : 0, "B":null, "C" : "CO UCOU", "D": {},
"E": [0 , 1, 2, 4, "coucou" ] }
    )";

        ParseContext context{ source };
        yyparse(&context);
        JsonObject root = std::move(context.ParseOutput());
        PrintJsonObject(root, 0);
        return 0;
    }
}
