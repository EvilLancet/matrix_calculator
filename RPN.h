#ifndef RPN_H_INCLUDED
#define RPN_H_INCLUDED

#include <vector>
#include "Token.h"
#include "Error.h"

void shuntingYard(const std::vector<Token> &expr, std::vector<Token> &outQueue);

double countRPN(const std::vector<Token> &expr);

#endif // RPN_H_INCLUDED
