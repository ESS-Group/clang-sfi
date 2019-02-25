#include "_all.h"

std::string MRSInjector::toString() {
    return "MRS";
};

MRSInjector::MRSInjector() { // Missing if construct plus statements plus else
                             // plus statements
    Matcher.addMatcher(returnStmt().bind("returnStmt"), createStmtHandler("returnStmt"));
}

bool MRSInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    const Stmt *stmt = current.stmt;

    SourceRange range(stmt->getLocStart(), stmt->getLocEnd());
    R.RemoveText(range);
    return true;
}
bool MRSInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    return true;
}
