MIAInjector::MIAInjector(){//Missing if construct around statements
    Matcher.addMatcher(ifStmt().bind("ifStmt"), createStmtHandler("ifStmt"));
}

std::string MIAInjector::toString(){
    return "MIA";
};
std::string MIAInjector::inject(StmtBinding current, ASTContext &Context){
    const IfStmt* ifS = (IfStmt *)(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getThen()->getLocStart().getLocWithOffset(-1));
    //SourceRange sr(IfS->getLocStart(), Else->getLocStart().getLocWithOffset(-1));//-1 Offset da sonst das erste Zeichen des Else blockes mit gelöscht wird
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIAInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    //if(const IfStmt* ifS = (IfStmt *)(stmt)){
    //commented to also inject, when the then-block contains more than 5 statements
    //if(!C9(ifS->getThen())) 
    //    return false;
    return C8(ifS);
    //} else return false;
}