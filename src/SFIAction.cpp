#include "ASTConsumer.h"

#include <sstream>
#include <string>
#include <algorithm>

//include "StmtHandler.h"
class FaultInjector;
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


SFIAction::SFIAction(std::vector<FaultInjector*> injs):injectors(injs){}
void SFIAction::EndSourceFileAction() /*override*/ {
    //Rewrite.getEditBuffer(Rewrite.getSourceMgr().getMainFileID()).write(llvm::outs());//an dieser Stelle durch änderungen durchiterieren (mit begin() den iterator erhalten)
    //cout<<locations.size();
    cout<<"done";
    //cout<<"testitest"<<endl<<countIf<<":"<<countElse<<endl;
}
std::unique_ptr<ASTConsumer> SFIAction::CreateASTConsumer( CompilerInstance &CI,
                                                StringRef file){
    //llvm::errs() << "createing ASTConsumer for "<<file<<"\n";
    //sm = CI.getSourceManager();
    //lo = CI.getLangOpts();
    cout << "Parsing file '" << file.data() << "'" << endl;

    CI.getDiagnostics().setClient(new IgnoringDiagConsumer());//keine warnings ausgeben

    //Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<SFIASTConsumer>(/*Rewrite,*/ file, injectors);
}
