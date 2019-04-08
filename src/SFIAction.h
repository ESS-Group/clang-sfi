#ifndef SFIASTACTION_H
#define SFIASTACTION_H

#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"

#include "FaultInjector.h"

#include <string>

using namespace clang;

class SFIAction : public ASTFrontendAction {
  public:
    SFIAction(std::vector<std::string> pinjectornames, FaultInjectorOptions &pfiOpt);
    void EndSourceFileAction() override;
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override;

  private:
    std::string fileName;
    std::vector<std::string> injectornames;
    FaultInjectorOptions &fiOpt;
};

#endif
