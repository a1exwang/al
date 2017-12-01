//
// Created by alexwang on 11/24/17.
//

#ifndef AL_LEX_H
#define AL_LEX_H

#include <istream>
#include <memory>
#include <regex>
#include <re2/set.h>
#include "parser.tab.hpp"
#include <codecvt>
#include <locale>
#include "ast.h"
#include <string>


namespace al {
  class Lexer {
  public:
    explicit Lexer(const std::string &s) :s(s), input(s) { }

    std::string nextUtf8CodePoint() {
      char c = input[0];
      uint32_t cp_len = 1;
      if((c & 0xf8) == 0xf0) cp_len = 4;
      else if((c & 0xf0) == 0xe0) cp_len = 3;
      else if((c & 0xe0) == 0xc0) cp_len = 2;
      string ret = input.substr(0, cp_len).as_string();
      input.remove_prefix(cp_len);
      return ret;
    }

    bool parseQuoteString(std::string &str, std::string eos);

    Parser::symbol_type lex();
  private:
    std::string s;
    re2::StringPiece input;
  };


}

#endif //AL_LEX_H
