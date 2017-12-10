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
      vr.value = ct.castStringToValuePtr(ct.createStringValuePtr(getValue(), ct.getBuilder()));
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
            items.push_back(rt.castStringToValuePtr(
                rt.castStringToValuePtr(rt.createStringValuePtr(str->getValue(), rt.getBuilder()))));
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
      bool isNative = false;
      auto fnName = symbol->getValue();
      llvm::Function *callee = nullptr;

      if (fnName == "wtf") {
        vector<llvm::Type*> types(3, llvm::Type::getInt64Ty(rt.getContext()));
        auto ft = llvm::FunctionType::get(
            llvm::Type::getVoidTy(rt.getContext()),
            types,
            true
        );
        callee = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "AL__callFunction", rt.getMainModule());

        auto strValuePtr = rt.createStringValuePtr(fnName, rt.getBuilder());
        auto nameStrValue = rt.castStringToValuePtr(strValuePtr);

        std::vector<llvm::Value *> ArgsV;
        // void AL__callFunction(uint64_t _prt, uint64_t _pname, uint64_t nargs, ...);
        ArgsV.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(rt.getContext()), (uint64_t)&rt));
        ArgsV.push_back(rt.getBuilder().CreatePtrToInt(nameStrValue, llvm::Type::getInt64Ty(rt.getContext())));
        ArgsV.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(rt.getContext()), (uint64_t)(c.size() - 1)));

        for (unsigned i = 1; i < c.size(); ++i) {
          auto result = c[i]->visit(rt);
          if (result.value) {
            ArgsV.push_back(rt.getBuilder().CreatePointerCast(result.value, rt.getValuePtrType()));
          }
          else {
            ArgsV.push_back(rt.getBuilder().CreatePointerCast(
                rt.castStringToValuePtr(rt.createStringValuePtr("wtf", rt.getBuilder())), rt.getValuePtrType())
            );
          }
        }

        post_visit(rt);

        VisitResult vr;
        for (int i = 1; i < ArgsV.size(); ++i)
          ArgsV[i] = rt.getBuilder().CreatePtrToInt(ArgsV[i], llvm::Type::getInt64Ty(rt.getContext()));

        vr.value = rt.getBuilder().CreateCall(callee, ArgsV);
        return vr;
      }
      else {
        callee = rt.getMainModule()->getFunction(fnName);
        if (callee == nullptr) {
          auto nativeFnName = "AL__" + fnName;
          uint32_t argc = c.size() - 1;
          vector<llvm::Type*> ps;
          ps.push_back(llvm::Type::getInt64Ty(rt.getContext()));
          for (int i = 0; i < argc; ++i) {
            ps.push_back(llvm::Type::getInt64Ty(rt.getContext()));
          }
          auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(rt.getContext()), ps, false);
          callee = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, nativeFnName, rt.getMainModule());
//        callee = rt.getMainModule()->getFunction();
          isNative = true;
        }
      }
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
      if (isNative) {
        ArgsV.insert(ArgsV.begin(), llvm::ConstantInt::get(llvm::Type::getInt64Ty(rt.getContext()), (uint64_t)&rt));
        for (int i = 1; i < ArgsV.size(); ++i)
          ArgsV[i] = rt.getBuilder().CreatePtrToInt(ArgsV[i], llvm::Type::getInt64Ty(rt.getContext()));
      }


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

