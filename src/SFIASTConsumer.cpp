#include "SFIASTConsumer.h"

#include <iostream>

using namespace clang;

SFIASTConsumer::SFIASTConsumer(std::string name, std::vector<FaultInjector *> injectors)
    : faultInjectors(injectors), fileName(name) {
    for (FaultInjector *injector : faultInjectors) {
        injector->setFileName(name);
    }
}

void SFIASTConsumer::HandleTranslationUnit(ASTContext &Context) {
    for (FaultInjector *injector : faultInjectors) {
        injector->matchAST(Context); // Match AST and find injection locations

        std::cout << "Found " << injector->locations.size() << " " << injector->toString() << " injection locations"
                  << std::endl;

        injector->inject(injector->locations, Context);                   // inject faults
        for (FaultInjector::StmtBinding &binding : injector->locations) { // only for verbose
            binding.calculateRange(Context);
        }
    }
}
