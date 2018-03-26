MVAVInjector::MVAVInjector(){
    Matcher.addMatcher(
            varDecl(
                    hasAncestor(compoundStmt())
            ).bind("varDecl"), createStmtHandler("varDecl")); 

}

std::string MVAVInjector::toString(){
    return "MVAV";
};




std::string MVAVInjector::inject(StmtBinding current, ASTContext &Context){
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    
    SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
    R.RemoveText(range);
    
    return getEditedString(R, Context);
}
bool MVAVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){
    std::vector<const BinaryOperator*> list = getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, 
                                            true, //also search in loops
                                            false, //do not search in for constructs
                                            true); //do not check initialization => use every assignment
    for(const BinaryOperator* op:list){
        if(
            isValueAssignment(op)
            //&& isInitializedBefore((const DeclRefExpr*)((op)->getLHS()), Context)
        ){
            if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3/*5*/)){
                if(!isParentOf(forstmt->getCond(), decl, Context) && !isParentOf(forstmt->getInc(), decl,Context)){ // not part of for construct!!!
                    nodeCallback(binding, op);
                }
            } else {
                nodeCallback(binding, op);
            }
        }
    }

    return false;
}