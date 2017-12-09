//
// Created by alexwang on 11/24/17.
//

#include "lex.h"

bool al::Lexer::parseQuoteString(std::string &str, std::string eos) {
  string content;
  bool escaping = false;
  while (!input.empty()) {
    string cp = nextUtf8CodePoint();
    if (escaping) {
      escaping = false;
      if (cp == "n") {
        content += "\n";
      }
      else {
        content += cp;
      }
      continue;
    }
    if (cp == "\\") {
      escaping = true;
      continue;
    }
    if (cp == eos) {
      str = content;
      return true;
    }
    content += cp;
  }
  str = content;
  return false;
}

al::Parser::symbol_type al::Lexer::lex() {
  string regs[] = {
      "\\s+",
      "\\(",
      R"(\))",
      "'",
      R"(\w(\w|\d|[-_+=?!@#$%^&*])*)",
      "\\d+",
  };
  std::function<Parser::symbol_type (const std::string &s)> fns[] = {
      nullptr,
      [](const std::string &s) -> Parser::symbol_type {
        return Parser::make_LEFTPAR(Parser::location_type());
      },
      [](const std::string &s) -> Parser::symbol_type {
        return Parser::make_RIGHTPAR(Parser::location_type());
      },
      [this](const std::string &s) -> Parser::symbol_type {
        std::string result;
        if (!this->parseQuoteString(result, "'"))
          throw "failed to parse quote string";

        auto p = std::make_shared<ast::StringLiteral>(result);
        return Parser::make_STRING(p, Parser::location_type());
      },
      [](const std::string &s) -> Parser::symbol_type {
        auto p = std::make_shared<ast::Symbol>(s);
        return Parser::make_SYMBOL(p, Parser::location_type());
      },
      [](const std::string &s) -> Parser::symbol_type {
        return Parser::make_INT(s, Parser::location_type());
      },
  };
  if (input.empty()) {
    return al::Parser::make_END(Parser::location_type());
  }

  int i = 0;
  for (const auto &reg: regs) {
    string var;
    int value;
    if (RE2::Consume(&input, "(" + reg + ")", &var)) {
      // FIXME: i == 0 for blank characters
      if (i != 0)
        return fns[i](var);
    }
    i++;
  }

  throw "wtf";
}
