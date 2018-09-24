#ifndef STMTHANDLER_H
#define STMTHANDLER_H 1

#include <iostream>

#include "clang/ASTMatchers/ASTMatchFinder.h"

class FaultInjector;
// include "FaultInjector.h"

using namespace clang;
using namespace clang::ast_matchers;

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