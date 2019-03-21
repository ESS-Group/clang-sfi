#include "_all.h"

std::string MIFSInjector::toString() {
    return "MIFS";
};
std::string SMIFSInjector::toString() {
    return "SMIFS";
};

MIFSInjector::MIFSInjector() { // Missing if construct plus statements
    Matcher.addMatcher(ifStmt(unless(hasElse(stmt()))).bind("ifStmt"), createMatchHandler("ifStmt"));
}

bool MIFSInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    const IfStmt *ifS = cast<IfStmt>(current.stmt);

    SourceRange range(ifS->getLocStart(), ifS->getLocEnd());
    R.RemoveText(range);
    return true;
}
bool MIFSInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(stmt);
    // THEN block should contain less than 5 statements.
    if (!C9(ifS->getThen(), &Context)) {
        return false;
    }
    // IF statement should not be the only statement in the block.
    return C2(stmt, Context);
}

bool SMIFSInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const IfStmt *ifS = cast<IfStmt>(stmt);
    if (!C9(ifS->getThen(), &Context, false, 5, true)) {
        return false;
    }
    return C2(stmt, Context);
}
