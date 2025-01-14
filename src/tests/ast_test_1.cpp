#include <iostream>
#include <cassert>
#include <unordered_map>
#include "../math/ast.hpp"
#include "../parser/parser.hpp"


void test_expression_to_string(const Expression &expr, const std::string &expected) {
    assert(expr.to_string() == expected);
}

// Тесты для создания узлов и выражений
void test_creation_and_to_string() {
    ASTNode node_a(1);
    ASTNode node_b(2);
    ASTNode node_c(3);

    Expression expr_a({node_a});
    Expression expr_b({node_b});
    Expression expr_c({node_c});

    test_expression_to_string(expr_a, "a");
    test_expression_to_string(expr_b, "b");
    test_expression_to_string(expr_c, "c");

    Expression a_and_b = contruct_expression(expr_a, Operation::Conjunction, expr_b);
    test_expression_to_string(a_and_b, "a*b");

    Expression not_c = expr_c.negation();
    test_expression_to_string(not_c, "!c");

    Expression a_and_b_or_not_c = contruct_expression(a_and_b, Operation::Disjunction, not_c);
    test_expression_to_string(a_and_b_or_not_c, "(a*b)|!c");

	Expression implication_op = contruct_expression(a_and_b_or_not_c, Operation::Implication, a_and_b);
    test_expression_to_string(implication_op, "((a*b)|!c)>(a*b)");

	assert(implication_op.n_ops() == 4);
	assert(implication_op.n_vars() == 5);
	assert(implication_op.n_tokens() == 9);

    std::cout << "Test creation and to_string passed." << std::endl;
}

// Тест парсера
void test_parser() {
	std::string consecutive_string = "(a+!b)|(a+!b)";
	std::string test_string = "(a+!b)";
	std::string a_and_not_b = "(a+!b)";
	std::string disjunction_string = "|";
	std::string closing = "";

	for (int i = 0; i < 11; ++i) {
		consecutive_string = consecutive_string + disjunction_string + a_and_not_b;
		test_string = test_string + disjunction_string + "(" + a_and_not_b;
		closing += ")";
	}

	test_string += disjunction_string + a_and_not_b;
	test_string += closing;

	auto consecutive_expression = ExpressionParser(consecutive_string).parse();

	// std::cout << test_string << std::endl;
	// std::cout << consecutive_expression << std::endl;

	test_expression_to_string(consecutive_expression, test_string);

	// consecutive_expression = contruct_expression(expr_a_xor_not_b, Operation::Implication, expr_a_xor_not_b);

	// for (int i = 0; i < 20; ++i) {
	// 	consecutive_expression = contruct_expression(consecutive_expression, Operation::Implication, expr_a_xor_not_b);
	// 	consecutive_string = consecutive_string + implication_string + a_and_not_b;
	// }

    std::cout << "Test consecutive nodes passed." << std::endl;
}

void test_alternating_operations() {
	std::string consecutive_string = "a";
	// std::string test_string = "a";
	std::string a = "a";

	std::unordered_map<int, std::string> op_map=
    {
        {0, "|"},
        {1, "*"},
        {2, "+"},
		{3, ">"},
        {4, "="}
    };

	std::string closing = "";

	for (int i = 0; i < 10; ++i) {

		consecutive_string = consecutive_string + op_map[i % 5] + a;

		// test_string = test_string + conjuction_string + "(" + a_and_not_b;
		closing += ")";
	}

	// test_string += conjuction_string + a_and_not_b;
	// test_string += closing;

	auto alternating_expression = ExpressionParser(consecutive_string).parse();

	std::cout << alternating_expression << std::endl;

	assert(alternating_expression.n_ops() == 10);
	assert(alternating_expression.n_vars() == 11);
	assert(alternating_expression.n_tokens() == 21);

	// std::cout << test_string << std::endl;
	// std::cout << consecutive_expression << std::endl;

	// test_expression_to_string(consecutive_expression, test_string);

    std::cout << "Test alternating operations passed." << std::endl;
}



int main() {
    test_creation_and_to_string();
    test_parser();
	test_alternating_operations();

    std::cout << "All tests passed." << std::endl;
    return 0;
}
