#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include <vector>
#include "Token.h"

enum State
{
    S0, // Стартовое
    S1, // Токенизация скобки/оператора
    S2, // Запись целого числа в буфер
    S3, // Запись floating-point числа в буфер
    S4, // Запись функции в буфер
    S5  // Токенизация записанного числа/функции из буфера
};


void tokenize(const std::string &expr, std::vector<Token> &tokens);

#endif // TOKENIZER_H_INCLUDED
