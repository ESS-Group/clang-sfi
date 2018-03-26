MVAEInjector::MVAEInjector(){
    Matcher.addMatcher(
            varDecl(
                    hasAncestor(compoundStmt())
            ).bind("varDecl"), createStmtHandler("varDecl")); 
}

std::string MVAEInjector::toString(){
    return "MVAE";
};




std::string MVAEInjector::inject(StmtBinding current, ASTContext &Context){

    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());

    SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
    R.RemoveText(range);

    return getEditedString(R, Context);
}


bool MVAEInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){
        std::vector<const BinaryOperator*> list = getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, true, false, true);
        for(const BinaryOperator* op:list){
            
            if(isExprAssignment(op)
                //&&(isInitializedBefore((const DeclRefExpr*)((op)->getLHS()), Context))
            ){
                if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){
                    if(!isParentOf(forstmt->getCond(), decl, Context) && !isParentOf(forstmt->getInc(), decl,Context) && C2(op, Context)){
                        nodeCallback(binding, op);
                    }
                } else if(C2(op, Context)){
                    nodeCallback(binding, op);
                }
            }
        }

        return false;
}