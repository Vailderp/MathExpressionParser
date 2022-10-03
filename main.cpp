#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

typedef enum _e_operator_token : unsigned char
{
    EP_OPERATOR_ITERATOR = 0x01,
    EP_ADD = 0 * EP_OPERATOR_ITERATOR,
    EP_SUB = 1 * EP_OPERATOR_ITERATOR,
    EP_MUL = 2 * EP_OPERATOR_ITERATOR,
    EP_DIV = 3 * EP_OPERATOR_ITERATOR,
    EP_OPERATOR_BEGIN = EP_ADD,
    EP_OPERATOR_END = EP_DIV

} _e_token;

typedef enum _e_ent_token : unsigned char
{
    EP_ENT_ITERATOR = 0x01,
    EP_ENT_BEGIN = EP_OPERATOR_END + EP_ENT_ITERATOR,
	EP_ROUND_BKT_OPEN = 0 * EP_ENT_ITERATOR + EP_ENT_BEGIN,
	EP_ROUND_BKT_CLOSE = 1 * EP_ENT_ITERATOR + EP_ENT_BEGIN,
    EP_VARIABLE = 2 * EP_ENT_ITERATOR + EP_ENT_BEGIN,
    EP_FUNCTION = 3 * EP_ENT_ITERATOR + EP_ENT_BEGIN,
    EP_NUMBER = 4 * EP_ENT_ITERATOR + EP_ENT_BEGIN,
    EP_ENT_END = EP_NUMBER
} _e_ent_token;

constexpr char name_chars[] = "qwertyuiopasdfghjklzxcvbnm1234567890_";
constexpr char punctuator_chars[] = "[](){}.,-+<>!/%^&|#*=;: ";
constexpr char name_correct_number_value[] = "1234567890";

struct Lexeme
{
	unsigned char token;
	const char* name;
	const unsigned short length;
	constexpr bool __fastcall operator == (Lexeme lexeme) const
	{
		if (this->length != lexeme.length)
			return false;
		for (unsigned short i = 0; i < length; i++)
			if (this->name[i] != lexeme.name[i])
				return  false;
		return true;
	}
	constexpr bool __fastcall operator == (const char* chars) const
	{
		for (unsigned short i = 0; i < length; i++)
			if (this->name[i] != chars[i])
				return  false;
		return true;
	}
};

struct MathLexeme
{
	Lexeme lexeme;
	const unsigned char priority;
};

struct _s_cmp_str
{
	bool operator()(char const* a, char const* b) const
	{
		return std::strcmp(a, b) < 0;
	}
};

constexpr MathLexeme tokenizer_lexicon[] =
{
	EP_ADD, "+", 1, 1,
	EP_SUB, "-", 1, 1,
	EP_MUL, "*", 1, 2,
	EP_DIV, "/", 1, 2,
	EP_ROUND_BKT_OPEN, "(", 1, 3,
	EP_ROUND_BKT_CLOSE, ")", 1, 3,
};

template <unsigned short _Chars_length>
bool char_equal_chars(const char ch, const char chs[_Chars_length])
{
	for (unsigned short i = 0; i < _Chars_length; i++)
		if (ch == chs[i])
			return true;
	return false;
}

template <unsigned short _Chars_length>
unsigned short __fastcall how_long_to_chars(const char* code, size_t& offset, const size_t code_length, const char chs[_Chars_length])
{
	unsigned short i = 0;
	if (offset + 1 >= code_length)
		return i;
	while (!char_equal_chars<_Chars_length>(code[offset], chs))
	{
		if (offset++ >= code_length)
			return i;
		i++;
	}
	return i;
}

inline char* __fastcall create_chars(const char* source, const size_t chars_length)
{
	char* chars = new char[chars_length + 1];
	memcpy(chars, source, chars_length);
	chars[chars_length] = '\0';
	return chars;
}

struct _s_trimmed_chars_result
{
	char* chars;
	unsigned short chars_size;
};

template <unsigned short _Chars_length>
_s_trimmed_chars_result __fastcall get_trimmed_chars(const char* code, size_t& offset, const size_t code_length, const char chs[_Chars_length])
{
	const unsigned short chars_size = how_long_to_chars<_Chars_length>(code, offset, code_length, chs);
	return { create_chars(&code[offset - chars_size], chars_size),  chars_size };
}

inline bool __fastcall is_digit(const char ch)
{
	return char_equal_chars<10>(ch, name_correct_number_value);
}

inline bool __fastcall is_correct_number_value(const char* value_chars)
{
	unsigned short i = 0;
	if (value_chars[0] == '-')
	{
		return false;
	}
	while (value_chars[i] != '\0')
	{
		if (!char_equal_chars<10>(value_chars[i], name_correct_number_value))
			return false;
		i++;
	}
	return true;
}

inline bool __fastcall is_correct_char_value(const char* value_chars)
{
	if (value_chars[0] != '\'' || value_chars[2] != '\'')
		return false;
	return true;
}

inline bool __fastcall is_correct_variable_name(const char* value_chars)
{
	if (is_digit(value_chars[0]))
		return false;
	unsigned short i = 0;
	while (value_chars[i] != '\0')
	{
		if (!char_equal_chars<38>(value_chars[i], name_chars))
			return false;
		i++;
	}
	return true;
}

struct _s_split_result
{
	char** words = {};
	size_t words_count = 0;
	char* operator[](size_t word_index) const
	{
		if (word_index >= words_count)
			return nullptr;
		return words[word_index];
	}
};

inline void __fastcall skip_char(const char* code, size_t& offset, const size_t code_length, const char char_to_skip)
{
	if (offset >= code_length)
		return;
	while (code[offset] == char_to_skip)
		if (offset++ >= code_length)
			return;
}

template <unsigned short _Chars_length>
void __fastcall skip_chars(const char* code, size_t& offset, const size_t code_length,
	const char chars_to_skip[_Chars_length])
{
	if (offset >= code_length)
	{
		offset++;
		return;
	}
	if (!char_equal_chars<_Chars_length>(code[offset], chars_to_skip))
		return;
	while (char_equal_chars<_Chars_length>(code[offset], chars_to_skip))
		if (offset++ >= code_length)
			return;
}

inline void __fastcall skip_space_chars(const char* code, size_t& offset, const size_t code_length)
{
	skip_chars<1>(code, offset, code_length, " ");
}

inline void __fastcall delete_split(const _s_split_result* split_result)
{
	for (int i = 0; i < split_result->words_count; i++)
		delete[] split_result->words[i];
	delete[] split_result->words;
}

template <unsigned short _Chars_length>
_s_split_result __fastcall split(const char* chars, const size_t chars_length, const char chs[_Chars_length])
{
	size_t words_count = 0;
	for (size_t offset = 0; offset < chars_length; offset++)
	{
		skip_chars<_Chars_length>(chars, offset, chars_length, chs);
		how_long_to_chars<_Chars_length>(chars, offset, chars_length, chs);
		words_count++;
	}
	if (words_count <= 0)
	{
		char* empty_result = create_chars(chars, chars_length);
		return { &empty_result, 1 };
	}
	char** words = new char* [words_count];
	size_t word_offset = 0;
	for (size_t offset = 0; offset < chars_length; offset++)
	{
		skip_chars<_Chars_length>(chars, offset, chars_length, chs);
		if (word_offset >= words_count)
			return { nullptr, 0 };
		words[word_offset] = get_trimmed_chars<_Chars_length>(chars, offset, chars_length, chs).chars;
		word_offset++;
	}
	return { words, words_count };
}


inline _s_split_result __fastcall split_space(const char* chars, const size_t chars_length)
{
	return split<2>(chars, chars_length, " \n");
}

struct Tokenizer
{
	std::vector<Lexeme> lexemes;
};

inline Tokenizer* __fastcall tokenize_chars(const char* code, size_t code_length)
{
	Tokenizer* tokenizer = new Tokenizer;
	size_t offset = 0;
	while (code[offset] != '\0')
	{
		bool find = false;
		skip_space_chars(code, offset, code_length);
		for (unsigned char i = EP_OPERATOR_BEGIN; i <= EP_ROUND_BKT_CLOSE; i++)
		{
			const Lexeme lexeme = { 0xFF, &code[offset], tokenizer_lexicon[i].lexeme.length };
			if (tokenizer_lexicon[i].lexeme == lexeme)
			{
				tokenizer->lexemes.push_back(tokenizer_lexicon[i].lexeme);
				offset += tokenizer_lexicon[i].lexeme.length;
				find = true;
				break;
			}
		}
		if (find)
			continue;
		const auto trimmed_result = get_trimmed_chars<24>(code, offset, code_length, punctuator_chars);
		if (is_correct_variable_name(trimmed_result.chars))
		{
			const Lexeme lexeme = { EP_VARIABLE, trimmed_result.chars, trimmed_result.chars_size };
			tokenizer->lexemes.push_back(lexeme);
			continue;
		}
		if (is_correct_number_value(trimmed_result.chars))
		{
			const Lexeme lexeme = { EP_NUMBER, trimmed_result.chars, trimmed_result.chars_size };
			tokenizer->lexemes.push_back(lexeme);
			continue;
		}
		const Lexeme lexeme = { 0xFF, trimmed_result.chars, trimmed_result.chars_size };
		tokenizer->lexemes.push_back(lexeme);
	}
	
	return tokenizer;
}

inline Tokenizer* tokenize_chars_from_file(const char* source_file)
{
	std::ifstream fs(source_file, std::fstream::binary);
	std::string chars;
	if (fs.is_open())
	{
		std::string line;
		while (getline(fs, line))
		{
			if (line[line.size() - 1] == '\r' || line[line.size() - 1] == '\n')
				line.erase(line.end() - 1);
			chars += line;
		}
	}
	const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	Tokenizer* tokenizer = tokenize_chars(chars.c_str(), chars.size());
	const std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	const std::chrono::high_resolution_clock::duration dur = end - start;
	//std::cout << "timing in milliseconds: " << std::chrono::duration<double, std::milli>(dur).count() << " ms" << std::endl;
	//std::cout << "timing in nanoseconds: " << std::chrono::duration<double, std::nano>(dur).count() << " ns" << std::endl;
	return tokenizer;
}

class MathExpression
{
public:
	std::vector<int> number_stack;
	std::vector<unsigned char> operator_stack;
	int __fastcall result()
	{
		
	}
};

inline  MathExpression* __fastcall parse(Tokenizer& tokenizer)
{
	Lexeme* lexeme = tokenizer.lexemes.data();
	const size_t lexeme_count = tokenizer.lexemes.size();
	const Lexeme* lexemes_end = lexeme + lexeme_count;
	MathExpression* math_expression = new MathExpression;

	while (lexeme <= lexemes_end)
	{
		if (lexeme->token >= EP_OPERATOR_BEGIN && lexeme->token <= EP_OPERATOR_END)
		{
			if (math_expression->operator_stack.empty())
			{
				math_expression->operator_stack.push_back(lexeme->token);
				lexeme++;
				continue;
			}
			while(!math_expression->operator_stack.empty())
			{
				if (math_expression->operator_stack.back() == EP_ROUND_BKT_OPEN || math_expression->operator_stack.back() == EP_ROUND_BKT_CLOSE)
					break;
				if(!(tokenizer_lexicon[lexeme->token].priority <= tokenizer_lexicon[math_expression->operator_stack.back()].priority))
					break;
				const int second = math_expression->number_stack.back();
				math_expression->number_stack.pop_back();
				const int first = math_expression->number_stack.back();
				math_expression->number_stack.pop_back();
				switch (math_expression->operator_stack.back())
				{
				case EP_ADD:
					math_expression->number_stack.push_back(first + second);
					break;
				case EP_MUL:
					math_expression->number_stack.push_back(first * second);
					break;
				case EP_SUB:
					math_expression->number_stack.push_back(first - second);
					break;
				case EP_DIV:
					math_expression->number_stack.push_back(first / second);
					break;
				}
				math_expression->operator_stack.pop_back();
			}
			math_expression->operator_stack.push_back(lexeme->token);
			lexeme++;
			continue;
		}
		if (lexeme->token == EP_NUMBER)
		{
			math_expression->number_stack.push_back(atoi(lexeme->name));
			lexeme++;
			continue;
		}
		switch (lexeme->token)
		{
		case EP_ROUND_BKT_OPEN:
			math_expression->operator_stack.push_back(lexeme->token);
			lexeme++;
			break;
		case EP_ROUND_BKT_CLOSE:
			while (!math_expression->operator_stack.empty())
			{
				if (math_expression->operator_stack.back() == EP_ROUND_BKT_OPEN)
				{
					math_expression->operator_stack.pop_back();
					break;
				}
				int second = math_expression->number_stack.back();
				math_expression->number_stack.pop_back();
				int first = math_expression->number_stack.back();
				math_expression->number_stack.pop_back();
				switch (math_expression->operator_stack.back())
				{
				case EP_ADD:
					math_expression->number_stack.push_back(first + second);
					break;
				case EP_MUL:
					math_expression->number_stack.push_back(first * second);
					break;
				case EP_SUB:
					math_expression->number_stack.push_back(first - second);
					break;
				case EP_DIV:
					math_expression->number_stack.push_back(first / second);
					break;
				}
				math_expression->operator_stack.pop_back();
			}
			lexeme++;
			break;
		case 0xFF:
			std::cout << "error => unknown lexeme";
			break;
		default:
			lexeme++;
		}
	}
	while (!math_expression->operator_stack.empty())
	{
		int second = math_expression->number_stack.back();
		math_expression->number_stack.pop_back();
		int first = math_expression->number_stack.back();
		math_expression->number_stack.pop_back();
		switch (math_expression->operator_stack.back())
		{
		case EP_ADD:
			math_expression->number_stack.push_back(first + second);
			break;
		case EP_MUL:
			math_expression->number_stack.push_back(first * second);
			break;
		case EP_SUB:
			math_expression->number_stack.push_back(first - second);
			break;
		case EP_DIV:
			math_expression->number_stack.push_back(first / second);
			break;
		}
		math_expression->operator_stack.pop_back();
	}
	return math_expression;
}

inline MathExpression*  parse_with_timer(Tokenizer& tokenizer)
{
	const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	MathExpression* math_expression = parse(tokenizer);
	const std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	const std::chrono::high_resolution_clock::duration dur = end - start;
	std::cout << "timing in milliseconds: " << std::chrono::duration<double, std::milli>(dur).count() << " ms" << std::endl;
	std::cout << "timing in nanoseconds: " << std::chrono::duration<double, std::nano>(dur).count() << " ns" << std::endl;
	return math_expression;
}

int main()
	{
	Tokenizer* tokenizer = tokenize_chars_from_file("ep.txt");
	MathExpression* exp = parse_with_timer(*tokenizer);
	std::cout << "exp=" << (int)exp->number_stack[0];
    return 0;
}
