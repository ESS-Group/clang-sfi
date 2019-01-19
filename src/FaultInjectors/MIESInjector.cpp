#include "_all.h"

std::string MIESInjector::toString() {
    return "MIES";
};

MIESInjector::MIESInjector() { // Missing if construct plus statements plus else
                               // plus statements
    Matcher.addMatcher(ifStmt(hasElse(stmt())).bind("ifStmt"), createStmtHandler("ifStmt"));
}

bool MIESInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    const IfStmt *ifS = cast<IfStmt>(current.stmt);

    SourceRange range(ifS->getLocStart(), ifS->getElse()->getLocEnd());
    R.RemoveText(range);
    // return getEditedString(R, Context);
    return true;
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
