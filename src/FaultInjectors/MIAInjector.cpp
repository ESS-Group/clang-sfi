#include "_all.h"

std::string MIAInjector::toString() {
    return "MIA";
};
std::string SMIAInjector::toString() {
    return "SMIA";
};

MIAInjector::MIAInjector() { // Missing if construct around statements
    Matcher.addMatcher(ifStmt(unless(hasElse(stmt()))).bind("ifStmt"), createStmtHandler("ifStmt"));
}

bool MIAInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    const IfStmt *ifS = cast<IfStmt>(current.stmt);

    SourceRange range(ifS->getLocStart(), ifS->getThen()->getLocStart().getLocWithOffset(-1));
    R.RemoveText(range);
    return true;
}
bool MIAInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(stmt);
    return C9(ifS->getThen(), &Context);
}

bool SMIAInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(stmt);
    return C9(ifS->getThen(), &Context, false, 5, true); // && C2(ifS,Context);
}
