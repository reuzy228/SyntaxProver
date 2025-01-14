#include <algorithm>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <stack>
#include <queue>
#include <functional>
#include <string>
#include <numeric>
#include "ast.hpp"
#include "../parser/parser.hpp"


const auto operation_dict = [] () -> std::unordered_map<operation_t, std::string>
{
	return std::unordered_map<operation_t, std::string>{
		{operation_t::Nop, "Nop"},
		{operation_t::Negation, "!"},
		{operation_t::Disjunction, "|"},
		{operation_t::Conjunction, "*"},
		{operation_t::Implication, ">"},
		{operation_t::Xor, "+"},
		{operation_t::Equivalent, "="}
	};
}();


const auto opposite_operation = [] () -> std::unordered_map<operation_t, operation_t>
{
	return std::unordered_map<operation_t, operation_t>{
		{operation_t::Nop, operation_t::Nop},
		{operation_t::Negation, operation_t::Negation},
		{operation_t::Disjunction, operation_t::Conjunction},
		{operation_t::Conjunction, operation_t::Implication},
		{operation_t::Implication, operation_t::Conjunction},
		{operation_t::Xor, operation_t::Equivalent},
		{operation_t::Equivalent, operation_t::Xor}
	};
}();


operation_t opposite(operation_t operation_t)
{
	return opposite_operation.at(operation_t);
}


std::size_t increase_index(std::size_t index, std::size_t offset)
{
	return index == INVALID_INDEX ? INVALID_INDEX : index + offset;
}


std::size_t decrease_index(std::size_t index, std::size_t offset)
{
	return index == INVALID_INDEX || offset > index ? INVALID_INDEX : index - offset;
}


Term::Term(term_t type, operation_t op, value_t value) noexcept
	: type(type)
	, op(op)
	, value(value)
{}


std::string Term::to_string() const noexcept
{
	std::stringstream representation;

	if (type == term_t::None)
	{
		representation << "None";
		return representation.str();
	}

	if (type == term_t::Function)
	{
		representation << operation_dict.at(op);
	}
	else
	{
		if (op == operation_t::Negation)
		{
			representation << '!';
		}

		if (type == term_t::Constant)
		{
			char suitable_constant = std::abs(value) - 1 + 'a';
			representation << suitable_constant;
		}
		else
		{
			char suitable_constant = std::abs(value) - 1 + 'A';
			representation << suitable_constant;
		}
	}

	return representation.str();
}


bool Term::operator==(const Term &other) const noexcept
{
	return type == other.type && op == other.op && value == other.value;
}


Expression::Expression() = default;


Expression::Expression(std::string_view expression)
{
	nodes_ = std::move(ExpressionParser(expression).parse().nodes_);
	modified_ = true;
}


Expression::Expression(Term term)
{
	nodes_.emplace_back(
		term,
		Relation(0)
	);

	modified_ = true;
}


Expression::Expression(const Expression &other)
	: nodes_(other.nodes_)
	, representation_(other.representation_)
	, modified_(true)
{}


Expression::Expression(Expression &&other)
	: nodes_(std::move(other.nodes_))
	, representation_(std::move(other.representation_))
	, modified_(true)
{}


Expression::Expression(const std::vector<Node> &nodes)
	: nodes_(nodes)
{
	modified_ = true;
}


Expression::Expression(std::vector<Node> &&nodes)
	: nodes_(std::move(nodes))
{
	modified_ = true;
}


Expression &Expression::operator=(const Expression &other)
{
	if (this == &other)
	{
		return *this;
	}

	nodes_ = other.nodes_;
	modified_ = true;
	return *this;
}


Expression &Expression::operator=(Expression &&other)
{
	if (this == &other)
	{
		return *this;
	}

	nodes_ = std::move(other.nodes_);
	modified_ = true;
	return *this;
}


std::size_t Expression::size() const noexcept
{
	return nodes_.size();
}


std::size_t Expression::operations(operation_t op) const noexcept
{
	std::size_t count = 0;

	for (const auto &node : nodes_)
	{
		if (node.term.type != term_t::Function)
		{
			continue;
		}

		if (node.term.op == op)
		{
			++count;
		}
	}

	return count;
}


bool Expression::empty() const noexcept
{
	return nodes_.empty();
}


std::vector<value_t> Expression::variables() const noexcept
{
	std::vector<value_t> vars;
	vars.reserve(nodes_.size());

	for (const auto &node : nodes_)
	{
		if (node.term.type == term_t::Variable)
		{
			vars.push_back(node.term.value);
		}
	}

	return vars;
}


void Expression::recalculate_representation() noexcept
{
	if (empty())
	{
		representation_ = "empty";
		modified_ = false;
		return;
	}

	std::stringstream ss;
	std::function<void(std::ostream &, Relation, const Expression &)> f =
	[&] (std::ostream &out, Relation root, const Expression &expression)
	{
		if (root.self() == INVALID_INDEX)
		{
			return;
		}

		const bool brackets = root.parent() != INVALID_INDEX &&
			expression[root.self()].type == term_t::Function;
		if (brackets)
		{
			out << "(";
		}

		f(out, subtree(root.left()), expression);
		out << expression[root.self()].to_string();
		f(out, subtree(root.right()), expression);

		if (brackets)
		{
			out << ")";
		}

		return;
	};

	f(ss, subtree(0), *this);
	representation_ = std::move(ss.str());
	modified_ = false;
}


std::string Expression::to_string() noexcept
{
	if (modified_)
	{
		recalculate_representation();
	}

	return representation_;
}


value_t Expression::max_value() const noexcept
{
	value_t value = 0;

	// find max value in variables
	for (const auto &node : nodes_)
	{
		if (node.term.type == term_t::Variable)
		{
			value = std::max(value, node.term.value);
		}
	}

	return value;
}


value_t Expression::min_value() const noexcept
{
	value_t min_value = std::numeric_limits<value_t>::max();

	// find min value in variables
	for (const auto &node : nodes_)
	{
		if (node.term.type == term_t::Variable)
		{
			min_value = std::min(min_value, node.term.value);
		}
	}

	return min_value;
}


void Expression::normalize() noexcept
{
	std::vector<value_t> order;
	std::unordered_map<value_t, value_t> remapping;

	std::function<void(Relation)> traverse =
	[&] (Relation node)
	{
		if (node.self() == INVALID_INDEX)
		{
			return;
		}

		traverse(subtree(node.left()));

		if (nodes_[node.self()].term.type == term_t::Variable)
		{
			order.push_back(nodes_[node.self()].term.value);
		}

		traverse(subtree(node.right()));
	};
	traverse(subtree(0));

	value_t new_value = 1;
	for (const auto &entry : order)
	{
		if (remapping.contains(entry))
		{
			continue;
		}

		remapping[entry] = new_value;
		++new_value;
	}

	for (auto &node : nodes_)
	{
		if (node.term.type != term_t::Variable)
		{
			continue;
		}

		node.term.value = remapping[node.term.value];
	}

	modified_ = true;
}


void Expression::standardize() noexcept
{
	// bfs traverse
	std::queue<std::size_t> q;
	q.push(0);

	while (!q.empty())
	{
		const auto node_idx = q.front();
		q.pop();

		// skip invalid nodes
		if (node_idx == INVALID_INDEX)
		{
			continue;
		}

		if (nodes_[node_idx].term.type != term_t::Function)
		{
			continue;
		}

		if (nodes_[node_idx].term.op == operation_t::Disjunction)
		{
			nodes_[node_idx].term.op = operation_t::Implication;
			negation(subtree(node_idx).left());
		}

		// continue standartization if required
		if (has_left(node_idx))
		{
			q.push(subtree(node_idx).left());
		}
		if (has_right(node_idx))
		{
			q.push(subtree(node_idx).right());
		}
	}

	modified_ = true;
}


void Expression::make_permanent() noexcept
{
	for (auto &node : nodes_)
	{
		if (node.term.type == term_t::Variable)
		{
			node.term.type = term_t::Constant;
		}
	}

	modified_ = true;
}


Relation Expression::subtree(std::size_t idx) const noexcept
{
	return in_range(idx) ? nodes_[idx].rel : Relation{};
}


Expression Expression::subtree_copy(std::size_t idx) const noexcept
{
	std::size_t new_root_index = subtree(idx).self();
	std::vector<Node> nodes;
	std::unordered_map<std::size_t, std::size_t> remapping;
	std::size_t i = 0;

	std::function<void(Relation)> traverse =
	[&] (Relation node)
	{
		if (node.self() == INVALID_INDEX)
		{
			return;
		}

		// append new node
		nodes.push_back(nodes_[node.self()]);
		remapping[node.self()] = i++;

		traverse(subtree(node.left()));
		traverse(subtree(node.right()));
	};

	// traverse to collect values of subtree
	traverse(subtree(new_root_index));

	// break relation with old parent
	nodes[0].rel.refs[3] = INVALID_INDEX;

	// update indices
	for (auto &node : nodes)
	{
		for (auto &ref : node.rel.refs)
		{
			if (!remapping.contains(ref))
			{
				continue;
			}

			ref = remapping.at(ref);
		}
	}

	return Expression{std::move(nodes)};
}


bool Expression::contains(Term term) const noexcept
{
	// since we check single `term`, then function is not possible
	if (term.type != term_t::Variable && term.type != term_t::Constant)
	{
		return false;
	}

	for (const auto &node : nodes_)
	{
		if (node.term.type != term_t::Variable &&
			node.term.type != term_t::Constant)
		{
			continue;
		}

		if (node.term.value == term.value)
		{
			return true;
		}
	}

	return false;
}


bool Expression::has_left(std::size_t idx) const noexcept
{
	return !in_range(idx) ? false : in_range(nodes_[idx].rel.left());
}


bool Expression::has_right(std::size_t idx) const noexcept
{
	return !in_range(idx) ? false : in_range(nodes_[idx].rel.right());
}


void Expression::negation(std::size_t idx)
{
	if (!in_range(idx))
	{
		return;
	}

	// bfs traverse
	std::queue<std::size_t> q;
	q.push(idx);

	while (!q.empty())
	{
		const auto node_idx = q.front();
		q.pop();

		// skip invalid nodes
		if (node_idx == INVALID_INDEX)
		{
			continue;
		}

		if (nodes_[node_idx].term.type != term_t::Function)
		{
			nodes_[node_idx].term.op =
				nodes_[node_idx].term.op == operation_t::Negation ?
				operation_t::Nop :
				operation_t::Negation;

			continue;
		}

		// inverse operation_t
		nodes_[node_idx].term.op = opposite(nodes_[node_idx].term.op);

		// continue negation if required
		if (nodes_[node_idx].term.op == operation_t::Implication ||
			nodes_[node_idx].term.op == operation_t::Conjunction)
		{
			q.push(subtree(node_idx).right());
		}
		else if (nodes_[node_idx].term.op == operation_t::Disjunction)
		{
			q.push(subtree(node_idx).left());
			q.push(subtree(node_idx).right());
		}
	}

	modified_ = true;
}


void Expression::change_variables(value_t bound)
{
	// adjust variables to be at least bound
	bound -= min_value();
	for (auto &node : nodes_)
	{
		if (node.term.type == term_t::Variable)
		{
			node.term.value += bound;
		}
	}

	modified_ = true;
}


Expression &Expression::replace(value_t value, const Expression &expression)
{
	if (expression.empty())
	{
		return *this;
	}

	std::vector<std::size_t> indices;
	Expression new_expr = expression;
	Expression new_expr_neg = new_expr;
	new_expr_neg.negation();

	value_t appropriate_value = 0;

	// find all places
	for (const auto &node : nodes_)
	{
		if (node.term.type == term_t::Variable &&
			node.term.value == value)
		{
			indices.push_back(node.rel.self());
			appropriate_value = std::max(
				std::abs(appropriate_value),
				node.term.value
			);
		}
	}

	// nothing to replace
	if (indices.empty())
	{
		return *this;
	}

	// don't forget that other variables are different and should not intersect with ours
	auto offset = size();
	appropriate_value = appropriate_value + 1;
	for (const auto &entry : indices)
	{
		auto replacement =
			nodes_[entry].term.op == operation_t::Negation ?
			new_expr_neg :
			new_expr;

		nodes_[entry] = Node{
			replacement.nodes_[0].term,
			Relation{
				nodes_[entry].rel.refs[0],
				increase_index(
					replacement.subtree(0).left(),
					offset - 1
				),
				increase_index(
					replacement.subtree(0).right(),
					offset - 1
				),
				nodes_[entry].rel.refs[3]
			}
		};

		for (std::size_t i = 1; i < replacement.nodes_.size(); ++i)
		{
			nodes_.push_back(replacement.nodes_[i]);
			//nodes_.back().term.value += appropriate_value;

			for (auto &ref : nodes_.back().rel.refs)
			{
				ref = increase_index(ref, offset - 1);
			}
		}

		// update relations for subroot nodes
		if (subtree(entry).left() != INVALID_INDEX)
		{
			nodes_[subtree(entry).left()].rel.refs[3] = entry;
		}
		if (subtree(entry).right() != INVALID_INDEX)
		{
			nodes_[subtree(entry).right()].rel.refs[3] = entry;
		}

		// mode offset
		offset = nodes_.size();
	}

	modified_ = true;
	return *this;
}


Expression Expression::construct(
	const Expression &lhs,
	operation_t op,
	const Expression &rhs
)
{
	Expression expression;
	std::size_t offset = 1;

	expression.nodes_.emplace_back(
		Term(term_t::Function, op),
		Relation(0, 1, 1 + lhs.size())
	);

	for (const auto &node : lhs.nodes_)
	{
		expression.nodes_.push_back(node);

		for (auto &ref : expression.nodes_.back().rel.refs)
		{
			ref = increase_index(ref, offset);
		}

		if (expression.nodes_.back().rel.refs[3] == INVALID_INDEX)
		{
			expression.nodes_.back().rel.refs[3] = 0;
		}
	}

	offset += lhs.size();
	for (const auto &node : rhs.nodes_)
	{
		expression.nodes_.push_back(node);

		for (auto &ref : expression.nodes_.back().rel.refs)
		{
			ref = increase_index(ref, offset);
		}

		if (expression.nodes_.back().rel.refs[3] == INVALID_INDEX)
		{
			expression.nodes_.back().rel.refs[3] = 0;
		}
	}

	expression.modified_ = true;
	return expression;
}

bool Expression::operator<(const Expression &other) const noexcept
{
	return size() > other.size();
}


bool Expression::equals(const Expression &other, bool var_ignore) const noexcept
{
	if (size() != other.size())
	{
		return false;
	}

	for (std::size_t i = 0; i < size(); ++i)
	{
		if ((nodes_[i].term.type == term_t::Function) !=
			(other.nodes_[i].term.type == term_t::Function))
		{
			return false;
		}

		if (nodes_[i].term.type == term_t::Function &&
			nodes_[i].term.op != other.nodes_[i].term.op)
		{
			return false;
		}

		if (!var_ignore &&
			(nodes_[i].term.type != other.nodes_[i].term.type))
		{
			return false;
		}

		if (nodes_[i].term.value != other.nodes_[i].term.value ||
			nodes_[i].term.op != other.nodes_[i].term.op)
		{
			return false;
		}
	}

	return true;
}


std::ostream &operator<<(std::ostream &out, Expression &expression)
{
	return out << expression.to_string();
}
