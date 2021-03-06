#pragma once
#include <memory>
#include <utility>
#include <llvm/IR/LLVMContext.h>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include <map>


namespace al {
  struct Value;
  struct StringValue {
    uint32_t len;
    uint8_t *data;
  };
  struct ArrayValue {
    uint32_t n;
    Value *pVal;
  };
  enum ValueType {
    Integer = 0,
    String,
    Array
  };
  struct Value {
    uint32_t type;

    StringValue sVal;
    uint32_t iVal;
    ArrayValue aVal;
  };
  namespace ast {
    class ASTNode;
  }
  class CompileTime {
  public:
    CompileTime();
    ~CompileTime();

    void setASTRoot(std::shared_ptr<ast::ASTNode> root) {
      this->root = std::move(root);
    }
    void init();
    void finish();
    void createFunction(llvm::Module &module, const std::string &name, std::vector<std::string> paramNames);
    void setupMainModule();
    void createFnFunc();
    void createMainFunc();
    void createLibFunc();
    void createPrimitiveTypes();
    void createPlaceHolderFunc(const std::string &name, int n);
    void createPutsFunc();
    void traverse1();
    llvm::BasicBlock* createFunctionBody(llvm::Module &module, const std::string &name);
    llvm::Module* getMainModule() const;

    llvm::Value *createStringValuePtr(const std::string &s, llvm::IRBuilder<> &builder);
    llvm::Value *castStringToValuePtr(llvm::Value *);

    llvm::PointerType *getStringPtrType() const;
    llvm::StructType *getStringType() const;
    llvm::StructType *getValueType() const { return valueType; }
    llvm::PointerType *getValuePtrType();
    llvm::StructType *getArrayType() const { return arrayType; }
    std::unique_ptr<llvm::Module> &&moveMainModule() { return std::move(mainModule); }

    void pushCurrentBlock(llvm::BasicBlock *b) { this->currentBlocks.push_back(b); }
    void popCurrentBlock() { this->currentBlocks.pop_back(); }
    llvm::BasicBlock *getCurrentBlock() const { return *(this->currentBlocks.end() - 1); }
    llvm::Function *getMainFunc() const { return mainFunction; }

    llvm::IRBuilder<> &getBuilder() { return builder; }

    llvm::LLVMContext &getContext() { return theContext; }

//  private:
  public:
    std::string nextConstVarName();

    struct {
      llvm::Function *printf;
    } fns;
    llvm::Function *mainFunction;

    llvm::StructType *valueType;
    llvm::PointerType *valuePtrType;
    llvm::StructType *stringType;
    llvm::StructType *arrayType;

    std::shared_ptr<ast::ASTNode> root;

    llvm::LLVMContext theContext;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> mainModule;

    std::vector<llvm::BasicBlock*> currentBlocks;
    int strCounter;

    std::map<std::string, llvm::Module*> symbolTable;
  };
}