MIEBInjector::MIEBInjector(){//Missing if construct plus statements + Else before statements
    Matcher.addMatcher(ifStmt().bind("ifStmt"), createStmtHandler("ifStmt"));
}

std::string MIEBInjector::toString(){
    return "MIEB";
};
std::string MIEBInjector::inject(StmtBinding current, ASTContext &Context){

    const IfStmt* ifS = (IfStmt *)(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getElse()->getLocStart().getLocWithOffset(-1));
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIEBInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
        const IfStmt* ifS = (IfStmt *)(stmt);
        if(!C9(ifS->getThen()))
            return false;
        return !C8(ifS);
}