#include "compile_time.h"
#include "ast.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "compile_time.h"

using namespace llvm;
using namespace std;

void al::CompileTime::traverse1() {
  this->root->visit(*this);
}

void al::CompileTime::createFunction(Module &module, const string &name, vector<string> paramNames) {
  IRBuilder<> builder(theContext);
  // Declare function
  vector<Type *> params(paramNames.size(), Type::getDoubleTy(theContext));
  FunctionType *ft =
      FunctionType::get(Type::getDoubleTy(theContext), params, false);
  Function *func = Function::Create(ft, Function::ExternalLinkage, name, &module);

  int i = 0;
  for (auto &arg : func->args()) {
    arg.setName(paramNames[i]);
    i++;
  }
}

void al::CompileTime::setupMainModule() {
  mainModule = llvm::make_unique<llvm::Module>("main", theContext);
  createFnFunc();
}

al::CompileTime::CompileTime() :theContext(), builder(theContext) {
}

llvm::BasicBlock *al::CompileTime::createFunctionBody(llvm::Module &module, const std::string &name) {
  Function *fn = module.getFunction(name);
  return BasicBlock::Create(theContext, "__entry", fn);
}

llvm::Module* al::CompileTime::getMainModule() const { return mainModule.get(); }

void al::CompileTime::createFnFunc() {
  IRBuilder<> builder(theContext);
  // Declare function
  // int len; char *data;
  /**
   * Create 'String' Type
   * */
  auto stringType = StructType::create(theContext, {
      Type::getInt32Ty(theContext),
      Type::getInt8PtrTy(theContext),
  }, "String");
  // Value type consists of
  //  String, Int
  auto valueType = StructType::create(
      theContext,
      "Value");

  auto arrayType = StructType::create(
      theContext,
      {
          Type::getInt32Ty(theContext), // length,
          PointerType::getUnqual(valueType)
      },
      "Array"
  );
  valueType->setBody(
      {
          Type::getInt8PtrTy(theContext), // object type

          stringType,
          Type::getInt32Ty(theContext),
          arrayType,
      });
  vector<Type *> params = vector<Type*>(3, valueType); //name
  FunctionType *ft = FunctionType::get(valueType, params, false);

  Function *func = Function::Create(ft, Function::ExternalLinkage, "fn", getMainModule());

  string argNames[] = {"name", "args", "statements"};

  int i = 0;
  for (auto &arg : func->args()) {
    if (i < 2) {
      arg.setName(argNames[i]);
    }
    i++;
  }

  BasicBlock *BB = BasicBlock::Create(theContext, "entry", func);
  builder.SetInsertPoint(BB);
  builder.CreateRet(llvm::ConstantInt::get(Type::getInt32Ty(theContext), 0));
  verifyFunction(*func);
}
