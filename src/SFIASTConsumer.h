#ifndef SFIASTCONSUMER_H
#define SFIASTCONSUMER_H

#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"

#include "FaultInjector.h"

using namespace clang;

class SFIASTConsumer : public ASTConsumer {
  public:
    SFIASTConsumer(std::string name, std::vector<FaultInjector *> injectors);
    void HandleTranslationUnit(ASTContext &Context) override;

  private:
    std::string fileName;
    std::vector<FaultInjector *> faultInjectors;
};

class SFIAction : public ASTFrontendAction {
  public:
    std::vector<FaultInjector *> injectors;
    SFIAction(std::vector<FaultInjector *> injs);
    void EndSourceFileAction() override;
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override;

  private:
    std::string fileName;
};

#endif
