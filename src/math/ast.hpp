#ifndef AST_HPP
#define AST_HPP

#include <cstdint>
#include <ostream>
#include <algorithm>
#include <vector>
#include <array>
#include <string>


using value_t = std::int32_t;
constexpr const std::size_t INVALID_INDEX = static_cast<std::size_t>(-1);


enum class operation_t : std::int32_t
{
	Nop = 0,
	Negation,
	Implication,
	Disjunction,
	Conjunction,
	Xor,
	Equivalent
};


enum class term_t : std::int32_t
{
	None = 0,
	Constant,
	Variable,
	Function
};


std::int32_t priority(operation_t op);


/**
 * @brief determine whether operation_t is commutative or not
 */
inline bool is_commutative(operation_t operation_t)
{
	// operation_t: ?, !, >, |, *, +, =
	// commut.  : 0, 0, 0, 1, 1, 1, 1
	// index    : 0, 1, 2, 3, 4, 5, 6
	return static_cast<std::int32_t>(operation_t) > 2;
}


/**
 * @brief returns !(operation_t)
 * @note list:
 * Nop - Nop
 * Negation - Negation
 * Implication - Conjuction
 * Disjunction - Conjuction
 * Conjunction - Implication
 * Xor - Equivalent
 * Equivalent - Xor
 */
operation_t opposite(operation_t operation);


std::size_t increase_index(std::size_t index, std::size_t offset);
std::size_t decrease_index(std::size_t index, std::size_t offset);


struct Term
{
	term_t type;
	operation_t op;
	value_t value;

	Term(term_t type = term_t::None,
		operation_t op = operation_t::Nop,
		value_t value = 0) noexcept;


	std::string to_string() const noexcept;
	bool operator==(const Term &other) const noexcept;
};


struct Relation
{
	// references to self,left,right,parent
	std::array<std::size_t, 4> refs;

	Relation(std::size_t self = INVALID_INDEX,
		std::size_t left = INVALID_INDEX,
		std::size_t right = INVALID_INDEX,
		std::size_t parent = INVALID_INDEX) noexcept
		: refs{self, left, right, parent}
	{}

	inline std::size_t self() const noexcept { return refs[0]; }
	inline std::size_t left() const noexcept { return refs[1]; }
	inline std::size_t right() const noexcept { return refs[2]; }
	inline std::size_t parent() const noexcept { return refs[3]; }
};


class Expression
{
	struct Node
	{
		Term term;
		Relation rel;

		Node(const Term &term, const Relation &rel) noexcept
			: term(term), rel(rel)
		{}
	};

private:
	std::vector<Node> nodes_;
	std::string representation_;
	bool modified_ = true;

	inline bool in_range(std::size_t index) const noexcept
	{
		return index < nodes_.size();
	}

	void recalculate_representation() noexcept;
public:
	// construction
	Expression();
	Expression(std::string_view expression);
	Expression(Term term);
	Expression(const Expression &other);
	Expression(Expression &&other);
	Expression(const std::vector<Node> &nodes);
	Expression(std::vector<Node> &&nodes);
	Expression &operator=(const Expression &other);
	Expression &operator=(Expression &&other);


	// general information
	bool empty() const noexcept;
	std::size_t size() const noexcept;
	std::size_t operations(operation_t op) const noexcept;
	std::vector<value_t> variables() const noexcept;

	inline Term &operator[](std::size_t idx) { return nodes_[idx].term; }
 	inline const Term &operator[](std::size_t idx) const { return nodes_[idx].term; }
	std::string to_string() noexcept;

	// max variable value
	value_t max_value() const noexcept;

	// min variable value
	value_t min_value() const noexcept;

	// expression normalization
	void normalize() noexcept;
	void standardize() noexcept;
	void make_permanent() noexcept;

	// relation information
	Relation subtree(std::size_t idx) const noexcept;

	// deep copy of subtree
	Expression subtree_copy(std::size_t idx) const noexcept;

	// does certain subtree contains certain variable or constant?
	bool contains(Term term) const noexcept;
	bool has_left(std::size_t idx) const noexcept;
	bool has_right(std::size_t idx) const noexcept;


	// applying negation to certain subtree
	void negation(std::size_t idx = 0);

	// adjust all variables values to be at least bound
	void change_variables(value_t bound);

	// replace all occurrences of `value` to `expression
	Expression &replace(value_t value, const Expression &expression);


	// expression construction
	static Expression construct(
		const Expression &lhs,
		operation_t op,
		const Expression &rhs
	);

	// for priority queue store
	bool operator<(const Expression &other) const noexcept;

	// compare with other tree
	bool equals(const Expression &other, bool var_ignore = true) const noexcept;
};


std::ostream &operator<<(std::ostream &out, Expression &expression);

#endif // AST_HPP
