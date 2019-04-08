#include "SFIAction.h"

#include <iostream>

#include "clang/Frontend/CompilerInstance.h"

#include "SFIASTConsumer.h"

using namespace clang;
using namespace clang::ast_matchers;

SFIAction::SFIAction(std::vector<std::string> pinjectornames, FaultInjectorOptions &pfiOpt)
    : injectornames(pinjectornames), fiOpt(pfiOpt) {
}

void SFIAction::EndSourceFileAction() {
    std::cout << "Parsed file  " << fileName << std::endl;
}

std::unique_ptr<ASTConsumer> SFIAction::CreateASTConsumer(CompilerInstance &CI, StringRef file) {
    fileName = file.str();
    std::cout << "Parsing file '" << fileName << "'" << std::endl;

    // Do not print warnings:
    // CI.getDiagnostics().setClient(new IgnoringDiagConsumer());

    std::vector<std::unique_ptr<FaultInjector>> injectors;
    for (std::string name : injectornames) {
        auto fi = FaultInjector::create(name);
        fi->setVerbose(fiOpt.verbose);
        fi->setPatchDirectory(fiOpt.patchDir);
        fi->setRootDir(fiOpt.rootDir);
        fi->setFileList(fiOpt.fileList);
        fi->setFileName(fileName);
        fi->setMatchMacro(fiOpt.matchMacro);
        injectors.push_back(std::move(fi));
    }
    return llvm::make_unique<SFIASTConsumer>(fileName, std::move(injectors), &CI);
}
