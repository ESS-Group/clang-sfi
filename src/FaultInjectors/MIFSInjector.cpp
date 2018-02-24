
MIFSInjector::MIFSInjector(){//Missing if construct plus statements
    Matcher.addMatcher(ifStmt().bind("ifStmt"), createStmtHandler("ifStmt"));
}

std::string MIFSInjector::toString(){
    return "MIFS";
};
std::string MIFSInjector::inject(StmtBinding current, ASTContext &Context){

    const IfStmt* ifS = (IfStmt *)(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getThen()->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIFSInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    //if(const IfStmt* ifS = (IfStmt *)(stmt)){
    const IfStmt* ifS = (IfStmt *)(stmt);
    //commented to also inject, when the then-block contains more than 5 statements
    //if(!C9(ifS->getThen()))
    //    return false;
    return C8(ifS) ;// && C2(stmt, Context); //also if the statement is the only statement in the block
    //} else return false;
}