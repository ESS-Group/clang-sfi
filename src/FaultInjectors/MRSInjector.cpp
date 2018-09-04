#include "_all.h"

std::string MRSInjector::toString() {
    return "MRS";
};

MRSInjector::MRSInjector() { // Missing if construct plus statements plus else
                             // plus satements
    Matcher.addMatcher(returnStmt().bind("returnStmt"), createStmtHandler("returnStmt"));
}

std::string MRSInjector::inject(StmtBinding current, ASTContext &Context) {
    const Stmt *stmt = current.stmt;
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(stmt->getLocStart(), stmt->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MRSInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) { // no else
    return true;
}
