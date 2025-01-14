#ifndef RULES_HPP
#define RULES_HPP

#include "ast.hpp"


// 2 variables
/**
 * @brief a, a > b ⊢ b
 */
Expression modus_ponens(const Expression &a, const Expression &b);

/**
 * @brief a > b, !b ⊢ !a
 */
Expression modus_tollens(const Expression &a, const Expression &b);

/**
 * @brief !a, a | b ⊢ b
 */
Expression disjunctive_syllogism(const Expression &a, const Expression &b);

/**
 * @brief a, a xor b ⊢ !b
 */
Expression separating_syllogism(const Expression &a, const Expression &b);

/**
 * @brief a, a xor b ⊢ !b
 */
Expression contradiction_rule(const Expression &a, const Expression &b);

// 3 variables

/**
 * @brief a > b, b > c ⊢ a > c
 */
Expression hypothetical_syllogism(const Expression &a, const Expression &b, const Expression &c);

/**
 * @brief a > c, b > c, a | b ⊢ c
 */
Expression simple_constructive_dilemma(const Expression &a, const Expression &b, const Expression &c);

/**
 * @brief a > c, a > b, !b | !c ⊢ !a
 */
Expression simple_destructive_dilemma(const Expression &a, const Expression &b, const Expression &c);

// 4 variables
/**
 * @brief a > c, b > d, a | b ⊢ c | d
 */
Expression complex_constructive_dilemma(const Expression &a, const Expression &b, const Expression &c, const Expression &d);

/**
 * @brief a > c, b > d, !c | !d ⊢ !a | !b
 */
Expression complex_destructive_dilemma(const Expression &a, const Expression &b, const Expression &c, const Expression &d);


#endif // RULES_HPP
