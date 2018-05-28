MIEBInjector::MIEBInjector(){//Missing if construct plus statements + Else before statements
    Matcher.addMatcher(ifStmt(hasElse(stmt())).bind("ifStmt"), createStmtHandler("ifStmt"));
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
        //commented to also inject, when the then-block contains more than 5 statements
        /*if(!C9(ifS->getThen()))
            return false;*/
        return C9(ifS->getThen(), &Context);
        //return !C8(ifS); //Else construct needed (ODC type)
}



std::string SMIEBInjector::toString(){
    return "SMIEB";
};
bool SMIEBInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){
        const IfStmt* ifS = (IfStmt *)(stmt);
        return C9(ifS->getThen(), &Context, false, 5, true);
}