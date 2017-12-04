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

using namespace llvm;
using namespace std;


int main() {
  // Initialize interpreter
  InitializeNativeTarget();
  LLVMLinkInInterpreter();


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
  ofstream fs("test.ll");
  fs << s;

  // start interpreter

  auto mainFunc = rt.getMainFunc();
  auto mainModule = rt.moveMainModule();
  auto *m = mainModule.get();
  EngineBuilder eb(move(mainModule));
  ExecutionEngine* EE = eb
      .setEngineKind(EngineKind::Interpreter)
      .create();
  GenericValue gv = EE->runFunction(mainFunc, {});

  outs() << "Result: " << gv.IntVal << "\n";
  delete EE;
  llvm_shutdown();
  return 0;
}
