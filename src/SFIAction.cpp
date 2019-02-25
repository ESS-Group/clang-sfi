#include "SFIASTConsumer.h"

#include <iostream>

using namespace clang;
using namespace clang::ast_matchers;

SFIAction::SFIAction(std::vector<FaultInjector *> injs) : injectors(injs) {
}
void SFIAction::EndSourceFileAction() {
    std::cout << "Parsed file  " << fileName << std::endl;
}
std::unique_ptr<ASTConsumer> SFIAction::CreateASTConsumer(CompilerInstance &CI, StringRef file) {
    fileName = file.data();
    std::cout << "Parsing file '" << fileName << "'" << std::endl;

    // Do not print warnings:
    // CI.getDiagnostics().setClient(new IgnoringDiagConsumer());

    // Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<SFIASTConsumer>(file, injectors);
}
