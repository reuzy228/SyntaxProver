#include <unordered_map>
#include <queue>
#include <iostream>
#include <stack>
#include "helper.hpp"


void topological_sort_util(
	value_t v,
	std::vector<std::vector<value_t>> &adj,
	std::vector<bool> &visited,
	std::stack<value_t> &s
)
{
	visited[v] = true;

	for (value_t i : adj[v])
	{
		if (!visited[i])
		{
			topological_sort_util(i, adj, visited, s);
		}
	}

	s.push(v);
}


std::vector<value_t> topological_sort(
	std::vector<std::vector<value_t>> &adj,
	value_t size
)
{
	std::stack<value_t> s;
	std::vector<bool> visited(size, false);
	std::vector<value_t> order;

	for (value_t i = 0; i < size; ++i)
	{
		if (!visited[i])
		{
			topological_sort_util(i, adj, visited, s);
		}
	}

	while (!s.empty())
	{
		order.push_back(s.top());
		s.pop();
	}

	return order;
}


bool add_constraint(
	Term term,
	Expression substitution,
	std::unordered_map<value_t, Expression> &sub
)
{
	if (substitution[0].type == term_t::Function &&
		substitution.contains(term))
	{
		return false;
	}

	sub[term.value] = substitution;
	return true;
}


bool unification(
	Expression left,
	Expression right,
	std::unordered_map<value_t, Expression> &substitution
)
{
	std::unordered_map<value_t, Expression> sub;

	// change variables to avoid intersections
	right.change_variables(left.max_value() + 1);
	value_t v = right.max_value() + 1;

	// algorithm
	// step 1: find the set of mismatches
	// step 2: apply appropriate changes (if possible)
	// goto 1
	// mismatches can only be in `current`, `left` or `right` subtrees
	// therefore we will use preorder tree traverse

	std::queue<std::pair<std::size_t, std::size_t>> expression;
	expression.emplace(left.subtree(0).self(), right.subtree(0).self());

	Expression lhs, rhs;

	while (!expression.empty())
	{
		auto [left_idx, right_idx] = expression.front();
		expression.pop();

		auto left_term = left[left_idx];
		auto right_term = right[right_idx];

		// case 0: both terms are functions
		if (left_term.type == term_t::Function &&
			right_term.type == term_t::Function)
		{
			// it's impossible to unify different operations
			if (left_term.op != right_term.op)
			{
				return false;
			}

			// add nodes to process
			expression.emplace(
				left.subtree(left_idx).left(),
				right.subtree(right_idx).left()
			);
			expression.emplace(
				left.subtree(left_idx).right(),
				right.subtree(right_idx).right()
			);
			continue;
		}

		lhs = std::move(left.subtree_copy(left_idx));
		rhs = std::move(right.subtree_copy(right_idx));

		// adjust terms since it may have subs
		while (lhs[0].type == term_t::Variable &&
			sub.contains(lhs[0].value))
		{
			bool should_negate = lhs[0].op == operation_t::Negation;

			lhs = sub.at(lhs[0].value);
			if (should_negate)
			{
				lhs.negation();
			}
		}
		while (rhs[0].type == term_t::Variable &&
			sub.contains(rhs[0].value))
		{
			bool should_negate = rhs[0].op == operation_t::Negation;

			rhs = sub.at(rhs[0].value);
			if (should_negate)
			{
				rhs.negation();
			}
		}

		// case 1: both terms are constants
		if (lhs[0].type == term_t::Constant &&
			rhs[0].type == term_t::Constant)
		{
			// can't unify two constants
			if (lhs[0] != rhs[0])
			{
				return false;
			}

			// well, everything seems to be fine
			continue;
		}

		// case 2: left term is constant and right is variable
		if (lhs[0].type == term_t::Constant &&
			rhs[0].type == term_t::Variable)
		{
			if (rhs[0].op == operation_t::Negation)
			{
				lhs[0].op =
				lhs[0].op != operation_t::Negation ?
				operation_t::Negation :
				operation_t::Nop;
			}

			if (!add_constraint(rhs[0], lhs, sub))
			{
				return false;
			}

			continue;
		}

		// case 3: left term is variable and right is constant
		if (lhs[0].type == term_t::Variable &&
			rhs[0].type == term_t::Constant)
		{
			if (lhs[0].op == operation_t::Negation)
			{
				rhs[0].op =
				rhs[0].op != operation_t::Negation ?
				operation_t::Negation :
				operation_t::Nop;
			}

			if (!add_constraint(lhs[0], rhs, sub))
			{
				return false;
			}

			continue;
		}

		// case 4: both terms are variables
		if (lhs[0].type == term_t::Variable &&
			rhs[0].type == term_t::Variable)
		{
			// are variables equal?
			if (lhs[0].value == rhs[0].value)
			{
				if (lhs[0].op != rhs[0].op)
				{
					return false;
				}

				continue;
			}

			// add new variable
			Expression expr(Term(
				term_t::Variable,
				lhs[0].op == operation_t::Negation ||
				rhs[0].op == operation_t::Negation ?
				operation_t::Negation :
				operation_t::Nop,
				v++
			));
			Expression neg_expr = expr;
			neg_expr.negation();

			if (lhs[0].op == operation_t::Negation)
			{
				add_constraint(lhs[0], neg_expr, sub);
			}
			else
			{
				add_constraint(lhs[0], expr, sub);
			}

			if (rhs[0].op == operation_t::Negation)
			{
				add_constraint(rhs[0], neg_expr, sub);
			}
			else
			{
				add_constraint(rhs[0], expr, sub);
			}

			continue;
		}

		// case 5: left term is function
		if (lhs[0].type == term_t::Function)
		{
			if (rhs[0].type != term_t::Variable)
			{
				return false;
			}

			if (rhs[0].op == operation_t::Negation)
			{
				lhs.negation();
			}

			if (!add_constraint(rhs[0], lhs, sub))
			{
				return false;
			}

			continue;
		}

		// case 6: right term is function
		if (rhs[0].type == term_t::Function)
		{
			if (lhs[0].type != term_t::Variable)
			{
				return false;
			}

			if (lhs[0].op == operation_t::Negation)
			{
				rhs.negation();
			}

			if (!add_constraint(lhs[0], rhs, sub))
			{
				return false;
			}

			continue;
		}

		// something went wrong during unification?
		// this point is basically unreachable
		return false;
	}

	std::vector<std::vector<value_t>> adjacent(v - 1);
	for (const auto &[u, expr] : sub)
	{
		for (const auto &w : expr.variables())
		{
			adjacent[w - 1].push_back(u - 1);
		}
	}

	auto order = topological_sort(adjacent, v - 1);
	for (auto &variable : order)
	{
		variable = variable + 1;

		if (!sub.contains(variable))
		{
			continue;
		}

		auto &expr = sub.at(variable);
		if (expr[0].type != term_t::Function)
		{
			continue;
		}

		for (const auto &var : expr.variables())
		{
			if (!sub.contains(var))
			{
				continue;
			}

			auto replacement = sub.at(var);
			while (replacement[0].type == term_t::Variable &&
					sub.contains(replacement[0].value))
			{
				bool should_negate = replacement[0].op == operation_t::Negation;
				replacement = sub.at(replacement[0].value);
				if (should_negate)
				{
					replacement.negation();
				}
			}

			Term to_check(term_t::Variable, operation_t::Nop, var);
			if (replacement.contains(to_check))
			{
				return false;
			}

			expr.replace(var, replacement);
		}
	}

	substitution = std::move(sub);
	return true;
}


bool is_equal(Expression left, Expression right)
{
	// few O(1) checks
	if (left.size() != right.size())
	{
		return false;
	}

	if (left[0].op != right[0].op)
	{
		return false;
	}

	left.normalize();
	right.normalize();

	return left.equals(right);
}
