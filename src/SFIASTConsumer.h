#ifndef SFIASTCONSUMER_H
#define SFIASTCONSUMER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"

#include "FaultInjector.h"

using namespace clang;

class SFIASTConsumer : public ASTConsumer {
  public:
    SFIASTConsumer(std::string name, std::vector<std::unique_ptr<FaultInjector>> injectors, CompilerInstance *CI);
    void HandleTranslationUnit(ASTContext &Context) override;

  private:
    std::string fileName;
    std::vector<std::unique_ptr<FaultInjector>> faultInjectors;
};

#endif
