#include "SFIAction.h"

#include <iostream>

#include "clang/Frontend/CompilerInstance.h"

#include "SFIASTConsumer.h"

using namespace clang;
using namespace clang::ast_matchers;

SFIAction::SFIAction(std::vector<FaultInjector *> injs) : injectors(injs) {
}

void SFIAction::EndSourceFileAction() {
    std::cout << "Parsed file  " << fileName << std::endl;
}

std::unique_ptr<ASTConsumer> SFIAction::CreateASTConsumer(CompilerInstance &CI, StringRef file) {
    fileName = file.str();
    std::cout << "Parsing file '" << fileName << "'" << std::endl;

    // Do not print warnings:
    // CI.getDiagnostics().setClient(new IgnoringDiagConsumer());

    // Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<SFIASTConsumer>(fileName, injectors, &CI);
}
