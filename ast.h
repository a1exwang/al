#pragma once

#include <memory>
#include <string>
#include <vector>


namespace al {
  namespace ast {
    class ASTNode {

    };

    class Exp :public ASTNode {

    };

    class ExpList :public ASTNode {
    public:
      std::shared_ptr<ExpList> append(std::shared_ptr<Exp> node) {
        auto ret = std::shared_ptr<ExpList>(new ExpList(*this));
        ret->_append(node);
        return ret;
      }
    private:
      void _append(std::shared_ptr<Exp> node) {
        this->nodes.push_back(node);
      }
    private:
      std::vector<std::shared_ptr<Exp>> nodes;

    };

    class List :public Exp {
    public:
      List(std::shared_ptr<ExpList> p) : list(p) { }

    private:
      std::shared_ptr<ExpList> list;
    };

    class Symbol :public Exp {
    public:
      Symbol(const std::string &s): s(s) { }
      std::string getValue() const {
        return this->s;
      }

    private:
      std::string s;
    };


    class Literal :public Exp {

    };

    class StringLiteral :public Literal {
    public:
      StringLiteral(const std::string &s): s(s) { }
      std::string getValue() const {
        return this->s;
      }

    private:
      std::string s;
    };

  }

}