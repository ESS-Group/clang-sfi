#ifndef STMTHANDLER
#define STMTHANDLER 1
#include <sstream>
#include <string>
class FaultInjector;
// include "FaultInjector.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"

#include <iostream>
using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

using namespace std;

class StmtHandler : public MatchFinder::MatchCallback {
  public:
    StmtHandler(FaultInjector *pFaultInjector, std::string name, std::vector<std::string> bindings);
    virtual void run(const MatchFinder::MatchResult &Result);

  private:
    std::string fileName;
    std::vector<std::string> bindings;
    FaultInjector *faultInjector;
};

#endif