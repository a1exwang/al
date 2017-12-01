#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "parser.tab.hpp"
#include "lex.h"
#include "compile_time.h"

using namespace llvm;
using namespace std;


static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule = llvm::make_unique<Module>("test", TheContext);
static std::map<std::string, Value *> NamedValues;

int maina() {

  // Declare function
  vector<Type *> Doubles(2, Type::getDoubleTy(TheContext));
  FunctionType *FT =
      FunctionType::get(Type::getDoubleTy(TheContext), Doubles, false);
  Function *F = Function::Create(FT, Function::ExternalLinkage, "Add", TheModule.get());
  std::string names[] = {"p1", "p2"};
  int i = 0;
  for (auto &Arg : F->args()) {
    Arg.setName(names[i]);
    i++;
  }

  // Define function

  Function *TheFunction = TheModule->getFunction("Add");

  // Create a new basic block to start insertion into.
  BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
  Builder.SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  NamedValues.clear();

    // Finish off the function.
  auto ret = ConstantFP::get(TheContext, APFloat(0.1));
  Builder.CreateRet(ret);

  // Validate the generated code, checking for consistency.
  verifyFunction(*TheFunction);



  // Call  fn
  vector<Type *> vd(0);
  FunctionType *FT1 = FunctionType::get(Type::getInt32Ty(TheContext), vd, false);
  Function *fmain = Function::Create(FT1, Function::ExternalLinkage, "main", TheModule.get());
  Function *fmainf = TheModule->getFunction("main");
  BasicBlock *BBf = BasicBlock::Create(TheContext, "entry1", fmainf);
  Builder.SetInsertPoint(BBf);

  Function *CalleeF = TheModule->getFunction("Add");

  std::vector<Value *> ArgsV;
  ArgsV.push_back(ConstantFP::get(TheContext, APFloat(0.5)));
  ArgsV.push_back(ConstantFP::get(TheContext, APFloat(0.5)));

  Builder.CreateCall(CalleeF, ArgsV, "calltmp");

  verifyFunction(*fmain);

  TheModule->print(errs(), nullptr);

  return 0;
}

int main() {
  al::CompileTime rt;

  ifstream ifs("/home/alexwang/dev/proj/cpp/al/hello.al");
  std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

  al::Lexer lexer(str);
  al::Parser parser(lexer, rt);
  parser.parse();

  rt.setupMainModule();
  rt.traverse1();

  rt.getMainModule()->print(errs(), nullptr);

  return 0;
}
