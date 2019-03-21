#include "_all.h"

std::string MIEBInjector::toString() {
    return "MIEB";
};
std::string SMIEBInjector::toString() {
    return "SMIEB";
};

MIEBInjector::MIEBInjector() { // Missing if construct plus statements + Else
                               // before statements
    Matcher.addMatcher(ifStmt(hasElse(stmt())).bind("ifStmt"), createMatchHandler("ifStmt"));
}

bool MIEBInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    const IfStmt *ifS = cast<IfStmt>(current.stmt);

    SourceRange range(ifS->getLocStart(), ifS->getElse()->getLocStart().getLocWithOffset(-1));
    R.RemoveText(range);
    return true;
}
bool MIEBInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(stmt);
    // IF block should contain less than 5 statements.
    return C9(ifS->getThen(), &Context);
    // ELSE block may contain more than 5 statements.
}

bool SMIEBInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(stmt);
    return C9(ifS->getThen(), &Context, false, 5, true);
}
