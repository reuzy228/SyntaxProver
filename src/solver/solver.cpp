#include <queue>
#include <ranges>
#include <chrono>
#include <iostream>
#include <set>
#include <queue>
#include "solver.hpp"
#include "../math/helper.hpp"
#include "../math/rules.hpp"


std::uint64_t ms_since_epoch()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}


Solver::Solver(std::vector<Expression> axioms,
		Expression target,
		std::uint64_t time_limit_ms
) 	: known_axioms_()
	, axioms_(std::move(axioms))
	, produced_()
	, targets_()
	, time_limit_(time_limit_ms)
	, ss{}
	, dump_("conclusions.txt")
{
	if (axioms_.size() < 3)
	{
		throw std::invalid_argument("[-] error: at least 3 axioms are required");
	}

	targets_.emplace_back(std::move(target));
	axioms_.reserve(1000);
	known_axioms_.reserve(10000);

	// produce hack: implication swap rule (a->b) ~ (!b->!a)
	axioms = {
		Expression("a>(b>a)"),
		Expression("(a>(b>c))>((a>b)>(a>c))"),
		Expression("(!a>!b)>((!a>b)>a)")
	};

	axioms.emplace_back(modus_ponens(axioms[0], axioms[0]));
	axioms.emplace_back(modus_ponens(axioms[1], axioms[0]));
	axioms.emplace_back(modus_ponens(axioms[3], axioms[1]));
	axioms.emplace_back(modus_ponens(axioms[4], axioms[1]));
	axioms.emplace_back(modus_ponens(axioms[2], axioms[5]));
	axioms.emplace_back(modus_ponens(axioms[6], axioms[6]));
	axioms.emplace_back(modus_ponens(axioms[7], axioms[8]));
	axioms.emplace_back(modus_ponens(axioms[3], axioms[9]));

	dump_ << axioms[3] << ' ' << "mp" << ' ' << axioms[0] << ' ' << axioms[0] << '\n';
	dump_ << axioms[4] << ' ' << "mp" << ' ' << axioms[1] << ' ' << axioms[0] << '\n';
	dump_ << axioms[5] << ' ' << "mp" << ' ' << axioms[3] << ' ' << axioms[1] << '\n';
	dump_ << axioms[6] << ' ' << "mp" << ' ' << axioms[4] << ' ' << axioms[1] << '\n';
	dump_ << axioms[7] << ' ' << "mp" << ' ' << axioms[2] << ' ' << axioms[5] << '\n';
	dump_ << axioms[8] << ' ' << "mp" << ' ' << axioms[6] << ' ' << axioms[5] << '\n';
	dump_ << axioms[9] << ' ' << "mp" << ' ' << axioms[7] << ' ' << axioms[8] << '\n';
	dump_ << axioms[10] << ' ' << "mp" << ' ' << axioms[3] << ' ' << axioms[9] << '\n';
}


bool Solver::is_target_proved_by(const Expression &expression) const
{
	if (expression.empty())
	{
		return false;
	}

	for (const auto &target : targets_)
	{
		if (is_equal(target, expression))
		{
			return true;
		}
	}

	return false;
}


bool Solver::is_good_expression(const Expression &expression, std::size_t max_len) const
{
	return !(expression.size() > max_len || expression.empty() ||
		expression[0].op == operation_t::Conjunction ||
		expression.operations(operation_t::Conjunction) > 1);
}


bool Solver::deduction_theorem_decomposition(Expression expression)
{
	if (expression.empty())
	{
		return false;
	}

	if (expression[0].op != operation_t::Implication)
	{
		return false;
	}

	// Γ ⊢ A → B <=> Γ U {A} ⊢ B
	axioms_.emplace_back(expression.subtree_copy(expression.subtree(0).left()));
	targets_.emplace_back(expression.subtree_copy(expression.subtree(0).right()));
	return true;
}


void Solver::produce(std::size_t max_len)
{
	if (produced_.empty())
	{
		return;
	}

	std::vector<Expression> newly_produced;
	newly_produced.reserve(2 * produced_.size());

	Expression expr;
	std::string repr;

	for (auto &expression : produced_)
	{
		if (ms_since_epoch() > time_limit_)
		{
			break;
		}

		if (expression.size() > max_len)
		{
			continue;
		}

		// add expression
		expression.normalize();
		axioms_.push_back(expression);

		if (is_target_proved_by(axioms_.back()))
		{
			return;
		}

		// produce new expressions
		for (std::size_t j = 0; j < axioms_.size(); ++j)
		{
			expr = std::move(modus_ponens(axioms_[j], axioms_.back()));

			if (!is_good_expression(expr, max_len) ||
				known_axioms_.contains(expr.to_string()))
			{
				continue;
			}

			newly_produced.emplace_back(expr);
			known_axioms_.insert(newly_produced.back().to_string());

			dump_ << newly_produced.back() << ' ' << "mp" << ' '
			<< axioms_[j] << ' ' << axioms_.back() << '\n';

			if (is_target_proved_by(newly_produced.back()))
			{
				axioms_.push_back(newly_produced.back());
				return;
			}

			if (j + 1 == axioms_.size())
			{
				break;
			}

			// inverse order
			expr = std::move(modus_ponens(axioms_.back(), axioms_[j]));

			if (!is_good_expression(expr, max_len) ||
				known_axioms_.contains(expr.to_string()))
			{
				continue;
			}

			newly_produced.emplace_back(expr);
			known_axioms_.insert(newly_produced.back().to_string());

			dump_ << newly_produced.back() << ' ' << "mp" << ' '
			<< axioms_.back() << ' ' << axioms_[j] << '\n';

			if (is_target_proved_by(newly_produced.back()))
			{
				axioms_.push_back(newly_produced.back());
				return;
			}
		}
	}

	if (ms_since_epoch() > time_limit_)
	{
		return;
	}

	std::ranges::sort(newly_produced, [] (const auto &lhs, const auto &rhs) {
		return lhs.size() < rhs.size();
	});

	produced_ = std::move(newly_produced);
}


void Solver::solve()
{
	ss.clear();

	// we will produce at most m operations in one expression
	// if more operations are required, increase it
	std::size_t len = 20;

	// simplify target if it's possible
	while (deduction_theorem_decomposition(targets_.back()))
	{
		auto &prev = targets_[targets_.size() - 2];
		auto &curr = targets_.back();
		auto &axiom = axioms_.back();

		ss << "deduction theorem: " << "Γ ⊢ " << prev << " <=> "
		<< "Γ U {" << axiom << "} ⊢ " << curr << '\n';
	}

	// write all axioms to produced array
	for (std::size_t i = 0; i < axioms_.size(); ++i)
	{
		axioms_[i].normalize();
		produced_.push_back(axioms_[i]);
		dump_ << axioms_[i] << ' ' << "axiom" << '\n';
	}

	// isr rule
	produced_.emplace_back(Expression("(!a>!b)>(b>a)"));
	axioms_.clear();
	known_axioms_.clear();

	// calculating the stopping criterion
	const auto time = ms_since_epoch();
	time_limit_ =
		time > std::numeric_limits<std::uint64_t>::max() - time_limit_ ?
		std::numeric_limits<std::uint64_t>::max() :
		time + time_limit_;

	while (ms_since_epoch() < time_limit_)
	{
		produce(len);

		if (is_target_proved_by(axioms_.back()))
		{
			break;
		}
	}

	if (std::ranges::none_of(axioms_, [&] (const auto &expression) {
		return is_target_proved_by(expression);
	}))
	{
		ss << "No proof was found in the time allotted\n";
		return;
	}

	// find which target was proved
	Expression proof;
	Expression target_proved;

	for (const auto &axiom : axioms_)
	{
		if (!proof.empty())
		{
			break;
		}

		for (const auto &target : targets_)
		{
			if (is_equal(target, axiom))
			{
				proof = axiom;
				target_proved = target;
				break;
			}
		}
	}

	// build proof chain
	dump_.flush();
	build_thought_chain(proof, target_proved);
}


void Solver::build_thought_chain(Expression proof, Expression proved_target)
{
	std::ifstream conclusions("conclusions.txt");
	std::unordered_map<std::string, Node> conclusions_;
	std::unordered_map<std::string, std::size_t> indices;
	std::unordered_set<std::string> processed_proofs;
	std::unordered_map<std::size_t, Node> chain;
	std::size_t next_index = 1;

	std::string line;
	std::string expression;
	std::string rule;
	std::string dependency;

	while (std::getline(conclusions, line))
	{
		std::istringstream tss(line);
		tss >> expression >> rule;

		if (conclusions_.contains(expression))
		{
			continue;
		}

		conclusions_[expression] = {expression, rule};
		while (tss >> dependency)
		{
			conclusions_[expression].dependencies.push_back(dependency);
		}
	}

	std::vector<std::vector<std::string>> tree_levels;
	tree_levels.push_back({proof.to_string()});

	while (!tree_levels.back().empty())
	{
		std::vector<std::string> level;
		for (const auto &expression : tree_levels.back())
		{
			auto &node = conclusions_[expression];
			if (processed_proofs.contains(node.expression))
			{
				continue;
			}

			if (node.rule == "axiom")
			{
				if (indices.contains(node.expression))
				{
					continue;
				}

				chain[next_index] = node;
				indices[node.expression] = next_index;
				++next_index;
			}

			for (const auto &dependency : node.dependencies)
			{
				level.push_back(dependency);
			}

			processed_proofs.insert(node.expression);
		}

		tree_levels.emplace_back(level);
	}

	std::ranges::reverse(tree_levels);

	for (const auto &level : tree_levels)
	{
		for (const auto &expression : level)
		{
			if (indices.contains(expression))
			{
				continue;
			}

			chain[next_index] = conclusions_[expression];
			indices[expression] = next_index;
			++next_index;
		}
	}

	for (std::size_t i = 1; i < next_index; ++i)
	{
		const auto &node = chain[i];

		ss << i << ". ";

		if (node.rule == "axiom")
		{
			ss << "axiom";
		}
		else
		{
			ss << node.rule << "(";

			for (std::size_t k = 0; k < node.dependencies.size(); ++k)
			{
				ss << indices[node.dependencies[k]];

				if (k + 1 != node.dependencies.size())
				{
					ss << ",";
				}
			}

			ss << ")";
		}

		ss << ": " << node.expression << '\n';
	}

	// change variables if required
	std::unordered_map<value_t, Expression> substitution;
	unification(proved_target, proof, substitution);

	if (substitution.empty())
	{
		return;
	}

	ss << "change variables: " << proof << "\n";
	for (auto &[v, s] : substitution)
	{
		ss << (char)(v + 'A' - 1) << " -> " << s << '\n';
	}

	ss << "proved: " << proved_target << '\n';
}


std::string Solver::thought_chain() const
{
	return ss.str();
}
