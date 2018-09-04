#include "_all.h"

std::string MIESInjector::toString() {
    return "MIES";
};

MIESInjector::MIESInjector() { // Missing if construct plus statements plus else
                               // plus satements
    Matcher.addMatcher(ifStmt().bind("ifStmt"), createStmtHandler("ifStmt"));
}

std::string MIESInjector::inject(StmtBinding current, ASTContext &Context) {
    const IfStmt *ifS = (IfStmt *)(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getElse()->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIESInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) { // no else
    // if(const IfStmt* ifS = (IfStmt *)(stmt)){
    const IfStmt *ifS = (IfStmt *)(stmt);
    if (const Stmt *Else = ifS->getElse()) {
        return true;
    }
    return false;
    /*
    if(!C9(ifS->getThen()))
        return false;
    return C8(ifS) && C2(stmt, Context); //also if the statement is the only
    statement in the block
    //} else return false;
    */
}
