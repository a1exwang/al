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

    VisitResult StringLiteral::gen_visit_result(CompileTime &ct) {
      VisitResult vr;
//        vr.value = createStringTypeObject(this->s);
      vr.value = ct.createStringValuePtr("null", ct.getBuilder());
      return vr;
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
        // not function call list, maybe value array, string array
        vector<llvm::Value*> items;
        for (const auto &item : c) {
          auto str = dynamic_cast<StringLiteral*>(item.get());
          if (str != nullptr) {
            items.push_back(rt.createStringValuePtr(str->getValue(), rt.getBuilder()));
          }
        }
        VisitResult vr;
        vr.value = llvm::ConstantPointerNull::get(rt.getValuePtrType());
//        vr.value = llvm::Array::get(arr, items);
        return vr;
//        return llvm::ConstantArray::get(arr, {});
//        cerr << "Compile error, the first element in function call list must be a symbol" << endl;
//        abort();
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
        if (result.value) {
          ArgsV.push_back(rt.getBuilder().CreatePointerCast(result.value, rt.getValuePtrType()));
        }
        else {
          ArgsV.push_back(rt.getBuilder().CreatePointerCast(
              rt.createStringValuePtr("wtf", rt.getBuilder()),
              rt.getValuePtrType()));
        }
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

