#include "ASTConsumer.h"

#include <algorithm>
#include <sstream>
#include <string>

// include "StmtHandler.h"
class FaultInjector;
#include "FaultInjector.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"

#include <iostream>

using namespace clang;
using namespace clang::ast_matchers;
using namespace std;

SFIAction::SFIAction(std::vector<FaultInjector *> injs) : injectors(injs) {
}
void SFIAction::EndSourceFileAction() {
    cout << "Parsed file  " << fileName /*<<" - done."*/ << endl;
}
std::unique_ptr<ASTConsumer> SFIAction::CreateASTConsumer(CompilerInstance &CI, StringRef file) {
    fileName = file.data();
    cout << "Parsing file '" << fileName << "'" << endl;

    // CI.getDiagnostics().setClient(new IgnoringDiagConsumer());//keine
    // warnings ausgeben

    // Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<SFIASTConsumer>(file, injectors);
}
