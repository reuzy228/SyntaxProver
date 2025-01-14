#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <fstream>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "../math/ast.hpp"


struct Node
{
	std::string expression;
	std::string rule;
	std::vector<std::string> dependencies;

	Node(std::string expression = "", std::string rule = "")
		: expression(std::move(expression))
		, rule(std::move(rule))
		, dependencies({})
	{
		if (rule == "")
		{
			rule = "axiom";
		}
	}
};


class Solver
{
	std::unordered_set<std::string> known_axioms_;

	// map hash value of expression to hash_values of dependent expressions
	std::vector<Expression> axioms_;
	std::vector<Expression> produced_;

	std::vector<Expression> targets_;
	std::uint64_t time_limit_;

	// stream to store thought chain
	std::stringstream ss;
	std::ofstream dump_;

	// Γ ⊢ A → B <=> Γ U {A} ⊢ B
	bool deduction_theorem_decomposition(Expression expression);

	// iteration function
	void produce(std::size_t max_len);

	// is any target if follows from expression?
	bool is_target_proved_by(const Expression &expression) const;

	// determine whether expression is good or not based on heuristic function
	bool is_good_expression(const Expression &expression, std::size_t max_len) const;

	void build_thought_chain(Expression proof, Expression proved_target);
public:
	Solver(std::vector<Expression> axioms,
		Expression target,
		std::uint64_t time_limit_ms = 60000
	);

	void solve();
	std::string thought_chain() const;
};

#endif // SOLVER_HPP
