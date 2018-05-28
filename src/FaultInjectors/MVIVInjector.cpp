MVIVInjector::MVIVInjector(){
    Matcher.addMatcher(
        varDecl(hasInitializer(
            allOf(
            unless(
                anyOf(
                    callExpr(),
                    cxxNewExpr(),
                    binaryOperator(),
                    unaryOperator(),
                    cxxConstructExpr(),
                    declRefExpr(),
                    memberExpr(), 
                    castExpr(anyOf(
                            hasDescendant(declRefExpr()),
                            hasDescendant(memberExpr())))
            ))
            ,hasAncestor(compoundStmt())
            )
        )).bind("variable"), createStmtHandler("variable"));
}

std::string MVIVInjector::toString(){
    return "MVIV";
};
std::string MVIVInjector::inject(StmtBinding current, ASTContext &Context){
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    if(current.isStmt){
        SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
        R.RemoveText(range);
    } else {
        VarDecl temp (*((const VarDecl*)current.decl));
        temp.setInit(NULL);
        const VarDecl* tempP = &temp;
        std::string withoutInit = stmtToString(tempP, Context.getLangOpts());
        SourceRange range(current.decl->getLocStart(), current.decl->getLocEnd());
        R.ReplaceText(range, withoutInit);
    }
    return getEditedString(R, Context);
}
bool MVIVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){
    return C2(decl, Context);
    //C2 implementation implicitly excludes decl being part of an for construct.
}