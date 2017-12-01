#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <iterator>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constants.h>


namespace al {
  class CompileTime;
  namespace ast {

    struct VisitResult {
      VisitResult() :value() { }
      VisitResult(const nullptr_t &nptr) :value(nullptr) {}
      llvm::Value *value;
    };

    // Visitor design pattern
    class ASTNode {
    public:
      ASTNode() :vr() { }
      virtual void pre_visit(CompileTime &rt) { }
      virtual void post_visit(CompileTime &rt) { }
      virtual VisitResult gen_visit_result(CompileTime &ct) { return vr; }
      virtual VisitResult visit(CompileTime &rt) {
        pre_visit(rt);
        for (auto &child : getChildren()) {
          child->visit(rt);
        }
        post_visit(rt);
        return gen_visit_result(rt);
      }
      virtual std::vector<std::shared_ptr<ASTNode>> getChildren() {
        return children;
      }
      VisitResult getVR() const { return vr; }

    protected:
      void appendChild(const std::shared_ptr<ASTNode> &node) {
        this->children.push_back(node);
      }
      void prependChild(const std::shared_ptr<ASTNode> &node) {
        this->children.insert(this->children.begin(), node);
      }

      VisitResult vr;

      static int indent;
      static void incIndent() { indent++; }
      static void decIndent() {
        indent--;
        if (indent < 0)
          indent = 0;
      }
    private:
      std::vector<std::shared_ptr<ASTNode>> children;
    };

    class Exp :public ASTNode {
    };

    class ExpList :public ASTNode {
    public:
      std::shared_ptr<ExpList> append(std::shared_ptr<Exp> node) {
        auto ret = std::make_shared<ExpList>(*this);
        ret->appendChild(std::move(node));
        return ret;
      }
      std::shared_ptr<ExpList> prepend(std::shared_ptr<Exp> node) {
        auto ret = std::make_shared<ExpList>(*this);
        ret->prependChild(std::move(node));
        return ret;
      }
      VisitResult visit(CompileTime &) override;

      void pre_visit(CompileTime &) override;
      void post_visit(CompileTime &) override;
    };

    class List :public Exp {
    public:
      explicit List(const std::shared_ptr<ExpList> &p) : list(p) {
        this->appendChild(p);
      }
      void pre_visit(CompileTime &) override;
      void post_visit(CompileTime &) override;
      VisitResult gen_visit_result(CompileTime &) override {
        return getChildren()[0]->getVR();
      }

    private:
      std::shared_ptr<ExpList> list;
    };

    class Symbol :public Exp {
    public:
      explicit Symbol(std::string s): s(std::move(s)) { }
      std::string getValue() const {
        return this->s;
      }

      void pre_visit(CompileTime &) override;

    private:
      std::string s;
    };


    class Literal :public Exp {
    };

    class StringLiteral :public Literal {
    public:
      explicit StringLiteral(std::string s): s(std::move(s)) { }
      std::string getValue() const {
        return this->s;
      }

      void pre_visit(CompileTime &) override;
      VisitResult gen_visit_result(CompileTime &ct) override;

    private:
      std::string s;
    };
  }
}