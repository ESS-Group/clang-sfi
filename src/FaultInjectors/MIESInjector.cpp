#include "_all.h"

std::string MIESInjector::toString() {
    return "MIES";
};

MIESInjector::MIESInjector() { // Missing if construct plus statements plus else
                               // plus statements
    Matcher.addMatcher(ifStmt(hasElse(stmt())).bind("ifStmt"), createStmtHandler("ifStmt"));
}

std::string MIESInjector::inject(StmtBinding current, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getElse()->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIESInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(stmt);
    // IF and ELSE block should not contain more than 5 statements.
    if (!C9(ifS->getThen())) {
        return false;
    }
    if (!C9(ifS->getElse())) {
        return false;
    }
    // IF statement should not be the only statement in the block.
    return C2(stmt, Context);
}
