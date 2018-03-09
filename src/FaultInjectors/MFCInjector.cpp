MFCInjector::MFCInjector(){
    Matcher.addMatcher(callExpr(unless(anyOf(hasParent(varDecl(isDefinition())), hasParent(returnStmt()), cxxOperatorCallExpr(), hasParent(callExpr()), hasParent(expr())))).bind("FunctionCall"), createStmtHandler("FunctionCall"));
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
bool MFCInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    return C2(stmt, Context);
}