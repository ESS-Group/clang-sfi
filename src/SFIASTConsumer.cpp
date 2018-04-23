#include "ASTConsumer.h"

#include <sstream>
#include <string>
#include <algorithm>

//include "StmtHandler.h"
#include "FaultInjector.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"

#include <iostream>

using namespace clang;
using namespace clang::ast_matchers;
using namespace std;


SFIASTConsumer::SFIASTConsumer(std::string name, std::vector<FaultInjector*> injectors ): faultInjectors(injectors), fileName(name){
    for(FaultInjector *injector: faultInjectors){
        injector->setFileName(name);    
    }
}

void SFIASTConsumer::HandleTranslationUnit(ASTContext &Context){
    for(FaultInjector *injector: faultInjectors){

        injector->matchAST(Context);//Match AST and find injection locations

        cout << "Found " << injector->locations.size() << " " <<  injector->toString() << " injection locations" << endl;

        injector->inject(injector->locations, Context);//inject Faults
        for(FaultInjector::StmtBinding& binding: injector->locations)//only for verbose
            binding.calculateRange(Context);
        
    }
}