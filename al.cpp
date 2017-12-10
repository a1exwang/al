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

#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>
#include <cstdarg>

using namespace llvm;
using namespace std;

#ifdef LLVM_ON_WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" {
DLLEXPORT void callMeDaddy() {
  cout << "daddy" << endl;
}

ExecutionEngine *EE;
llvm::Module *mainModule;

DLLEXPORT void AL__callFunction(uint64_t _prt, uint64_t _pname, uint64_t nargs, ...) {
  auto rt = reinterpret_cast<al::CompileTime*>(_prt);
  auto name = reinterpret_cast<al::Value *>(_pname);
  vector<uint64_t> params;
  va_list args;
  va_start(args, nargs);
  for (int i = 0; i < nargs; ++i) {
    auto arg = va_arg(args, uint64_t);
    params.push_back(arg);
  }
  va_end(args);

  string fnName((const char*)name->sVal.data, name->sVal.len);
  fnName = "AL__" + fnName;

  decltype(rt->symbolTable.find(fnName)) wtf;
  if ((wtf = rt->symbolTable.find(fnName)) == rt->symbolTable.end()) {
    cerr << "Cannot find function '" << fnName << "'" << endl;
    assert(false);
  }
  auto mod = wtf->second;
  auto func = mod->getFunction(fnName);

  EE->addModule(move(unique_ptr<llvm::Module>(mod)));
  vector<GenericValue> gvs;
  for (auto p : params) {
    gvs.push_back(GenericValue((llvm::Value*)p));
  }
  EE->runFunction(func, gvs);
}

DLLEXPORT void AL__insertFunction(uint64_t _rt, uint64_t _name) {
  auto rt = reinterpret_cast<al::CompileTime*>(_rt);
  auto name = &reinterpret_cast<al::Value *>(_name)->sVal;
  string fnName((const char*)name->data, name->len);
  fnName = "AL__" + fnName;

  FunctionType *ft = FunctionType::get(rt->getValuePtrType(), {}, false);
  if (rt->symbolTable.find(fnName) != rt->symbolTable.end()) {
    cerr << "Function '" << fnName << "' exists" << endl;
    assert(false);
  }
  auto mod = new llvm::Module(fnName, rt->getContext());

  rt->symbolTable[fnName] = mod;
  Function *func = Function::Create(ft, Function::ExternalLinkage, fnName, mod);

  BasicBlock *BB = BasicBlock::Create(rt->getContext(), "entry", func);
  IRBuilder<> builder1(rt->getContext());
  builder1.SetInsertPoint(BB);

  // function body
  builder1.CreateCall(
      Function::Create(
          FunctionType::get(
              Type::getInt32Ty(rt->getContext()),
              {Type::getInt32Ty(rt->getContext())},
              false),
          Function::ExternalLinkage,
          "putchar",
          mod
      ),
      {ConstantInt::get(Type::getInt32Ty(rt->getContext()), 'x')}
  );
  builder1.CreateRet(ConstantPointerNull::get(rt->getValuePtrType()));

  verifyFunction(*func);

}

}



int main() {
  // Initialize interpreter
  LLVMLinkInMCJIT();
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();


  al::CompileTime rt;

  ifstream ifs("/home/alexwang/dev/proj/cpp/al/hello.al");
  std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

  al::Lexer lexer(str);
  al::Parser parser(lexer, rt);
  parser.parse();

  rt.init();
  rt.traverse1();

  rt.finish();

  std::error_code ec;
  string s;
  raw_string_ostream s1(s);
//  rt.getMainModule()->print(errs(), nullptr);
  rt.getMainModule()->print(s1, nullptr);
  mainModule = rt.getMainModule();
  ofstream fs("test.ll");
  fs << s;

  // start interpreter

  auto mainFunc = rt.getMainFunc();
  auto mainModule = rt.moveMainModule();
  EngineBuilder eb(move(mainModule));
  EE = eb
      .setEngineKind(EngineKind::JIT)
      .create();

  GenericValue gv = EE->runFunction(mainFunc, {});
  gv.AggregateVal[0];

  outs() << "Result: " << gv.IntVal << "\n";
  delete EE;
  llvm_shutdown();
  return 0;
}
