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
  vector<Type *> params = vector<Type*>(3, getValuePtrType()); //name
  FunctionType *ft = FunctionType::get(getValuePtrType(), params, false);

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
  builder.CreateRet(createStringValuePtr("return"));
  verifyFunction(*func);
}

llvm::StructType *al::CompileTime::getStringType() const {
  return stringType;
}

void al::CompileTime::createPlaceHolderFunc(const std::string &name, int n) {
  vector<llvm::Type*> types(n, getValuePtrType());
  FunctionType *ft = FunctionType::get(getValuePtrType(), types, false);
  Function *func = Function::Create(ft, Function::ExternalLinkage, name, getMainModule());

  BasicBlock *BB = BasicBlock::Create(theContext, "entry", func);
  IRBuilder<> builder1(theContext);
  builder1.SetInsertPoint(BB);
  builder1.CreateRet(llvm::ConstantPointerNull::get(getValuePtrType()));
  verifyFunction(*func);
}

void al::CompileTime::createPrimitiveTypes() {
  // int len; char *data;
  /**
   * Create 'String' Type
   * */
  // Value type consists of
  //  String, Int
  valueType = StructType::create(
      theContext,
      "Value");
  valuePtrType = PointerType::get(valueType, 0);

  stringType = llvm::StructType::create(theContext, {
      llvm::Type::getInt32Ty(theContext),
      llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(theContext)),
  }, "String");

  arrayType = StructType::create(
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

          getStringType(),
          Type::getInt32Ty(theContext),
          arrayType,
      });

}

void al::CompileTime::init() {
  setupMainModule();
  createPrimitiveTypes();
  createFnFunc();

  createPlaceHolderFunc("statements", 3);
  createPlaceHolderFunc("puts", 1);
  createMainFunc();
}

void al::CompileTime::createMainFunc() {
  Function *func = Function::Create(
      FunctionType::get(Type::getVoidTy(theContext), {}),
      Function::ExternalLinkage,
      "main",
      getMainModule()
  );

  BasicBlock *BB = BasicBlock::Create(theContext, "entry", func);
  builder.SetInsertPoint(BB);
  pushCurrentBlock(BB);
  // main function starts
}

llvm::Value *al::CompileTime::createStringValuePtr(const std::string &s) {

  auto dataArray = ConstantDataArray::get(
      theContext,
      ArrayRef<uint8_t>((uint8_t*)s.c_str(), 100)
  );

  auto *alloc = new AllocaInst(getStringType(), 0);
  auto lenPtr = GetElementPtrInst::Create(
      getStringType(),
      alloc,
      {ConstantInt::get(Type::getInt32Ty(theContext), 0),
       ConstantInt::get(Type::getInt32Ty(theContext), 0)});
  auto dataPtr = GetElementPtrInst::Create(
      getStringType(),
      alloc,
      {ConstantInt::get(Type::getInt32Ty(theContext), 0),
       ConstantInt::get(Type::getInt32Ty(theContext), 1)});
  auto storeLen = new StoreInst(
      ConstantInt::get(Type::getInt32Ty(theContext), s.size()),
      lenPtr
  );
  auto *allocatedBytes = new AllocaInst(
      Type::getInt8Ty(theContext),
      0,
      ConstantInt::get(Type::getInt32Ty(theContext), s.size())
  );

  // global

  getMainModule()->getOrInsertGlobal(".str", ArrayType::get(Type::getInt8Ty(theContext), s.size()));
  GlobalVariable *gVar = getMainModule()->getNamedGlobal(".str");
  gVar->setLinkage(GlobalValue::CommonLinkage);
  gVar->setAlignment(4);

  auto storeData = new StoreInst(
      ConstantPointerNull::get(Type::getInt8PtrTy(theContext)),
      dataPtr
  );
//  auto bc = new BitCastInst(alloc, getConstantStringType());
  cout << alloc->getType()->getTypeID() << endl;
  cout << ((PointerType*)alloc->getType())->getElementType()->getTypeID() << endl;
  return alloc;
}

llvm::PointerType *al::CompileTime::getStringPtrType() const {
  return llvm::PointerType::get(stringType, 0);
}

llvm::PointerType *al::CompileTime::getValuePtrType() {
  return valuePtrType;
}

llvm::Value *al::CompileTime::castToValuePtr(llvm::Value *val) {
  val->dump();
  return BitCastInst::CreatePointerCast(val, getValuePtrType());
}
