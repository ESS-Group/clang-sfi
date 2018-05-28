MFCInjector::MFCInjector(){
    Matcher.addMatcher(
        callExpr(
            ignoringImplicit(
                unless(
                    anyOf(
                        hasAncestor(varDecl(isDefinition())),
                        hasParent(returnStmt()),
                        hasParent(callExpr()),
                        hasParent(binaryOperator()),
                        hasParent(unaryOperator()),
                        cxxOperatorCallExpr()
                    )
                )
            )
        ).bind("FunctionCall"), createStmtHandler("FunctionCall"));
    //Matcher.addMatcher(callExpr().bind("FunctionCall"), createStmtHandler("FunctionCall"));
}

std::string MFCInjector::toString(){
    return "MFC";
};
std::string MFCInjector::inject(StmtBinding current, ASTContext &Context){
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MFCInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){
    /*if((C2(stmt, Context))){
    cerr<<((C2(stmt, Context))?"true":"false")<<endl;
    stmt->getLocStart().dump(Context.getSourceManager());
    stmt->dumpColor();
    }
    
    return false;*/
    /*stmt->getLocStart().dump(Context.getSourceManager());
        cerr<<endl;*/
    /*if(Context.getFullLoc(stmt->getLocStart()).getLineNumber()==1849 || Context.getFullLoc(stmt->getLocStart()).getLineNumber()==817){
        stmt->getLocStart().dump(Context.getSourceManager());
        cerr<<endl;
        stmt->dumpColor();
    }*/
    //return false;
    return C2(stmt, Context);
}