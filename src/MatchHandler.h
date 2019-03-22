#ifndef MATCHHANDLER_H
#define MATCHHANDLER_H 1

#include <iostream>

#include "clang/ASTMatchers/ASTMatchFinder.h"

class FaultInjector;
// include "FaultInjector.h"

using namespace clang;
using namespace clang::ast_matchers;

class MatchHandler : public MatchFinder::MatchCallback {
  public:
    MatchHandler(FaultInjector *pFaultInjector, std::string fileName, std::vector<std::string> bindings);
    virtual void run(const MatchFinder::MatchResult &Result);
    template <typename SD>
    void run_stmt_or_decl(const MatchFinder::MatchResult &Result, SourceManager &SM, std::string binding, SD &stmtOrDecl);

  private:
    std::string fileName;
    std::vector<std::string> bindings;
    FaultInjector *faultInjector;
};

#endif