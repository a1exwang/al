#pragma once
#include <memory>

namespace al {
  namespace ast {
    class ASTNode;
  }
  class Runtime {
  public:
    void setASTRoot(std::shared_ptr<ast::ASTNode> root) {
      this->root = root;
    }
  private:
    std::shared_ptr<ast::ASTNode> root;
  };
}