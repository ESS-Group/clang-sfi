
MIFSInjector::MIFSInjector(){//Missing if construct plus statements
    //StmtHandler handler = createStmtHandler("ifStmt");
    //test.push_back(handler);
    Matcher.addMatcher(ifStmt().bind("ifStmt"), createStmtHandler("ifStmt"));

    //cout<<"MIFSINJECTOR:CONSTRUCTOR" <<endl;
}

std::string MIFSInjector::toString(){
    return "MIFS";
};
std::string MIFSInjector::inject(StmtBinding current, ASTContext &Context){

    //cout<<"Bearbeite "<<current.binding<<endl;
    const IfStmt* ifS = (IfStmt *)(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getThen()->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIFSInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    //cout<<"check Node"<<endl;
    if(const IfStmt* ifS = (IfStmt *)(stmt)){
        if(!C9(ifS->getThen()))
            return false;
        return C8(ifS) && C2(stmt, Context);
        /*if(const Stmt* Else = ifS->getElse())
            return false;
        else
            return true;
        */
    } else return false;
}