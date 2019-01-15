#include "_all.h"

std::string MIEBInjector::toString() {
    return "MIEB";
};
std::string SMIEBInjector::toString() {
    return "SMIEB";
};

MIEBInjector::MIEBInjector() { // Missing if construct plus statements + Else
                               // before statements
    Matcher.addMatcher(ifStmt(hasElse(stmt())).bind("ifStmt"), createStmtHandler("ifStmt"));
}

std::string MIEBInjector::inject(StmtBinding current, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getElse()->getLocStart().getLocWithOffset(-1));
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIEBInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(stmt);
    return C9(ifS->getThen(), &Context);
    // Else block may contain more than 5 statements.
}

bool SMIEBInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(stmt);
    return C9(ifS->getThen(), &Context, false, 5, true);
}
