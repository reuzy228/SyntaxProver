#ifndef HELPER_HPP
#define HELPER_HPP

#include <unordered_map>
#include "ast.hpp"


bool add_constraint(
	Term term,
	Expression substitution,
	std::unordered_map<value_t, Expression> &sub
);


/**
 * @brief Performs unification between two expressions,
 * producing a substitution if possible.
 *
 * @param left The left-hand side expression.
 * @param right The right-hand side expression.
 * @param substitution A reference to a map where the resulting substitution will be stored.
 *
 * @note `right` is unified to `left`
 *
 * @return Returns `true` if unification was successful, `false` otherwise.
 */
bool unification(
	Expression left,
	Expression right,
	std::unordered_map<value_t, Expression> &substitution
);


/**
 * @brief Check if left and right expressions are the same
 *
 * @note unification of variables is allowed
 *
 * @param left The left-hand side expression.
 * @param right The right-hand side expression.
 *
 * @return Returns `true` if expressions are equal and `false` otherwise.
 */
bool is_equal(Expression left, Expression right);

#endif // HELPER_HPP
