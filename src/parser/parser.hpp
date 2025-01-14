#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <cstdint>
#include <stack>
#include <string>
#include "../math/ast.hpp"


enum class Token : std::int32_t
{
	Nop = 0,
	Negation,
	Implication,
	Disjunction,
	Conjunction,
	Xor,
	Equivalent,
	OpenBracket,
	CloseBracket
};


class ExpressionParser
{
	/**
	 * number of brackets `(` ~ `+1`; `)` ~ `-1`
	 */
	std::int32_t brackets;

	/**
	 * input expression to be parsed
	 */
	std::string_view expression;

	/**
	 * stacks for rpn
	 */
	std::stack<Expression> operands;
	std::stack<Token> operations;

	/**
	 * helper functions
	 */
	void construct_node();
	bool is_operation(char token);
	operation_t determine_operation(char token);
	Term determine_operand(char token);

public:
	ExpressionParser(std::string_view expression);
	Expression parse();
};


#endif // PARSER_HPP
