#ifndef TOKEN_H
#define TOKEN_H

#include "Constants.h"

#include <string>

using namespace std;

class Token
{
public:
    Token(TokenId id, const string &lexeme, int position)
      : id(id), lexeme(lexeme), position(position) { }

    TokenId getId() const { return id; }
    const string &getLexeme() const { return lexeme; }
    int getPosition() const { return position; }

private:
    TokenId id;
    string lexeme;
    int position;
};

#endif
