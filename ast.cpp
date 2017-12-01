#include "ast.h"
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include "compile_time.h"
#include "llvm/ADT/STLExtras.h"

using namespace std;

namespace al {
  namespace ast {
    int ASTNode::indent = 0;

    void StringLiteral::pre_visit(CompileTime &) {
      std::cout << std::string((uint32_t)this->indent, '\t')
                << "str<'" << this->s << "'>"
                << std::endl;
    }

    void ExpList::pre_visit(CompileTime &) {
      std::cout << std::string((uint32_t)this->indent, '\t')
                << "exp_list" << std::endl;
      this->incIndent();
    }

    void ExpList::post_visit(CompileTime &) {
      this->decIndent();
    }

    VisitResult ExpList::visit(CompileTime &rt) {
      pre_visit(rt);
      auto c = getChildren();
      if (c.empty()) {
        // TODO: should return llvm nullptr
        return nullptr;
      }
      auto symbol = dynamic_cast<Symbol*>(c[0].get());
      if (symbol == nullptr) {
        // not function call list, fail
        cerr << "Compile error, the first element in function call list must be a symbol" << endl;
        abort();
      }
      auto fnName = symbol->getValue();
      llvm::Function *callee = rt.getMainModule()->getFunction(fnName);
      if (!callee) {
        cerr << "Compile error, function '" << fnName << "' not defined" << endl;
        abort();
      }

      std::vector<llvm::Value *> ArgsV;
      for (unsigned i = 1; i < c.size(); ++i) {
        auto result = c[i]->visit(rt);
        ArgsV.push_back(result.value);
        if (!ArgsV.back())
          return nullptr;
      }

      post_visit(rt);
      VisitResult vr;
      vr.value = rt.getBuilder().CreateCall(callee, ArgsV);
      return vr;
    }

    void Symbol::pre_visit(CompileTime &) {
      std::cout << std::string((uint32_t)this->indent, '\t')
                << "sym<'" << this->s << "'>"
                << std::endl;
    }

    void List::pre_visit(CompileTime &) {
      std::cout << std::string((uint32_t)this->indent, '\t')
                << "list" << std::endl;
      incIndent();
    }

    void List::post_visit(CompileTime &) {
      decIndent();
    }
  }
}

