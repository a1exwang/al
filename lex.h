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
  class Token {

  };
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

    bool parseQuoteString(std::string &str, std::string eos) {
      string content;
      bool escaping = false;
      while (input.size() > 0) {
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
      }
      str = content;
      return false;
    }

    Parser::symbol_type lex() {
      string regs[] = {
          "\\s+",
          "\\(",
          "\\)",
          "'",
          "\\w+",
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

            auto p = std::shared_ptr<ast::StringLiteral>(new ast::StringLiteral(result));
            return Parser::make_STRING(p, Parser::location_type());
          },
          [](const std::string &s) -> Parser::symbol_type {
            auto p = std::shared_ptr<ast::Symbol>(new ast::Symbol(s));
            return Parser::make_SYMBOL(p, Parser::location_type());
          },
          [](const std::string &s) -> Parser::symbol_type {
            return Parser::make_INT(s, Parser::location_type());
          },
      };
      if (input.size() == 0) {
        return al::Parser::make_END(Parser::location_type());
      }

      int i = 0;
      for (auto reg: regs) {
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
  private:
    std::string s;
    re2::StringPiece input;
  };


}

#endif //AL_LEX_H
