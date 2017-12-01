#pragma once
#include <memory>
#include <utility>
#include <llvm/IR/LLVMContext.h>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"


namespace al {
  class Value {
    uint8_t type;
    llvm::Value *value;

  };
  namespace ast {
    class ASTNode;
  }
  class CompileTime {
  public:
    CompileTime();

    void setASTRoot(std::shared_ptr<ast::ASTNode> root) {
      this->root = std::move(root);
    }
    void createFunction(llvm::Module &module, const std::string &name, std::vector<std::string> paramNames);
    void setupMainModule();
    void createFnFunc();
    void createPlaceHolderFunc(const std::string &name, int n);
    void traverse1();
    llvm::BasicBlock* createFunctionBody(llvm::Module &module, const std::string &name);
    llvm::Module* getMainModule() const;

    llvm::StructType *getStringType() const;
    llvm::StructType *getValueType() const { return valueType; }
    llvm::StructType *getArrayType() const { return arrayType; }

    void pushCurrentBlock(llvm::BasicBlock *b) { this->currentBlocks.push_back(b); }
    void popCurrentBlock() { this->currentBlocks.pop_back(); }
    llvm::BasicBlock *getCurrentBlock() const { return *(this->currentBlocks.end() - 1); }

    llvm::IRBuilder<> &getBuilder() { return builder; }

    llvm::LLVMContext &getContext() { return theContext; }

  private:
    llvm::StructType *valueType;
    llvm::StructType *stringType;
    llvm::StructType *arrayType;


    std::shared_ptr<ast::ASTNode> root;

    llvm::LLVMContext theContext;
    std::unique_ptr<llvm::Module> mainModule;

    std::vector<llvm::BasicBlock*> currentBlocks;
    llvm::IRBuilder<> builder;
  };
}