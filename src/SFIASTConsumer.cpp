#include "SFIASTConsumer.h"

#include <iostream>

using namespace clang;

SFIASTConsumer::SFIASTConsumer(std::string name, std::vector<std::unique_ptr<FaultInjector>> injectors, CompilerInstance *CI)
    : fileName(name) {
    for (auto &injector : injectors) {
        injector->setFileName(name);
        injector->setCI(CI);
        faultInjectors.push_back(std::move(injector));
    }
}

void SFIASTConsumer::HandleTranslationUnit(ASTContext &Context) {
    for (auto &injector : faultInjectors) {
        injector->matchAST(Context); // Match AST and find injection locations

        std::cout << "Found " << injector->locations.size() << " " << injector->toString() << " injection locations"
                  << std::endl;

        injector->inject(injector->locations, Context);
        // calculateRanges for verbose output.
        for (FaultInjector::StmtBinding &binding : injector->locations) {
            binding.calculateRange(Context);
        }
    }
}
