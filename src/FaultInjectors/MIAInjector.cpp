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

std::string MIAInjector::inject(StmtBinding current, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getThen()->getLocStart().getLocWithOffset(-1));
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIAInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(stmt);
    return C9(ifS->getThen(), &Context);
}

bool SMIAInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(stmt);
    return C9(ifS->getThen(), &Context, false, 5, true); // && C2(ifS,Context);
}
